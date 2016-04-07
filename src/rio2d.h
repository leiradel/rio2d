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

#pragma once

#include <stdint.h>
#include <stdarg.h>

#include "cocos2d.h"

namespace rio2d
{
  class Script : public cocos2d::Ref
  {
  public:
    enum Config
    {
      kMaxGlobals = 128,
      kMaxLocalVars = 32,
      kMaxThreads = 32,
      kMaxStack = 16,
    };

    typedef uint32_t Insn;
    typedef uint32_t Hash;
    typedef int      Index;
    typedef int      Token;
    typedef uint32_t Address;
    typedef float    Number;

    union Bytecode
    {
      Insn    m_insn;
      Address m_address;
      Index   m_index;
      Number  m_number;
      Hash    m_hash;
    };

    typedef void (cocos2d::Ref::*NotifyFunc)(cocos2d::Node*, Hash);

    struct LocalVar
    {
      Hash  m_hash;
      Token m_type;

      union
      {
        Number m_number;
        void*  m_pointer;
      };
    };

    struct Subroutine
    {
      Hash     m_hash;
      Address  m_pc;
      size_t   m_numLocals;
      LocalVar m_locals[kMaxLocalVars];
    };

    static inline Script* initWithSource(const char* source)
    {
      return initWithSource(source, nullptr, 0);
    }

    static Script* initWithSource(const char* source, char* error, size_t size);

    bool runAction(Hash hash, cocos2d::Node* target, ...);
    bool runAction(const char* name, cocos2d::Node* target, ...);
    bool runActionWithListener(cocos2d::Ref* listener, NotifyFunc port, Hash hash, cocos2d::Node* target, ...);
    bool runActionWithListener(cocos2d::Ref* listener, NotifyFunc port, const char* name, cocos2d::Node* target, ...);

    static Hash hash(const char* str);
    static Hash hashLower(const char* str);
    static Hash hash(const char* str, size_t length);
    static Hash hashLower(const char* str, size_t length);

  protected:
    bool init(const char* source, char* error, size_t size);
    bool runActionV(cocos2d::Ref* listener, NotifyFunc port, Hash hash, cocos2d::Node* target, va_list args);

    Bytecode* m_bytecode;
    size_t m_bcSize;

    Subroutine* m_globals;
    size_t m_numGlobals;
  };
}
