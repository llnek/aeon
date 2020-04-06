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

#include <iostream>
#include "lexer.h"
#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CASTXXX(T,v,panic,object,msg) do { \
  if (auto p= v.ptr(); typeid(object)==typeid(*p)) { return s__cast(T,p); } \
  if (panic) expected(msg, v); \
  return NULL; } while (0)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue expected(const stdstr& m, d::DslValue v) {
  RAISE(d::BadArg,
        "Expected `%s`, got %s.\n", C_STR(m), C_STR(v->pr_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BNumber A_NUM { 0 };
BStr A_STR { "" };
LibFunc A_FUNC;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LibFunc* cast_native(d::DslValue v, int panic) {
  CASTXXX(LibFunc,v,panic,A_FUNC,"native");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BNumber* cast_number(d::DslValue v, int panic) {
  CASTXXX(BNumber,v,panic,A_NUM,"number");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BStr* cast_string(d::DslValue v, int panic) {
  CASTXXX(BStr,v,panic,A_STR,"string");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue op_math(BNumber* lhs, int op, BNumber* rhs) {
  bool ints = lhs->isInt() && rhs->isInt();
  llong L;
  double R;
  switch (op) {
    case T_INT_DIV:
      ASSERT1(ints);
      ASSERT1(!rhs->isZero());
      L = (lhs->getInt() / rhs->getInt());
    break;
    case d::T_PLUS:
      if (ints)
        L = lhs->getInt() + rhs->getInt();
      else
        R = lhs->getFloat() + rhs->getFloat();
    break;
    case d::T_MINUS:
      if (ints)
        L = lhs->getInt() - rhs->getInt();
      else
        R = lhs->getFloat() - rhs->getFloat();
    break;
    case d::T_MULT:
      if (ints)
        L = lhs->getInt() * rhs->getInt();
      else
        R = lhs->getFloat() * rhs->getFloat();
    break;
    case d::T_DIV:
      ASSERT1(!rhs->isZero());
      if (ints)
        L = lhs->getInt() / rhs->getInt();
      else
        R = lhs->getFloat() / rhs->getFloat();
    break;
    case T_MOD:
      if (ints) {
        L = (lhs->getInt() % rhs->getInt());
      } else {
        R = ::fmod(lhs->getFloat(),rhs->getFloat());
      }
    break;
    case T_POWER:
      if (ints) {
        L = ::pow(lhs->getInt(), rhs->getInt());
      } else {
        R = ::pow(lhs->getFloat(),rhs->getFloat());
      }
    break;
  }
  return ints ? NUMBER_VAL(L) : NUMBER_VAL(R);
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


