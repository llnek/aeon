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
  if (auto p= v.get(); p && typeid(object)==typeid(*p)) { return s__cast(T,p); } \
  if (panic) expected(msg, v); \
  return P_NIL; } while (0)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr& m, d::DValue v) {
  RAISE(d::BadArg,
        "Expected `%s`, got %s.", C_STR(m), C_STR(v->pr_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// globals used to test typeids
BNumber A_NUM;
BStr A_STR;
BChar A_CHAR;
BArray A_ARRAY;
LibFunc A_FUNC;
Lambda A_DEFN;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray::~BArray() { DEL_PTR(value); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray::BArray(const IntVec& szs) {
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
d::DValue BArray::set(d::VSlice pms, d::DValue v) {
  int pos = index(pms);
  ASSERT(pos >=0 && pos < value->size(),
         "Array::set, index out of bound, got %d.", pos);
  (*value)[pos]= v;
  return v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BArray::get(d::VSlice pms) {
  int pos= index(pms);
  ASSERT(pos >=0 && pos < value->size(),
         "Array::get, index out of bound, got %d.", pos);
  return value->operator[](pos);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BArray::index(d::VSlice pms) {
  ASSERT(ranges.size() == pms.size(),
         "Mismatch DIMs, expected %d, got %d.", (int)ranges.size(), (int)pms.size());
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
    buf += N_STR(n-1);
  }
  return "DIM(" + buf + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BArray::eq(d::DValue rhs) const {
  bool ok=0;
  if (d::is_same(rhs, this)) {
    auto p= DCAST(BArray, rhs);
    int i=0, len = value->size();
    if (len == p->value->size()) {
      for (; i < len; ++i) {
        if (! (*value)[i]->equals(
              p->value->operator[](i))) {
          break;
        }
      }
      ok = i >= len;
    }
  }
  return ok;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BArray::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    auto p= DCAST(BArray, rhs);
    auto len = value->size();
    auto rz = p->value->size();
    if (eq(rhs)) { return 0; }
    if (len > rz) { return 1; }
    if (len < rz) { return -1; }
    return 0;
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LibFunc* cast_native(d::DValue v, int panic) {
  CASTXXX(LibFunc,v,panic,A_FUNC,"native");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lambda* cast_lambda(d::DValue v, int panic) {
  CASTXXX(Lambda,v,panic,A_DEFN,"lambda");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray* cast_array(d::DValue v, int panic) {
  CASTXXX(BArray,v,panic,A_ARRAY,"array");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BNumber* cast_number(d::DValue v, int panic) {
  CASTXXX(BNumber,v,panic,A_NUM,"number");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BStr* cast_string(d::DValue v, int panic) {
  CASTXXX(BStr,v,panic,A_STR,"string");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BChar* cast_char(d::DValue v, int panic) {
  CASTXXX(BChar,v,panic,A_CHAR,"char");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue op_math(d::DValue left, int op, d::DValue right) {
  auto rhs = cast_number(right,1);
  auto lhs = cast_number(left,1);
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
void ensure_data_type(cstdstr& n, d::DValue v) {
  auto cz= n[n.size()-1];
  auto s= cast_string(v,0);
  switch (cz) {
  case '$':
    ASSERT(s, "Expecting string, got %s.", C_STR(v->pr_str(1)));
  break;
  case '!': // single
  case '#': // double
  case '%': // int
  break;
  default:
    ASSERT(!s, "Expecting number, got %s.", C_STR(v->pr_str(1)));
  break;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lambda::Lambda(cstdstr& name, StrVec& pms, d::DAst e) : Function(name) {
  body=e;
  s__ccat(params, pms);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Lambda::invoke(d::IEvaluator* e, d::VSlice args) {
  ASSERT(args.size() == params.size(),
      "Arity error, expecting %d got %d.", (int)params.size(), (int)args.size());
  // we need to create a new frame to process this defn.
  e->pushFrame(name());
  // push all args onto stack
  for (int i=0, z=params.size(); i < z; ++i) {
    auto pv= *(args.begin+i);
    e->setValue(params[i],pv);
  }
  auto res= body->eval(e);
  e->popFrame();
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Lambda::invoke(d::IEvaluator* e) {
  d::ValVec vs;
  return invoke(e, d::VSlice(vs));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Lambda::pr_str(bool p) const {
  return "#lambda@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lambda::eq(d::DValue rhs) const {
  return d::is_same(rhs, this) &&
         DCAST(Lambda, rhs)->name() == name();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Lambda::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    return eq(rhs) ? 0 : pr_str().compare(rhs->pr_str());
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LibFunc::LibFunc(cstdstr& name, Invoker k) : Function(name) {
  fn=k;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LibFunc::invoke(d::IEvaluator* e, d::VSlice args) {
  return fn(e, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LibFunc::invoke(d::IEvaluator* e) {
  d::ValVec vs;
  return invoke(e, d::VSlice(vs));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LibFunc::pr_str(bool p) const {
  return "#native@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LibFunc::eq(d::DValue rhs) const {
  return d::is_same(rhs, this) && DCAST(LibFunc, rhs)->fn == fn;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LibFunc::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    return DCAST(LibFunc, rhs)->fn == fn
           ? 0 : pr_str().compare(rhs->pr_str());
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BNumber::match(const BNumber* rhs) const {
  return (isInt() && rhs->isInt())
    ? getInt() == rhs->getInt()
    : a::fuzzy_equals(getFloat(), rhs->getFloat());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BNumber::eq(d::DValue rhs) const {
  return d::is_same(rhs, this) && match(DCAST(BNumber, rhs));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BNumber::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    auto p= DCAST(BNumber, rhs);
    return match(p) ? 0 : (getFloat() > p->getFloat() ? 1 : -1);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BStr::eq(d::DValue rhs) const {
  return d::is_same(rhs, this) && DCAST(BStr,rhs)->value == value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BStr::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    return value.compare(DCAST(BStr,rhs)->value);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BChar::eq(d::DValue rhs) const {
  return d::is_same(rhs, this) && DCAST(BChar,rhs)->value == value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BChar::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    auto p = DCAST(BChar,rhs);
    return value==p->value ? 0 : (value > p->value ? 1 : -1);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


