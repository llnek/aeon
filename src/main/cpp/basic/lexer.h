#pragma once
/* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright © 2013-2021, Kenneth Leung. All rights reserved. */

#include "../dsl/dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define PRK(t) DCAST(czlab::dsl::Token,t)->pr_str().c_str()

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic{
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum TokenType{
  T_ARRAYINDEX = 1000,
  T_FUNCALL,
  T_REM,
  T_INPUT,
  T_PRINTLN,
  T_PRINT,
  T_LET,
  T_END,
  T_RUN,
  T_IF,
  T_THEN,
  T_ELSE,
  T_GOTO,
  T_FOR,
  T_TO,
  T_NEXT,
  T_STEP,
  T_READ,
  T_DATA,
  T_GOSUB,
  T_RETURN,
  T_INT_DIV,
  T_POWER,
  T_MOD,
  T_NOTEQ,
  T_LTEQ,
  T_GTEQ,
  T_NOT,
  T_AND,
  T_OR,
  T_ON,
  T_DEF,
  T_XOR,
  T_DIM,
  T_RESTORE,
  T_PROGRAM,

  T_EOL
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int type);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lexer : public d::IScanner{

  virtual bool isKeyword(cstdstr&) const;
  d::Context& ctx(){ return _ctx; }

  virtual d::DToken getNextToken();
  virtual d::DToken skipComment();
  virtual d::DToken number();
  virtual d::DToken id();
  virtual d::DToken string();

  Lexer(const Tchar* src);
  virtual ~Lexer(){}

  private:

  d::Context _ctx;
};



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

