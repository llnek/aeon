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
d::DValue expected(cstdstr& m, d::DValue v, d::Addr k) {
  RAISE(d::BadArg,
        "Wanted `%s`, got %s near %s",
        m.c_str(), PSTR(v), d::pr_addr(k).c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr& m, d::DValue v) {
  RAISE(d::BadArg,
        "Wanted `%s`, got %s", m.c_str(), PSTR(v));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray::~BArray() { DEL_PTR(value); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BArray::BArray(const IntVec& szs) {
  // DIM(2,2,2) => 3 x 3 x 3 = 27
  int len = 1;
  for (auto& n : szs) {
    auto actual = n+1;
    len = len * actual;
    s__conj(ranges,actual); }

  ASSERT(len >= 0,
         "Array size >= 0, got %d", len);

  value=new d::ValVec(len);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BArray::set(d::VSlice pms, d::DValue v) {
  int pos = index(pms);
  if (pos < 0 || pos >= value->size())
    RAISE(d::IndexOOB,
          "Array::set, index out of bound, got %d", pos);
  value->operator[](pos)= v;
  return v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BArray::get(d::VSlice pms) {
  int pos= index(pms);
  if (pos < 0 || pos >= value->size())
    RAISE(d::IndexOOB,
          "Array::get, index out of bound, got %d", pos);
  return value->operator[](pos);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BArray::index(d::VSlice pms) {
  if (ranges.size() != pms.size())
    E_SEMANTIC("Mismatch DIMs, wanted %d, got %d",
               (int) ranges.size(), (int) pms.size());
  //algo= z * (XY) + yX + x
  //DIM(3,3,3) A(2,2,2)
  auto X=0,Y=0,Z=0;
  auto x=0,y=0,z=0;
  for (int i=0,e=pms.size();i<e;++i) {
    auto v= *(pms.begin+i);
    auto num= vcast<d::Number>(v);
    if (!(num && num->isInt()))
      E_SEMANTIC("Array index expected Int, got %s", PRV(v,1));
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
  for (auto& n : ranges)
    buf += (buf.empty()?"":",") + N_STR(n-1);
  return "DIM(" + buf + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BArray::equals(d::DValue rhs) const {
  bool ok=0;
  if (d::is_same(rhs, this)) {
    auto p= DCAST(BArray, rhs);
    int i=0;
    int len = value->size();
    if (len == p->value->size()) {
      for (; i < len; ++i) {
        if (! (*value)[i]->equals(
              p->value->operator[](i))) break; }
      ok = i >= len;
    }
  }
  return ok;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BArray::compare(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    auto p= DCAST(BArray, rhs);
    auto len = value->size();
    auto rz = p->value->size();
    if (equals(rhs)) { return 0; }
    if (len > rz) { return 1; }
    if (len < rz) { return -1; }
    return 0;
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue op_math(d::DValue left, int op, d::DValue right) {
  auto rhs = vcast<d::Number>(right,DMARK_00);
  auto lhs = vcast<d::Number>(left,DMARK_00);
  bool ints = lhs->isInt() && rhs->isInt();
  llong L;
  double R;
  switch (op) {
  case T_INT_DIV:
    if (!ints)
      E_SYNTAX("Operator INT-DIV requires %d ints", 2);
    if (rhs->isZero())
      RAISE(d::DivByZero,
            "Div by zero, denominator= %d", (int)rhs->getInt());
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
    if (rhs->isZero())
      RAISE(d::DivByZero,
            "Div by zero, denominator= %d", (int)rhs->getInt());
    if (ints)
      L = lhs->getInt() / rhs->getInt();
    else
      R = lhs->getFloat() / rhs->getFloat();
  break;
  case T_MOD:
    if (ints)
      L = (lhs->getInt() % rhs->getInt());
    else
      R = ::fmod(lhs->getFloat(),rhs->getFloat());
  break;
  case T_POWER:
    if (ints)
      L = ::pow(lhs->getInt(), rhs->getInt());
    else
      R = ::pow(lhs->getFloat(),rhs->getFloat());
  break;
  }
  return ints ? NUMBER_VAL(L) : NUMBER_VAL(R);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ensure_data_type(cstdstr& n, d::DValue v) {
  auto s= vcast<d::String>(v);
  auto cz= n[n.size()-1];
  switch (cz) {
  case '$':
    if (!s)
      E_SYNTAX("Wanted string, got %s", PRV(v,1));
  break;
  case '!': // single
  case '#': // double
  case '%': // int
  break;
  default:
    if (s)
      E_SYNTAX("Wanted number, got %s", PRV(v,1));
  break;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lambda::Lambda(cstdstr& name, StrVec& pms, d::DAst e) : Function(name) {
  s__ccat(params, pms);
  body=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Lambda::invoke(d::IEvaluator* e, d::VSlice args) {
  if (args.size() != params.size())
    RAISE(d::BadArity,
          "Arity error, wanted %d got %d",
          (int) params.size(), (int) args.size());

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
bool Lambda::equals(d::DValue rhs) const {
  return d::is_same(rhs, this) &&
         DCAST(Lambda, rhs)->name() == name();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Lambda::compare(d::DValue rhs) const {
  if (!d::is_same(rhs, this)) {
    return pr_str().compare(rhs->pr_str()); }
  else {
    return equals(rhs) ? 0 : pr_str().compare(rhs->pr_str()); }
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
bool LibFunc::equals(d::DValue rhs) const {
  return d::is_same(rhs, this) &&
         DCAST(LibFunc, rhs)->fn == fn;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LibFunc::compare(d::DValue rhs) const {
  if (!d::is_same(rhs, this)) {
    return pr_str().compare(rhs->pr_str()); }
  else {
    return DCAST(LibFunc, rhs)->fn == fn
           ? 0 : pr_str().compare(rhs->pr_str()); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BChar::equals(d::DValue rhs) const {
  return d::is_same(rhs, this) &&
         DCAST(BChar,rhs)->value == value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BChar::compare(d::DValue rhs) const {
  if (!d::is_same(rhs, this)) {
    return pr_str().compare(rhs->pr_str()); }
  else {
    auto p = DCAST(BChar,rhs);
    return value==p->value ? 0 : (value > p->value ? 1 : -1); }
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


