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
namespace czlab::otto {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a= czlab::aeon;
namespace d= czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool truthy(d::DValue v) { return DCAST(LValue,v)->truthy(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(LSeqable* s, d::ValVec& out) { appendAll(s, 0, out); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(LSeqable* s, int from, int to, d::ValVec& out) {
  for (; from < to; ++from) {
    s__conj(out,s->nth(from));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(LSeqable* s, int from, d::ValVec& out) {
  appendAll(s, from, s->count(), out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(d::VSlice args, int from, int to, d::ValVec& out) {
  for (auto i= from; (args.begin+i) != args.end; ++i) {
    if (i < to)
      s__conj(out, *(args.begin+i));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(d::VSlice args, int from, d::ValVec& out) {
  for (auto i= from; (args.begin+i) != args.end; ++i) {
    s__conj(out, *(args.begin+i));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr& m, d::DValue v, d::Addr info) {
  RAISE(d::BadArg,
        "Wanted `%s`, got %s near %s",
        C_STR(m), C_STR(v->pr_str()), d::pr_addr(info).c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr& m, d::DValue v) {
  RAISE(d::BadArg,
        "Wanted `%s`, got %s", C_STR(m), C_STR(v->pr_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr hash_key(d::DValue s) {
  if (!s) {
    RAISE(a::NPError, "Failed to hash key for map.%s","\n");
  }
  return s->pr_str();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential* cast_sequential(d::DValue v, d::Addr info) {
  if (auto p=cast_sequential(v); p)
    return p;
  else
    return expected("sequenctial", v, info), P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential* cast_sequential(d::DValue v) {
  if (auto r= vcast<LList>(v); r) return s__cast(LSequential,r);
  if (auto r= vcast<LVec>(v); r) return s__cast(LSequential,r);
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeqable* cast_seqable(d::DValue v, d::Addr info) {
  if (auto p= cast_seqable(v); p)
    return p;
  else
    return expected("seq'qble", v, info), P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeqable* cast_seqable(d::DValue v) {
  if (auto r= vcast<LVec>(v); r) { return s__cast(LSeqable,r); }
  if (auto r= vcast<LSet>(v); r) { return s__cast(LSeqable,r); }
  if (auto r= vcast<LHash>(v); r) { return s__cast(LSeqable,r); }
  if (auto r= vcast<LList>(v); r) { return s__cast(LSeqable,r); }
  if (auto r= vcast<LString>(v); r) { return s__cast(LSeqable,r); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFunction* cast_function(d::DValue v, d::Addr info) {
  if (auto p= cast_function(v); p)
    return p;
  else
    return expected("function", v, info), P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFunction* cast_function(d::DValue v) {
  if (auto a= vcast<LLambda>(v); a) return s__cast(LFunction,a);
  if (auto a= vcast<LMacro>(v); a) return s__cast(LFunction,a);
  if (auto a= vcast<LNative>(v); a) return s__cast(LFunction,a);
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool scan_numbers(d::VSlice vs) {
  auto r=false;
  for (auto i= 0; (vs.begin+i) != vs.end; ++i) {
    auto x= *(vs.begin+i);
    auto n= vcast<LNumber>(x);
    if (E_NIL(n)) { expected("number", x); }
    if (!n->isInt()) { r= true; }
  }
  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LString::encoded() const {
  return d::escape(value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LString::rest() const {
  if (value.size() == 0)
    return EMPTY_LIST();
  else
  { stdstr s{value.c_str()+1};
    return LString(s).seq(); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LString::contains(d::DValue key) const {
  return s__index(vcast<LNumber>(key,addr())->getInt(), value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LString::seq() const {
  d::ValVec out;
  for (auto i=value.begin(),e=value.end();i!=e;++i)
    s__conj(out, CHAR_VAL(*i));
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LString::nth(int pos) const {
  if (!s__index(pos,value))
    RAISE(d::IndexOOB,
          "Index out of range: %d near %s",
          pos, d::pr_addr(addr()).c_str());
  return CHAR_VAL(value[pos]);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(d::DToken t) : LValue(t->addr()) {
  int del=127;
  char c = (char) del;
  value = stdstr { c } + t->getStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(cstdstr& s) {
  int del=127;
  char c = (char) del;
  value = stdstr { c } + s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LKeyword::pr_str(bool p) const {
  stdstr s{value}; s[0]= ':'; return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSymbol::eval(Lisper*, d::DFrame e) {
  if (auto r= e->get(value); r) {
    return r;
  }
  RAISE(d::NoSuchVar,
        "No such symbol %s near %s",
        value.c_str(), d::pr_addr(addr()).c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential::LSequential(const LSequential* rhs, d::DValue m) : LValue(m) {
  s__ccat(values, rhs->values);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential::LSequential(d::VSlice chunk) {
  appendAll(chunk, 0, values);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential::LSequential(d::ValVec& chunk) {
  s__ccat(values, chunk);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LSequential::pr_str(bool p) const {
  stdstr out;
  for (auto b=values.cbegin(),e=values.cend(); b!=e; ++b) {
    out += (out.empty()?"":" ") + (*b)->pr_str(p);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void LSequential::evalEach(Lisper* e, d::DFrame env, d::ValVec& out) const {
  for (auto& i : values) {
    if (auto r= e->EVAL(i, env); r)
      s__conj(out, r);
    else
      RAISE(d::BadEval,
            "Failed to eval form: %s, near %s",
            i->pr_str(1).c_str(), d::pr_addr(addr()).c_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSequential::nth(int pos) const {
  return s__index(pos,values) ? values[pos] : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::equals(d::DValue rhs) const {
  const LSequential* p = P_NIL;
  auto sz= count();

  if (vcast<LList>(rhs))
    p= cast_sequential(rhs);
  else
  if (vcast<LVec>(rhs))
    p= cast_sequential(rhs);

  if (E_NIL(p) ||
      sz != p->count())
    return false;

  //ok,let's try
  auto i=0; for (; i < sz; ++i) {
    if (!(nth(i)->equals(p->nth(i))))
    break;
  }

  return i >= sz;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSequential::compare(d::DValue rhs) const {
  const LSequential* p = P_NIL;
  auto sz= count();

  if (vcast<LList>(rhs))
    p= cast_sequential(rhs);
  else
  if (vcast<LVec>(rhs))
    p= cast_sequential(rhs);

  if (p) {
    auto rc= p->count();
    if (sz != rc) { return (sz > rc) ? 1 : -1; }
  }

  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSequential::first() const {
  return count() == 0 ? NIL_VAL() : nth(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSequential::rest() const {
  d::ValVec out;
  if (count() > 0)
    for (auto b=values.cbegin()+1,e=values.cend(); b != e; ++b) {
      s__conj(out, *b);
    }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSequential::seq() const {
  auto p = const_cast<LSequential*>(this);
  return LIST_VAL(p->values);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::contains(d::DValue key) const {
  auto n= vcast<LNumber>(key,addr())->getInt();
  return n>= 0 && n < count();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::DValue v) { s__conj(values,v); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::Addr a, d::DValue v1, d::DValue v2) : LSequential(a) {
  s__conj(values,v1);
  s__conj(values,v2);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::DValue v1,d::DValue v2) {
  s__conj(values,v1);
  s__conj(values,v2);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::Addr a, d::DValue v1,d::DValue v2, d::DValue v3) : LSequential(a) {
  s__conj(values,v1);
  s__conj(values,v2);
  s__conj(values,v3);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::DValue v1,d::DValue v2, d::DValue v3) {
  s__conj(values,v1);
  s__conj(values,v2);
  s__conj(values,v3);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LList::pr_str(bool p) const {
  return "(" + LSequential::pr_str(p) + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LList::eval(Lisper* e, d::DFrame env) {
  d::ValVec out;
  if (values.size() > 0)
    evalEach(e, env, out);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LList::conj(d::VSlice args) const {
  d::ValVec out;
  if (args.size() > 0) {
    for (auto i= 1;
         (args.begin != args.end-i);
         ++i)
      s__conj(out, *(args.end-i));
    s__conj(out, *args.begin);
  }
  return s__ccat(out, values), LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(d::DValue v) { s__conj(values,v); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(d::DValue v1,d::DValue v2) {
  s__conj(values,v1);
  s__conj(values,v2);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(d::DValue v1,d::DValue v2, d::DValue v3) {
  s__conj(values,v1);
  s__conj(values,v2);
  s__conj(values,v3);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LVec::eval(Lisper* e, d::DFrame env) {
  d::ValVec out;
  evalEach(e,env, out);
  return VEC_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LVec::pr_str(bool p) const {
  return "[" + LSequential::pr_str(p) + "]";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LVec::conj(d::VSlice more) const {
  d::ValVec out;
  s__ccat(out,values);
  if (more.size() > 0)
    for (auto i=0;
        more.begin+i != more.end;
        ++i) s__conj(out, *(more.begin+i));
  return VEC_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(d::VSlice more) {
  int c = more.size();

  if (!a::is_even(more.size()))
    E_SYNTAX("Wanted even n# of args, got %d near %s",
             c, "");

  for (auto i = more.begin; i != more.end; i += 2)
    values[hash_key(*i)] = HASH_VAL(*i, *(i+1));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(d::Addr a, d::ValVec& v) : LHash(v) { loc=a; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(d::ValVec& v) : LHash(d::VSlice(v)) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(const std::map<stdstr,VPair>& m) {
  for (auto& x : m)
    values[_1(x)] = _2(x);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(const LHash* rhs, d::DValue m) : LValue(m) {
  for (auto& x : rhs->values)
    values[_1(x)]= _2(x);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::assoc(d::VSlice more) const {
  int c = more.size();
  if (!a::is_even(c))
    E_SYNTAX("Wanted even n# of args, got %d near %s",
        c, "");
  std::map<stdstr,VPair> m(values);
  for (auto i = more.begin; i != more.end; i += 2) {
    m[hash_key(*i)] = HASH_VAL(*i,*(i+1));
  }
  return MAP_VAL(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::dissoc(d::VSlice more) const {
  std::map<stdstr,VPair> m(values);
  for (auto i= more.begin; i != more.end; ++i)
    m.erase(hash_key(*i));
  return MAP_VAL(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::nth(int pos) const {
  auto q= seq();
  auto s = vcast<LList>(q);
  if (auto z= s->count(); z > 0 && pos >= 0 && pos < z)
    return s->nth(pos);
  else
    return NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::seq() const {
  d::ValVec out;
  for (auto x=values.begin(),e=values.end();
       x != e; ++x)
  { auto p= _2_(x);
    s__conj(out, VEC_VAL2(_1(p), _2(p))); }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::rest() const {
  auto s = s__cast(LList,seq().get());
  d::ValVec out;
  if (auto z= s->count(); z > 1)
    for (auto i=1;i<z;++i)
      s__conj(out, s->nth(i));
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::contains(d::DValue key) const {
  return s__contains(values, hash_key(key));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::get(d::DValue key) const {
  auto i = values.find(hash_key(key));
  return (i != values.end()) ? _2(_2_(i)) : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::eval(Lisper* p, d::DFrame e) {
  std::map<stdstr,VPair> out;
  for (auto& it : values)
    out[_1(it)] = HASH_VAL(p->EVAL(_1(_2(it)),e),
                           p->EVAL(_2(_2(it)),e));
  return MAP_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::keys() const {
  d::ValVec keys;
  for (auto& x : values)
    s__conj(keys, _1(_2(x)));
  return LIST_VAL(keys);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::vals() const {
  d::ValVec out;
  for (auto& x : values)
    s__conj(out, _2(_2(x)));
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LHash::pr_str(bool p) const {
  stdstr out;

  for (auto& x : values)
    out += (out.empty()?"":",") +
           _1(_2(x))->pr_str(p) + " " + _2(_2(x))->pr_str(p);

  return "{" + out + "}";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::equals(d::DValue rhs) const {

  if (!d::is_same(rhs,this))
  return 0;

  auto const &rvs = vcast<LHash>(rhs)->values;
  auto sz = values.size();

  if (sz != rvs.size())
  return 0;

  auto i=0;
  for (auto p=values.begin(), e=values.end(); p != e; ++p,++i) {
    if (auto r= rvs.find(_1_(p)); r != rvs.end())
    { auto ro= *r;
      if (_1(_2_(p))->equals(_1(_2(ro))) &&
          _2(_2_(p))->equals(_2(_2(ro)))) continue; }
    break;
  }
  return i >= sz;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LHash::compare(d::DValue rhs) const {
  const LHash* rs= d::is_same(rhs,this) ? vcast<LHash>(rhs) : P_NIL;
  auto sz = values.size();
  if (rs)
  { auto rc= rs->count();
    if (sz != rc)
      return sz > rc ? 1 : -1; }
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative::LNative(const LNative* rhs, d::DValue m) : LFunction(m) {
  _name=rhs->_name;
  fn=rhs->fn;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LNative::pr_str(bool) const {
  return "(native)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LNative::equals(d::DValue rhs) const {
  return d::is_same(rhs,this) && (this == rhs.get());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LNative::compare(d::DValue rhs) const {
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LNative::invoke(Lisper* p, d::VSlice args) {
  return fn(p, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LNative::invoke(Lisper* p) {
  d::ValVec v;
  return invoke(p, d::VSlice(v));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static int L_SEED=0;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LLambda::pr_str(bool) const {
  return "(lambda)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame LLambda::bindContext(d::VSlice args) {
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
      fm->set(params[i+1], LIST_VAL(x));
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
d::DValue LLambda::invoke(Lisper* p, d::VSlice args) {
  return p->EVAL(body, bindContext(args));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LLambda::invoke(Lisper* p) {
  d::ValVec out;
  return invoke(p, d::VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda::LLambda(const StrVec& _args, d::DValue body, d::DFrame env)
: LFunction("anon#" + N_STR(++L_SEED)) {
  this->body = body;
  this->env= env;
  s__ccat(params, _args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda::LLambda(cstdstr& n,
    const StrVec& _args, d::DValue body, d::DFrame env) : LFunction(n) {
  this->env=env;
  this->body=body;
  s__ccat(params, _args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda::LLambda(const LLambda* rhs, d::DValue m) : LFunction(m) {
  _name=rhs->_name;
  env=rhs->env;
  body=rhs->body;
  s__ccat(params, rhs->params);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LLambda::equals(d::DValue rhs) const {
  if (!d::is_same(rhs,this))
  return 0;
  auto x= vcast<LLambda>(rhs);
  return _name == x->_name &&
         a::equals<stdstr>(params, x->params) &&
         body.get() == x->body.get();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LLambda::compare(d::DValue rhs) const {
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(cstdstr& n, const StrVec& args, d::DValue body, d::DFrame env)
  : LLambda(n, args, body, env) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const StrVec& args, d::DValue body, d::DFrame env)
  : LLambda(args, body,env) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LMacro::pr_str(bool) const {
  return "(macro)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool lessThan(d::DValue a, d::DValue b) {
  return a->equals(b) ? false : (a->compare(b) < 0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(d::VSlice more) : LSet() {
  for (auto i = more.begin; i != more.end; ++i)
    values->insert(*i);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(d::Addr a, d::ValVec& v) : LSet(v) { loc=a; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(d::ValVec& v) : LSet(d::VSlice(v)) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(d::DValue m) : LValue(m) {
  values=new std::set<d::DValue,SetCompare> { &lessThan };
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet() {
  values=new std::set<d::DValue,SetCompare> { &lessThan };
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::~LSet() {
  DEL_PTR(values);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(const std::set<d::DValue,SetCompare>& m) : LSet() {
  for (auto& x : m)
    values->insert(x);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(const LSet* rhs, d::DValue m) : LSet(m) {
  for (auto& x : *(rhs->values))
    values->insert(x);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::conj(d::VSlice more) const {
  std::set<d::DValue,SetCompare> m(*values);
  for (auto i = more.begin; i != more.end; ++i)
    m.insert(*i);
  return SET_VAL(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::disj(d::VSlice more) const {
  std::set<d::DValue,SetCompare> m(*values);
  for (auto i= more.begin; i != more.end; ++i)
    m.erase(*i);
  return SET_VAL(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::nth(int pos) const {
  auto q= seq();
  auto s= vcast<LList>(q,addr());
  if (auto z= s->count(); z > 0 && pos >= 0 && pos < z)
    return s->nth(pos);
  else
    return NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::seq() const {
  d::ValVec out;
  for (auto& x : *values)
    s__conj(out, x);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::rest() const {
  auto s = s__cast(LList,seq().get());
  if (auto z= s->count(); z > 1) {
    d::ValVec out;
    for (auto i=1;i<z;++i)
      s__conj(out, s->nth(i));
    return LIST_VAL(out);
  } else
    return EMPTY_LIST();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSet::contains(d::DValue key) const {
  return values->find(key) != values->end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::get(d::DValue key) const {
  auto i = values->find(key);
  return (i != values->end()) ? *i : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::eval(Lisper* p, d::DFrame e) {
  std::set<d::DValue,SetCompare> out(lessThan);
  for (auto& it : *values)
    out.insert(it);
  return SET_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LSet::pr_str(bool p) const {
  stdstr out;

  for (auto& x : *values)
    out += (out.empty()?"":",") + x->pr_str();

  return "#{" + out + "}";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSet::equals(d::DValue rhs) const {

  if (!d::is_same(rhs,this))
  return 0;

  auto const &rvs = vcast<LSet>(rhs)->values;
  auto sz = values->size();

  if (sz != rvs->size())
  return 0;

  auto i=0;
  for (auto p=values->begin(), e=values->end(); p != e; ++p,++i)
  { auto r= rvs->find(*p);
    if (r != rvs->end()) continue; else break; }
  return i >= sz;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSet::compare(d::DValue rhs) const {
  const LSet* rs= d::is_same(rhs,this) ? vcast<LSet>(rhs) : P_NIL;
  auto sz = values->size();
  if (rs)
  { auto rc = rs->count();
    if (sz != rc)
      return sz < rc ? -1 : 1; }
  return pr_str().compare(rhs->pr_str());
}








//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

