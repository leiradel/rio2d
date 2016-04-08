/******************************************************************************
* Copyright (c) 2016 Andre Leiradella
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include <map>
#include <atomic>

#include "rio2d.h"
#include "civetweb.h"

static struct mg_callbacks s_callbacks;
static struct mg_context* s_ctx;
static std::map<rio2d::Hash, std::atomic_uintptr_t*> m_scripts;

bool rio2d::Webserver::init(short port)
{
  char str[16];
  snprintf(str, sizeof(str), "%d", port);
  str[sizeof(str) - 1] = 0;

  const char* options[] =
  {
    "listening_ports", str,
    nullptr, nullptr
  };

  memset(&s_callbacks, 0, sizeof(s_callbacks));
  s_ctx = mg_start(&s_callbacks, nullptr, options);

  return s_ctx != nullptr;
}

void rio2d::Webserver::destroy()
{
  mg_stop(s_ctx);
  s_ctx = nullptr;

  for (auto it = m_scripts.begin(); it != m_scripts.end(); ++it)
  {
    auto script = (Script*)it->second->load();
    delete script;
    delete it->second;
  }
}

static rio2d::Script* initWithFilename(const char* filename, char* error, size_t size)
{
  auto data = cocos2d::FileUtils::getInstance()->getDataFromFile(filename);
  
  if (data.isNull())
  {
    CCLOG("Error reading from %s", filename);
    return nullptr;
  }

  char* source = (char*)alloca(data.getSize() + 1);

  if (source == nullptr)
  {
    CCLOG("Out of memory while copying %s", filename);
    return nullptr;
  }

  memcpy(source, data.getBytes(), data.getSize());
  source[data.getSize()] = 0;

  rio2d::Script* script = rio2d::Script::initWithSource(source, error, size);

  if (script == nullptr)
  {
    CCLOG("Error: %s", error);
    return nullptr;
  }

  return script;
}

static int serverError(struct mg_connection* conn, const char* reason)
{
  // We're using 500 for all errors, but shouldn't :P
  mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n");
  mg_printf(conn, "Internal Server Error\r\n\r\n%s", reason);
  return 1;
}

static int handleScriptUpload(struct mg_connection* conn, void* cbdata)
{
  auto ctx = mg_get_context(conn);
  auto req = mg_get_request_info(conn);

  // Check if the request is valid.
  if (strcmp(req->request_method, "POST"))
  {
    return serverError(conn, "Request method must be POST");
  }

  if (req->content_length == -1)
  {
    return serverError(conn, "Content-length header not present");
  }

  // Check if we're watching the script.
  // Disregard the leading slash, note that the URI length must be at least 1 in a valid HTTP request.
  rio2d::Hash hash = rio2d::hash(req->request_uri + 1);
  auto it = m_scripts.find(hash);

  if (it == m_scripts.end())
  {
    return serverError(conn, "Unknown resource");
  }

  // Read POST data.
  char* source = (char*)alloca(req->content_length + 1);

  if (source == nullptr)
  {
    return serverError(conn, "Error allocating memory to receive POST data");
  }

  int numread = mg_read(conn, source, req->content_length);

  if (numread != req->content_length)
  {
    return serverError(conn, "Error reading POST data");
  }

  source[numread] = 0;

  // Compile the new script.
  char error[256];
  auto script = rio2d::Script::initWithSource(source, error, sizeof(error));

  if (script == nullptr)
  {
    CCLOG("Error: %s", error);
    return serverError(conn, error);
  }
  
  // Replace the old script for the new.
  auto old = (rio2d::Script*)it->second->exchange((uintptr_t)script);
  script->retain();
  old->release();

  // Ok!
  mg_printf(conn, "HTTP/1.1 200 Ok\r\nContent-Type: text/plain\r\n\r\n");
  mg_printf(conn, "Script successfully compiled!");
  return 1;
}

rio2d::Script* rio2d::Webserver::getScript(const char* filename)
{
  // Try to find an existing script.
  Hash hash = rio2d::hash(filename);
  auto it = m_scripts.find(hash);

  if (it != m_scripts.end())
  {
    return (Script*)it->second->load();
  }

  // Not found, try to load it from the file system.
  char error[256];
  Script* script = initWithFilename(filename, error, sizeof(error));

  if (script == nullptr)
  {
    return nullptr;
  }

  // Ok, add it to the resource map.
  auto atomic = new (std::nothrow) std::atomic_uintptr_t((uintptr_t)script);
  auto pair = std::pair<rio2d::Hash, std::atomic_uintptr_t*>(hash, atomic);
  m_scripts.insert(pair);

  // Add a watch to reload the script via a POST request.
  char* path = (char*)alloca(strlen(filename) + 2);

  if (path == nullptr)
  {
    CCLOG("Error allocating memory, script %s not being watched");
  }
  else
  {
    sprintf(path, "/%s", filename); // We're sure there is enough space.

    for (char* aux = path; *aux; aux++)
    {
      if (*aux == '\\')
      {
        *aux = '/';
      }
    }

    mg_set_request_handler(s_ctx, path, handleScriptUpload, nullptr);
  }

  // Ok!
  script->retain();
  return script;
}
