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
  T_ON,
  T_DEF,
  T_XOR,
  T_DIM,
  T_RESTORE,

  T_EOL
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BToken : public d::Lexeme {

  static d::DToken make(int t, cstdstr& s, d::Mark m) {
    return WRAP_TKN(BToken, t, s, m);
  }
  static d::DToken make(int t, Tchar c, d::Mark m) {
    return WRAP_TKN(BToken, t, c, m);
  }

  void setLiteral(double d) { number.r= d;}
  void setLiteral(llong n) { number.n=n; }
  void setLiteral(int n) { number.n=n; }

  virtual double getFloat() const;
  virtual stdstr getStr() const;
  virtual llong getInt() const;
  virtual stdstr pr_str() const;
  virtual ~BToken() {}

  private:

  stdstr lexeme;
  union { llong n; double r; } number;

  BToken(int t, Tchar c, d::Mark m);
  BToken(int t, cstdstr& s, d::Mark m);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int type);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lexer : public d::IScanner {

  virtual bool isKeyword(cstdstr&) const;

  virtual d::DToken getNextToken();
  virtual d::DToken skipComment();
  virtual d::DToken number();
  virtual d::DToken id();
  virtual d::DToken string();

  d::Context& ctx() { return _ctx; }

  Lexer(const Tchar* src);
  virtual ~Lexer() {}

  private:

  d::Context _ctx;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token(int, Tchar, d::Mark);
d::DToken token(int, cstdstr&, d::Mark);


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

