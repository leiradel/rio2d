#include "ScriptMan.h"
#include "rio2d.h"
#include "civetweb.h"

static rio2d::Script* s_script = nullptr;

static int serverError(struct mg_connection* conn, const char* reason)
{
  mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n");
  mg_printf(conn, "Internal Server Error\r\n\r\n%s", reason);
  return 1;
}

static int uploadScript(struct mg_connection* conn, void* cbdata)
{
  struct mg_context* ctx = mg_get_context(conn);
  const struct mg_request_info* req = mg_get_request_info(conn);

  if (strcmp(req->request_method, "POST"))
  {
    return serverError(conn, "Request method must be POST");
  }

  if (req->content_length == -1)
  {
    return serverError(conn, "Content-length header not present");
  }

  char* source = new (std::nothrow) char[req->content_length + 1];

  if (source == nullptr)
  {
    return serverError(conn, "Error allocating memory to receive POST data");
  }

  int numread = mg_read(conn, source, req->content_length);

  if (numread != req->content_length)
  {
    delete[] source;
    return serverError(conn, "Error reading POST data");
  }

  source[numread] = 0;

  char error[256];
  rio2d::Script* script = rio2d::Script::initWithSource(source, error, sizeof(error));

  if (script == nullptr)
  {
    CCLOG("Error: %s", error);
    return serverError(conn, error);
  }

  // This should be an atomic operation, we'll assume a pointer write is atomic for now.

  rio2d::Script* old = s_script;
  script->retain();
  s_script = script;
  old->release();

  mg_printf(conn, "HTTP/1.1 200 Ok\r\nContent-Type: text/plain\r\n\r\n");
  mg_printf(conn, "Script successfully compiled!");
  return 1;
}

static rio2d::Script* getScript()
{
  if (s_script != nullptr)
  {
    return s_script;
  }

  // Initialize the script from a local file.

  cocos2d::Data data = cocos2d::FileUtils::getInstance()->getDataFromFile("scripts.bas");
  CCASSERT(!data.isNull(), "Error reading scripts.bas");

  char* source = (char*)malloc(data.getSize() + 1);
  CCASSERT(source != nullptr, "Out of memory");
  memcpy(source, data.getBytes(), data.getSize());
  source[data.getSize()] = 0;

  char error[256];
  s_script = rio2d::Script::initWithSource(source, error, sizeof(error));

  if (s_script == nullptr)
  {
    CCLOG("Error: %s", error);
    CCASSERT(0, "Error in script, check output");
  }

  s_script->retain();

  // Initialize the web server.

  const char* options[] =
  {
    "listening_ports", "8080",
    nullptr, nullptr
  };

  static struct mg_callbacks s_callbacks;
  memset(&s_callbacks, 0, sizeof(s_callbacks));
  static struct mg_context* ctx = mg_start(&s_callbacks, nullptr, options);

  if (ctx == nullptr)
  {
    CCLOG("Error: could not initialize the embedded web server");
    CCASSERT(0, "Error initializing the embedded web server");
  }
  else
  {
    mg_set_request_handler(ctx, "/scripts.bas", uploadScript, nullptr);
  }

  return s_script;
}

void runGrowBomb(cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, cocos2d::Node* target, cocos2d::Vec2* pos)
{
  getScript()->runActionWithListener(listener, port, 0xdd2ca404U /* growbomb */, target, pos);
}

void runRot(cocos2d::Node* target)
{
  getScript()->runAction(0x0b88a69aU /* rot */, target);
}

void runSwingHealth(cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, cocos2d::Node* target, cocos2d::Size* screenSize, float speed)
{
  getScript()->runActionWithListener(listener, port, 0xf2ee2603U /* swinghealth */, target, screenSize, speed);
}

void runMeteorFall(cocos2d::Ref* listener, rio2d::Script::NotifyFunc port, cocos2d::Node* target, cocos2d::Size* screenSize, float speed)
{
  getScript()->runActionWithListener(listener, port, 0x5b09ceb0U /* meteorfall */, target, screenSize, speed);
}

void runShockWave(cocos2d::Node* target, cocos2d::Node* bomb)
{
  getScript()->runAction(0x900402b0U /* shockwave */, target, bomb);
}
