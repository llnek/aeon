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
namespace czlab::tiny14e{
namespace d=czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum TokenType{
  // keywords
  T_PROGRAM = 100,
  T_INT_DIV,
  T_EQUALS,
  T_VAR,
  T_STR,
  T_INT,
  T_REAL,
  T_PROCEDURE,
  T_FOR,
  T_ENDFOR,
  T_IF,
  T_ELSE,
  T_ENDIF,
  T_REPEAT,
  T_UNTIL,
  T_WHILE,
  T_ENDWHILE,
  T_BEGIN,
  T_END,

  T_WRITELN,
  T_WRITE,
  T_READLN,
  T_READ,
  T_GTEQ,
  T_LTEQ,
  T_NOTEQ,
  T_NOT,
  T_XOR,
  T_OR,
  T_AND,
  T_ASSIGN

};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lexer : public d::IScanner{

  virtual bool isKeyword(cstdstr&) const;
  virtual d::DToken getNextToken();
  virtual d::DToken skipComment();
  virtual d::DToken number();
  virtual d::DToken id();
  virtual d::DToken string();

  Lexer(const Tchar* src);
  virtual ~Lexer(){};

  d::Context& ctx(){ return _ctx; }

  private:

  d::Context _ctx;
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

