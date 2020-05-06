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
namespace czlab::elle {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a= czlab::aeon;
namespace d= czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SPair::makeList(d::Addr a,d::ValVec& vs) {
  auto res=SNil::make();
  for (int i=vs.size()-1; i>=0; --i) {
    res=SPair::make(a, vs[i], res);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SPair::makePair(d::Addr a,d::ValVec& vs) {
  int e=vs.size()-1;
  auto res= SPair::make(a, vs[e-1], vs[e]);
  for (int i=e-2; i>=0; --i) {
    res=SPair::make(a, vs[i], res);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SPair::equals(d::DValue rhs) const {
  if (!d::is_same(rhs,this))
    return false;
  else
  { auto p= vcast<SPair>(rhs);
    return f1->equals(p->f1) && f2->equals(p->f2); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SPair::compare(d::DValue rhs) const {
  if (!d::is_same(rhs,this))
    return pr_str(0).compare(rhs->pr_str(0));
  else
  { auto p= vcast<SPair>(rhs);
    return equals(rhs) ? 0 : pr_str(0).compare(rhs->pr_str(0)); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr SPair::pr_str(bool p) const {

  stdstr special;
  stdstr buf;

  if (auto p2= vcast<SPair>(f2); p2 && isNil(p2->f2)) {
    if (auto s= vcast<SSymbol>(f1); s) {
      auto i= s->impl();
      special = (i == "quote")
                ? "'" : (i == "quasiquote")
                ? "`" : (i == "unquote")
                ? "," : (i == "unquote-splicing") ? ",@" : "";
    }
  }

  if (!special.empty()) {
    buf += special;
    buf += first(f2)->pr_str(p);
  } else {
    buf += "(";
    buf += f1->pr_str(p);
    auto tail=f2;
    for (; vcast<SPair>(tail);) {
      auto sp= vcast<SPair>(tail);
	    buf += " ";
      buf += sp->f1->pr_str(p);
	    tail = sp->f2;
    }
    if (tail)
	    buf += " . " + tail->pr_str(1);
  }
  return buf + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SPair::eval(Scheme*,d::DFrame) {
  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SString::nth(int pos) const {
  if (!(pos >= 0 && pos < value.size()))
    RAISE(d::IndexOOB,
          "Index out of bound: %d", pos);
  return SChar::make(addr(),value[pos]);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr SString::encoded() const {
  return d::escape(value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SSymbol::eval(Scheme*, d::DFrame) {
  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SVec::SVec(d::VSlice v) {
  for (auto i=0; (v.begin+i) != v.end; ++i) {
    s__conj(values, *(v.begin+i));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SVec::SVec(d::ValVec& vs, d::Addr a) : SValue(a) {
  s__ccat(values,vs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SVec::SVec(d::ValVec& vs) {
  s__ccat(values,vs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SVec::SVec(d::DValue a,d::DValue b,d::DValue c) {
  s__conj(values,a);
  s__conj(values,b);
  s__conj(values,c);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SVec::SVec(d::DValue a) {
  s__conj(values,a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SVec::SVec(d::DValue a,d::DValue b) {
  s__conj(values,a);
  s__conj(values,b);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void SVec::evalEach(Scheme* s, d::DFrame env, d::ValVec& out) const {

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SVec::nth(int pos) const {
  if (!s__index(pos,values))
    RAISE(d::IndexOOB,
          "Index out of bound: %d", pos);
  return values[pos];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SVec::equals(d::DValue rhs) const {
  if (d::is_same(rhs,this)) {
    auto p= vcast<SVec>(rhs);
    if (values.size()==p->values.size()) {
      int i=0, e=values.size();
      for (;i<e;++i)
      { if(!values[i]->equals(p->values[i])) break; }
      return i>=e;
    }
  }
  return 0;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SVec::compare(d::DValue rhs) const {
  if (!d::is_same(rhs,this))
    return pr_str(0).compare(rhs->pr_str(0));
  else
    return equals(rhs) ? 0 : pr_str(0).compare(rhs->pr_str(0));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SVec::eval(Scheme*, d::DFrame) {
  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr SVec::pr_str(bool p) const {
  stdstr buf { "#(" };
  for (auto& v : values)
    buf += (buf.empty()?"":" ") + v->pr_str(p);
  return buf + ")";
}

//kenl
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr&, d::DValue, d::Addr) {
  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr&, d::DValue) {
  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue first(d::DValue x) {
  auto p=vcast<SPair>(x);
  return p ? p->head() : DVAL_NIL;
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue rest(d::DValue x) {
  auto p= vcast<SPair>(x);
  return p ? p->tail() : DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue setFirst(d::DValue des, d::DValue y) {
  auto p= vcast<SPair>(des);
  if (p)
    p->head(y);
  else
    RAISE(d::BadArg,
          "Attempt to set-car of a non-Pair near %s",
          d::pr_addr(DCAST(SValue,des)->addr()).c_str());
  return y;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue setRest(d::DValue des, d::DValue y) {
  if (auto p=vcast<SPair>(des); p)
    p->tail(y);
  else
    RAISE(d::BadArg,
          "Attempt to set-cdr of a non-Pair near %s",
          d::pr_addr(DCAST(SValue,des)->addr()).c_str());
  return y;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue second(d::DValue x) {
  return first(rest(x));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue third(d::DValue x) {
  return first(rest(rest(x)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue list(d::DValue a, d::DValue b) {
  return SPair::make(a, SPair::make(b));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue list(d::DValue a) {
  return SPair::make(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/** listStar(args) is like Common Lisp (apply #'list* args) **/
d::DValue listStar(d::DValue args) {
  if (!rest(args))
    return first(args);
  else
    return cons(first(args), listStar(rest(args)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/** cons(x, y) is the same as new Pair(x, y). **/
d::DValue cons(d::DValue a, d::DValue b) {
  return SPair::make(a, b);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue reverse(d::DValue src) {
  auto res= SNil::make(); // empty list
  for (auto x= src; vcast<SPair>(x);) {
    res = cons(first(x), res);
    x = rest(x);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool equals(d::DValue x, d::DValue y) {
  return (!x || !y) ? x.get() == y.get() : x->equals(y);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/** The length of a list, or zero for a non-list. **/
int length(d::DValue x) {
  auto len = 0;
  for (; vcast<SPair>(x); )
  { ++len;
    x = vcast<SPair>(x)->tail(); }
  return len;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue listToVector(d::DValue objs) {
  d::ValVec v;
  for (; vcast<SPair>(objs);) {
    s__conj(v, first(objs));
    objs = rest(objs);
  }
  return SVec::make(v);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue vectorToList(d::DValue x) {
  auto v= vcast<SVec>(x, DCAST(SValue,x)->addr());
  auto res= SNil::make();
  for (int e= v->count(),i=e-1; i>=0; --i) {
    res = cons(v->nth(i), res);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool isNil(d::DValue v) {
  return vcast<SNil>(v) != P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

