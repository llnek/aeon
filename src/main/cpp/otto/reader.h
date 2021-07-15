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

#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::otto{
namespace d= czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//extending from token defs in dsl.h
enum TokenType{
  T_SPLICE_UNQUOTE= 100,
  T_KEYWORD,
  T_ANONFN,
  T_SET,
  T_TRUE,
  T_FALSE,
  T_NIL,
  T_COMMENT
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int type);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//A Lexer.
struct Reader : public d::IScanner{

  virtual bool isKeyword(cstdstr&) const;
  virtual d::DToken getNextToken();
  virtual d::DToken number();
  virtual d::DToken id();
  virtual d::DToken string();

  d::Context& ctx(){ return _ctx; }
  Reader(const Tchar* src);
  virtual ~Reader(){};

  private:

  d::Context _ctx;

  Reader();
  void skipCommas();
  d::DToken keywd();
  d::DToken skipComment();
};










//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

