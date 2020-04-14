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

#include "lexer.h"
#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool is_same(const d::Data* x, const d::Data* y) {
  return typeid(*x) == typeid(*y);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CASTXXX(T,v,panic,object,msg) do { \
  if (auto p= v.get(); p && typeid(object)==typeid(*p)) { return s__cast(T,p); } \
  if (panic) expected(msg, v); \
  return P_NIL; } while (0)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue expected(const stdstr& m, d::DslValue v) {
  RAISE(d::BadArg,
        "Expected `%s`, got %s.", C_STR(m), C_STR(v->pr_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// globals used to test typeids
BNumber A_NUM;
BStr A_STR;
BArray A_ARRAY;
LibFunc A_FUNC;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray::~BArray() { DEL_PTR(value); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray::BArray(const std::vector<llong>& szs) {
  int len = 1;
  // DIM(2,2,2) => 3 x 3 x 3 = 27
  for (auto& n : szs) {
    auto actual = n+1;
    len = len * actual;
    s__conj(ranges,actual);
  }
  ASSERT(len >= 0, "Array size >= 0, got %d.", len);
  value=new d::ValVec(len);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BArray::set(d::VSlice pms, d::DslValue v) {
  int pos = index(pms);
  ASSERT(pos >=0 && pos < value->size(),
         "Array::set, index out of bound, got %d.", pos);
  (*value)[pos]= v;
  return v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BArray::get(d::VSlice pms) {
  int pos= index(pms);
  ASSERT(pos >=0 && pos < value->size(),
         "Array::get, index out of bound, got %d.", pos);
  return value->operator[](pos);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong BArray::index(d::VSlice pms) {
  ASSERT(ranges.size() == pms.size(),
         "Array dims mismatch, expected %d, got %d.", (int)ranges.size(), (int)pms.size());
  //algo= z * (XY) + yX + x
  //DIM(3,3,3) A(2,2,2)
  auto X=0,Y=0,Z=0;
  auto x=0,y=0,z=0;
  for (int i=0,e=pms.size();i<e;++i) {
    auto num= cast_number(*(pms.begin+i),1);
    ASSERT(num->isInt(),
           "Array index expected Int, got %s.", C_STR(num->pr_str()));
    switch (i) {
      case 0:
        X=ranges[i]; x= num->getInt(); break;
      case 1:
        Y=ranges[i]; y= num->getInt(); break;
      case 2:
        Z=ranges[i]; z= num->getInt(); break;
    }
  }
  return z * (X * Y) + y * X + x;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BArray::pr_str(bool p) const {
  stdstr buf;
  for (auto& n : ranges) {
    if (!buf.empty()) buf += ",";
    buf += std::to_string(n-1);
  }
  return "DIM(" + buf + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BArray::eq(d::DslValue rhs) const {
  auto ok=false;
  if (is_same(rhs.get(), this)) {
    auto p= s__cast(BArray, rhs.get());
    int i=0, len = value->size();
    if (len == p->value->size()) {
      for (; i < len; ++i) {
        if (!value->operator[](i)->equals(p->value->operator[](i))) {
          break;
        }
      }
      ok = i >= len;
    }
  }
  return ok;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BArray::cmp(d::DslValue rhs) const {
  if (is_same(rhs.get(), this)) {
    auto p= s__cast(BArray, rhs.get());
    auto len = value->size();
    auto rz = p->value->size();
    if (eq(rhs)) { return 0; }
    else if (len > rz) { return 1; }
    else if (len < rz) { return -1; }
    else {
      return 0;
    }
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LibFunc* cast_native(d::DslValue v, int panic) {
  CASTXXX(LibFunc,v,panic,A_FUNC,"native");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray* cast_array(d::DslValue v, int panic) {
  CASTXXX(BArray,v,panic,A_ARRAY,"array");
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
d::DslValue op_math(d::DslValue left, int op, d::DslValue right) {
  auto lhs = cast_number(left,1);
  auto rhs = cast_number(right,1);
  bool ints = lhs->isInt() && rhs->isInt();
  llong L;
  double R;
  switch (op) {
    case T_INT_DIV:
      ASSERT(ints, "Operator INT-DIV requires %d integers.", 2);
      ASSERT(!rhs->isZero(), "Div by zero error, denominator= %d.", (int)rhs->getInt());
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
      ASSERT(!rhs->isZero(), "Div by zero error, denominator= %d.", (int)rhs->getInt());
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
bool LibFunc::eq(d::DslValue rhs) const {
  return is_same(rhs.get(), this) &&
         fn == s__cast(LibFunc, rhs.get())->fn;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LibFunc::cmp(d::DslValue rhs) const {
  if (is_same(rhs.get(), this)) {
    auto v2= s__cast(LibFunc, rhs.get())->fn;
    return fn==v2 ? 0 : pr_str(0).compare(rhs->pr_str(0));
  } else {
    return pr_str(0).compare(rhs->pr_str(0));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BNumber::eq(d::DslValue rhs) const {
  return is_same(rhs.get(), this) && match(s__cast(BNumber, rhs.get()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BNumber::cmp(d::DslValue rhs) const {
  auto ok= is_same(rhs.get(), this);
  if (ok) {
    auto p= s__cast(BNumber, rhs.get());
    return match(p) ? 0 : (getFloat() > p->getFloat() ? 1 : -1);
  } else {
    return pr_str(0).compare(rhs->pr_str(0));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BStr::eq(d::DslValue rhs) const {
  return is_same(rhs.get(), this) &&
           value == s__cast(BStr,rhs.get())->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BStr::cmp(d::DslValue rhs) const {
  if (is_same(rhs.get(), this)) {
    return value.compare(s__cast(BStr,rhs.get())->value);
  } else {
    return pr_str(0).compare(rhs->pr_str(0));
  }
}







//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


