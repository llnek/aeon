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
 * Copyright © 2013-2020, Kenneth Leung. All rights reserved. */

#include "../dsl/dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum TokenType {
  T_ARRAYINDEX = 1000,
  T_REM,
  T_INPUT,
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
  T_XOR,
  T_DIM,
  T_RESTORE,

  T_EOL
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Token : public d::AbstractToken {

  Token(int type, const stdstr&, d::SrcInfo);
  Token(int type, Tchar, d::SrcInfo);

  virtual stdstr getLiteralAsStr() const;
  virtual double getLiteralAsReal() const;
  virtual llong getLiteralAsInt() const;
  virtual stdstr pr_str() const;
  virtual ~Token() {}

  d::Lexeme& impl() { return _impl; }

  static stdstr typeToString(int);

  private:

  d::Lexeme _impl;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lexer : public d::IScanner {

  virtual bool isKeyword(const stdstr&) const;
  virtual d::DslToken getNextToken();
  d::DslToken _getNextToken();

  virtual d::DslToken number();
  virtual d::DslToken id();
  virtual d::DslToken string();
  virtual d::DslToken skipComment();

  Lexer(const char* src);
  virtual ~Lexer() {}

  d::Context& ctx() { return _ctx; }

  private:

  d::Context _ctx;
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, Tchar, d::SrcInfo);
d::DslToken token(int type, const stdstr&, d::SrcInfo);
d::DslToken token(int type, const stdstr&, d::SrcInfo, llong n);
d::DslToken token(int type, const stdstr&, d::SrcInfo, double d);



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

