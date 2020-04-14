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

#include <typeinfo>
#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool is_same(const d::Data* x, const d::Data* y) {
  ASSERT1(x);
  ASSERT1(y);
  return typeid(*x) == typeid(*y);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SStr::eq(d::DslValue rhs) const {
  return is_same(rhs.get(), this) && CAST(SStr,rhs)->value == value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SStr::cmp(d::DslValue rhs) const {
  if (is_same(rhs.get(), this)) {
    auto p= CAST(SStr,rhs);
    return value==p->value ? 0 : value.compare(p->value);
  } else {
    return pr_str(0).compare(rhs->pr_str(0));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SNumber::eq(d::DslValue rhs) const {
  return is_same(rhs.get(), this) && match(CAST(SNumber, rhs));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SNumber::cmp(d::DslValue rhs) const {
  if (is_same(rhs.get(), this)) {
    auto p= CAST(SNumber, rhs);
    return match(p) ? 0 : (getFloat() > p->getFloat() ? 1 : -1);
  } else {
    return pr_str(0).compare(rhs->pr_str(0));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SNumber::match(const SNumber* rhs) const {
  return (isInt() && rhs->isInt())
         ? getInt() == rhs->getInt()
         : a::fuzzy_equals(getFloat(), rhs->getFloat());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue expected(const stdstr& m, d::DslValue v) {
  RAISE(d::BadArg,
        "Expected `%s`, got %s.", C_STR(m), C_STR(v->pr_str()));
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SNumber A_NUM;
SStr A_STR;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SNumber* cast_number(d::DslValue v, int panic) {
  if (auto p= v.get(); p && typeid(A_NUM) == typeid(*p)) {
    return s__cast(SNumber,p);
  }
  if (panic) expected("Number", v);
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SStr* cast_string(d::DslValue v,int panic) {
  if (auto p= v.get(); p && typeid(A_STR) == typeid(*p)) {
    return s__cast(SStr, p);
  }
  if (panic) expected("String", v);
  return P_NIL;
}





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF
