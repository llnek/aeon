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

#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::kirby {
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum TokenType {
  T_SPLICE_UNQUOTE = 100,
  T_KEYWORD,
  T_SET,
  T_TRUE,
  T_FALSE,
  T_NIL,
  T_COMMENT
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Token : public d::AbstractToken {

  // A Lex token.

  static stdstr typeToString(int);

  Token(int type, const stdstr&, d::SrcInfo);
  Token(int type, const char, d::SrcInfo);

  virtual stdstr getLiteralAsStr() const;
  virtual double getLiteralAsReal() const;
  virtual llong getLiteralAsInt() const;

  d::Lexeme& impl() { return _impl; }
  stdstr toString() const;

  virtual ~Token() {}

  private:
  Token();
  d::Lexeme _impl;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Reader : public d::IScanner {

  // A Lexer.

  bool isKeyword(const stdstr&) const;
  d::DslToken getNextToken();
  d::DslToken number();
  d::DslToken id();
  d::DslToken string();

  d::Context& ctx() { return _ctx; }
  Reader(const char* src);
  virtual ~Reader() {};

  private:

  d::Context _ctx;

  d::DslToken _getNextToken();
  Reader();
  void skipCommas();
  d::DslToken keywd();
  d::DslToken skipComment();
};










//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

