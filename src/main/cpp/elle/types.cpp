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

d::DValue SNil::_singleton=WRAP_VAL(SNil);
d::DValue STrue::_singleton=WRAP_VAL(STrue);
d::DValue SFalse::_singleton=WRAP_VAL(SFalse);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue makeList(d::Addr a, d::ValVec& vs) {
  auto res=SNil::make();
  for (int i=vs.size()-1; i>=0; --i) {
    res=SPair::make(a, vs[i], res);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue makeList(d::ValVec& vs) {
  return makeList(DMARK_00,vs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue makeList(d::VSlice args) {
  auto len= args.size();
  auto res=SNil::make();
  for (int i=len-1; i>=0; --i)
    res=SPair::make(DMARK_00, *(args.begin+i), res);
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue makePair(d::Addr a, d::ValVec& vs) {
  auto e=vs.size()-1;
  auto res= SPair::make(a, vs[e-1], vs[e]);
  for (auto i=e-2; i>=0; --i) {
    res=SPair::make(a, vs[i], res);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue makePair(d::ValVec& vs) {
  return makePair(DMARK_00,vs);
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
    return equals(rhs)
           ? 0 : pr_str(0).compare(rhs->pr_str(0));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr SPair::pr_str(bool p) const {

  stdstr special;
  stdstr buf;

  if (auto p2= vcast<SPair>(f2); p2 && p2->hasNilTail()) {
    if (auto s= vcast<SSymbol>(f1); s) {
      auto i= s->impl();
      special = (i == "quote")
                ? "'" : (i == "quasiquote")
                ? "`" : (i == "unquote")
                ? "," : (i == "unquote-splicing") ? ",@" : "";
      if (!special.empty())
        buf += special + p2->head()->pr_str(p);
    }
  }

  if (special.empty()) {
    buf += "(" + head()->pr_str(p);
    auto t=tail();
    for (; vcast<SPair>(t);) {
      auto sp= vcast<SPair>(t);
      buf += " ";
      buf += sp->head()->pr_str(p);
      t= sp->tail();
    }
    if (!vcast<SNil>(t))
      buf += " . " + t->pr_str(p);
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
d::DValue SSymbol::eval(Scheme*, d::DFrame env) {
  return env->get(value);
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

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SNative::compare(d::DValue rhs) const {
  if (!d::is_same(rhs,this))
    return pr_str(0).compare(rhs->pr_str(0));
  else
    return equals(rhs) ? 0 : pr_str(0).compare(rhs->pr_str(0));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SNative::equals(d::DValue rhs) const {
  if (!d::is_same(rhs,this))
    return false;
  else
  { auto p= vcast<SNative>(rhs);
    return this==p ||
           (name() == p->name() && fn == p->fn); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SNative::invoke(Scheme* s, d::VSlice args) {
  return fn(s,args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SNative::invoke(Scheme* s) {
  d::ValVec v;
  return invoke(s, d::VSlice(v));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame SLambda::bindContext(d::VSlice args) {
  auto fm= d::Frame::make(pr_str(1), env);
  auto len= args.size();
  auto z=params.size();
  auto i=0, j=0;
  // run through parameters...
  for (; i < z; ++i) {
    auto k= params[i];
    if (k == "&") {
      // var-args, next must be the last one
      // e.g. [a b c & x]
      ASSERT1((i+1 == (z-1)));
      d::ValVec x;
      appendAll(args,j,x);
      fm->set(params[i+1], makeList(addr(),x));
      j=len;
      i= z;
      break;
    }
    if (!(j < len))
      throw d::BadArity(z,len);
    fm->set(k, *(args.begin + j));
    ++j;
  }
  // make sure arg count matches param count
  ASSERT1(j==len && i== z);
  return fm;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SLambda::invoke(Scheme* p, d::VSlice args) {
  return p->EVAL(body, bindContext(args));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue SLambda::invoke(Scheme* p) {
  d::ValVec out;
  return invoke(p, d::VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static int L_SEED=0;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SLambda::SLambda(const StrVec& _args, d::DValue body, d::DFrame env)
: SFunction("anon#" + N_STR(++L_SEED)) {
  this->body = body;
  this->env= env;
  s__ccat(params, _args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SLambda::SLambda(cstdstr& n,
    const StrVec& _args, d::DValue body, d::DFrame env) : SFunction(n) {
  this->env=env;
  this->body=body;
  s__ccat(params, _args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SLambda::SLambda(const SLambda* rhs) : SFunction(rhs->name()) {
  env=rhs->env;
  body=rhs->body;
  s__ccat(params, rhs->params);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SLambda::equals(d::DValue rhs) const {
  if (!d::is_same(rhs,this))
  return 0;
  auto x= vcast<SLambda>(rhs);
  return this==x ||
    (name() == x->name() &&
         a::equals<stdstr>(params, x->params) &&
         body.get() == x->body.get());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SLambda::compare(d::DValue rhs) const {
  return equals(rhs)
         ? 0 : pr_str(0).compare(rhs->pr_str(0));
}

//kenl
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool truthy(d::DValue v) {
  return DCAST(SValue,v)->truthy();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue evalEach(Scheme* s, d::DFrame env,d::DValue v) {
  auto p= vcast<SPair>(v);
  if (!p)
    RAISE(d::BadArg, "Wanted list, got %s", v->pr_str(1).c_str());
  return evalEach(s, env, p);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue evalEach(Scheme* s, d::DFrame env, SPair* p) {
  auto _A= p->addr();
  d::ValVec out;
  std::cout << "start of evalEach at: " << d::pr_addr(_A) << "\n";
  while (p) {
    auto h= p->head();
    std::cout << "head= " << h->pr_str(1) << "\n";
    auto r= DCAST(SValue,h)->eval(s,env);
    s__conj(out, r);
    std::cout << "head evaled to = " << r->pr_str(1) << "\n";
    auto t= p->tail();
    p= vcast<SPair>(t);
  }
  std::cout << "end of evalEach\n";
  auto x= makeList(_A, out);
  std::cout << "end of evalEach = " << x <<"\n";
  return x;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue nth(d::DValue v, int pos) {
  auto s= vcast<SPair>(v);
  if (!s)
    RAISE(d::BadArg, "Wanted list, got %s", v->pr_str(1).c_str());
  return nth(s,pos);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool atomQ(d::DValue v) {
  return !vcast<SPair>(v) && !vcast<SNil>(v);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue nth(SPair* start, int pos) {
  if (pos < 0)
    return DVAL_NIL;
  for (int i=0; i != pos; ++i)
  { auto t= start->tail();
    if (auto z= vcast<SPair>(t); z)
      start=z;
    else
      RAISE(d::IndexOOB,"Index out of bound: %d", pos); }
  return start->head();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(d::DValue v, d::ValVec& out) {
  auto p=vcast<SPair>(v);
  if (p)
    appendAll(p, 0, out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(SPair* p, d::ValVec& out) {
  appendAll(p, 0, out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(d::DValue v, int from, int to, d::ValVec& out) {
  auto p=vcast<SPair>(v);
  if (p)
    appendAll(p,from,to,out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(SPair* p, int from, int to, d::ValVec& out) {
  for (; from < to; ++from)
    s__conj(out, nth(p, from));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(d::DValue v, int from, d::ValVec& out) {
  auto p=vcast<SPair>(v);
  if (p)
    appendAll(p, from, out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(SPair* p, int from, d::ValVec& out) {
  appendAll(p, from, count(p), out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(d::VSlice args, int from, int to, d::ValVec& out) {
  for (auto i= from;
      (args.begin+i) != args.end;
      ++i)
    if (i < to)
      s__conj(out, *(args.begin+i));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(d::VSlice args, int from, d::ValVec& out) {
  for (auto i= from;
      (args.begin+i) != args.end;
      ++i)
    s__conj(out, *(args.begin+i));
}

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
int length(d::DValue x) {
  auto p=vcast<SPair>(x);
  return p ? count(p) : 0;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int count(SPair* p) {
  int len=0;
  while (1)
  { auto t= p->tail();
    auto z= vcast<SPair>(t);
    ++len;
    if (!z ||
        vcast<SNil>(t)) break; else p=z; }
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
bool listQ(d::DValue v) {
  auto p= vcast<SPair>(v);
  return p ? listQ(p) : false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool listQ(SPair* p) {
  while (1)
  { if (!p)
      return false;
    auto t=p->tail();
    auto z= vcast<SPair>(t);
    if (vcast<SNil>(t))
      return true; else p=z; }
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

