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
#define CASTXXX(T,v,panic,object,msg) do { \
  if (auto p= v.ptr(); p && typeid(object)==typeid(*p)) { return s__cast(T,p); } \
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
BArray A_ARRAY;
LibFunc A_FUNC;


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray::~BArray() {
  DEL_PTR(value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray::BArray(const std::vector<llong>& szs) {
  auto len = 1;
  // DIM(2,2,2) => 3 x 3 x 3 = 27
  for (auto& n : szs) {
    auto actual = n+1;
    len = len * actual;
    s__conj(ranges,actual);
  }
  ASSERT1(len >= 0);
  value=new d::ValVec(len);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BArray::set(d::VSlice pms, d::DslValue v) {
  auto pos = index(pms);
  ASSERT1(pos >=0 && pos < value->size());
  (*value)[pos]= v;
  return v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BArray::get(d::VSlice pms) {
  auto pos= index(pms);
  ASSERT1(pos >=0 && pos < value->size());
  return value->operator[](pos);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong BArray::index(d::VSlice pms) {
  ASSERT1(ranges.size() == pms.size());
  //algo= z * (XY) + yX + x
  //DIM(3,3,3) A(2,2,2)
  auto X=0,Y=0,Z=0;
  auto x=0,y=0,z=0;
  for (int i=0,e=pms.size();i<e;++i) {
    auto num= cast_number(*(pms.begin+i),1);
    ASSERT(num->isInt(),
           "Array index expects int, got %s.\n", C_STR(num->pr_str()));
    ASSERT1(i < 3);
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
bool BArray::eq(const Data* other) const {
  auto ok=false;
  if (typeid(*this) == typeid(*other)) {
    auto rhs= s__cast(const BArray,other);
    int i=0, len = value->size();
    if (len == rhs->value->size()) {
      for (; i < len; ++i) {
        if (!value->operator[](i)->equals(rhs->value->operator[](i).ptr())) {
          break;
        }
      }
      ok = i >= len;
    }
  }
  return ok;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BArray::cmp(const d::Data* other) const {
  if (typeid(*this) == typeid(*other)) {
    auto rhs= s__cast(const BArray,other);
    auto len = value->size();
    auto rz = rhs->value->size();
    if (eq(rhs)) { return 0; }
    else if (len > rz) { return 1; }
    else if (len < rz) { return -1; }
    else {
      return 0;
    }
  } else {
    return pr_str().compare(other->pr_str());
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


