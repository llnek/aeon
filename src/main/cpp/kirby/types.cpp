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
namespace czlab::kirby {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a= czlab::aeon;
namespace d= czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CAST(t,x) s__cast(t,x.ptr())
#define TO_VAL(x) CAST(LValue,x)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative A_NATIVE;
LLambda A_LAMBDA;
LMacro A_MACRO;
LAtom A_ATOM;
LKeyword A_KEYWORD { "" };
LList A_LIST;
LVec A_VEC;
LHash A_MAP;
LSet A_SET;
LFloat A_FLOAT { 0.0 };
LInt A_INT { 0 };
LChar A_CHAR {'\0'};
LNil A_NIL;
LTrue A_TRUE;
LFalse A_FALSE;
LString A_STR { "" };
LSymbol A_SYMB { "" };

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool truthy(d::DslValue v) { return TO_VAL(v)->truthy(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEqual(int wanted, int got, const stdstr& fn) {
  if (wanted != got)
    RAISE(BadArity,
          "%s requires %d args, got %d.\n", C_STR(fn), wanted, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preMax(int max, int got, const stdstr& fn) {
  if (got > max)
    RAISE(BadArity,
          "%s requires at most %d args, got %d.\n", C_STR(fn), max, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preMin(int min, int got, const stdstr& fn) {
  if (got < min)
    RAISE(BadArity,
          "%s requires at least %d args, got %d.\n", C_STR(fn), min, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preNonZero(int c, const stdstr& fn) {
  if (c == 0)
    RAISE(BadArity,
          "%s requires some args, got %d.\n", C_STR(fn), c);
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEven(int c, const stdstr& fn) {
  if (!a::is_even(c))
    RAISE(BadArity,
          "%s requires even args, got %d.\n", C_STR(fn), c);
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(LSeqable* s, VVec& out) { appendAll(s, 0, out); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(LSeqable* s, int from, int to, VVec& out) {
  for (; from < to; ++from) {
    s__conj(out,s->nth(from));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(LSeqable* s, int from, VVec& out) {
  appendAll(s, from, s->count(), out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(VSlice args, int from, int to, VVec& out) {
  for (auto i= from; (args.begin+i) != args.end; ++i) {
    if (i < to)
      s__conj(out, *(args.begin+i));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(VSlice args, int from, VVec& out) {
  for (auto i= from; (args.begin+i) != args.end; ++i) {
    s__conj(out, *(args.begin+i));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue expected(const stdstr& m, d::DslValue v) {
  RAISE(BadArg,
        "Expected `%s`, got %s.\n", C_STR(m), C_STR(v->pr_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr hash_key(d::DslValue s) {
  if (s.isNull()) {
    RAISE(a::NPError, "Failed to hash key for map.%s","\n");
  }
  return s->pr_str();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CASTXXX(T,v,panic,object,msg) do { \
  if (auto p= v.ptr(); typeid(object)==typeid(*p)) { return s__cast(T,p); } \
  if (panic) expected(msg, v); \
  return P_NIL; } while (0)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNil* cast_nil(d::DslValue v, int panic) {
  CASTXXX(LNil,v,panic,A_NIL,"nil");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LChar* cast_char(d::DslValue v, int panic) {
  CASTXXX(LChar,v,panic,A_CHAR,"char");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFloat* cast_float(d::DslValue v, int panic) {
  CASTXXX(LFloat,v,panic,A_FLOAT,"float");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LInt* cast_int(d::DslValue v, int panic) {
  CASTXXX(LInt,v,panic,A_INT,"int");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol* cast_symbol(d::DslValue v, int panic) {
  CASTXXX(LSymbol,v,panic,A_SYMB,"symbol");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString* cast_string(d::DslValue v, int panic) {
  CASTXXX(LString,v,panic,A_STR,"string");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword* cast_keyword(d::DslValue v, int panic) {
  CASTXXX(LKeyword,v,panic,A_KEYWORD,"keyword");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList* cast_list(d::DslValue v, int panic) {
  CASTXXX(LList,v,panic,A_LIST,"list");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec* cast_vec(d::DslValue v, int panic) {
  CASTXXX(LVec,v,panic,A_VEC,"vector");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet* cast_set(d::DslValue v, int panic) {
  CASTXXX(LSet,v,panic,A_SET,"set");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash* cast_map(d::DslValue v, int panic) {
  CASTXXX(LHash,v,panic,A_MAP,"hashmap");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda* cast_lambda(d::DslValue v, int panic) {
  CASTXXX(LLambda,v,panic,A_LAMBDA,"lambda");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro* cast_macro(d::DslValue v, int panic) {
  CASTXXX(LMacro,v,panic,A_MACRO,"macro");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative* cast_native(d::DslValue v, int panic) {
  CASTXXX(LNative,v,panic,A_NATIVE,"native");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LAtom* cast_atom(d::DslValue v, int panic) {
  CASTXXX(LAtom,v,panic,A_ATOM,"atom");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential* cast_seq(d::DslValue v, int panic) {
  if (auto r= cast_list(v); r) return s__cast(LSequential,r);
  if (auto r= cast_vec(v); r) return s__cast(LSequential,r);
  if (panic) expected("sequenctial", v);
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeqable* cast_seqable(d::DslValue v, int panic) {
  if (auto r= cast_vec(v); r) { return s__cast(LSeqable,r); }
  if (auto r= cast_set(v); r) { return s__cast(LSeqable,r); }
  if (auto r= cast_map(v); r) { return s__cast(LSeqable,r); }
  if (auto r= cast_list(v); r) { return s__cast(LSeqable,r); }
  if (auto r= cast_string(v); r) { return s__cast(LSeqable,r); }
  if (panic) expected("seq'qble", v);
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFunction* cast_function(d::DslValue v, int panic) {
  if (auto a= cast_lambda(v); a) return s__cast(LFunction,a);
  if (auto a= cast_macro(v); a) return s__cast(LFunction,a);
  if (auto a= cast_native(v); a) return s__cast(LFunction,a);

  if (panic)
    expected("function", v);

  return NULL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool cast_numeric(VSlice vs, NumberVec& out) {
  auto r=false;
  for (auto i= 0; (vs.begin+i) != vs.end; ++i) {
    auto x= *(vs.begin+i);
    auto f= cast_float(x);
    auto n= cast_int(x);
    if (E_NIL(n) && E_NIL(f)) {
      expected("numeric", x);
    }
    if (n) {
      s__conj(out, d::Number(n->impl()));
    }
    else
    if (f) {
      r= true;
      s__conj(out, d::Number(f->impl()));
    }
  }
  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr unescape(const stdstr& src) {
  auto len = src.length();
  auto ch= '\n';
  stdstr res;
  if (len == 0 ||
      !(src[0]=='"' && src[len-1]=='"')) { return src; }
  // skip 1st and last => no dqoutes
  --len;
  for (auto i = 1; i < len; ++i) {
    ch = src[i];
    if (ch == '\\') {
      i += 1;
      res += a::unescape_char(src[i]);
    } else {
      res += ch;
    }
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr escape(const stdstr& src) {
  auto len = src.length();
  stdstr res;
  if (len == 0 ||
      (src[0]=='"' && src[len-1]=='"')) { return src; }
  for (auto i = 0; i < len; ++i) {
    res += a::escape_char(src[i]);
  }
  return "\"" + res + "\"";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LValue::withMeta(d::DslValue m) const {
  RAISE(Unsupported, "%s: not supported here.", "with-meta");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LValue::meta() const {
  return metaObj.isNull() ? NIL_VAL() : metaObj;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LValue::eval(Lisper*, d::DslFrame e) {
  // by default, eval to self.
  return d::DslValue(this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LValue::compare(const d::Data* rhs) const {
  ASSERT1(rhs != nullptr);
  return cmp(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LValue::equals(const d::Data* rhs) const {
  ASSERT1(rhs != nullptr);
  return eq(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LFalse::cmp(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs)
         ? 0 : pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LFalse::pr_str(bool p) const {
  return "false";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LFalse::withMeta(d::DslValue m) const {
  return new LFalse(*this,m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LFalse::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFalse::LFalse(const LFalse& rhs, d::DslValue m) : LValue(m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LTrue::LTrue(const LTrue& rhs, d::DslValue m) : LValue(m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LTrue::pr_str(bool p) const  { return "true"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LTrue::withMeta(d::DslValue m) const {
  return new LTrue(*this,m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LTrue::cmp(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs)
         ? 0 : pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LTrue::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNil::LNil(const LNil& rhs, d::DslValue m) : LValue(m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LNil::pr_str(bool p) const  { return "nil"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LNil::withMeta(d::DslValue m) const {
  return new LNil(*this,m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LNil::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LNil::cmp(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs)
         ? 0 : pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LFloat::withMeta(d::DslValue m) const {
  return new LFloat(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::Number LFloat::number() const {
  return d::Number(value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFloat::LFloat(const LFloat& rhs, d::DslValue m) : LValue (m) {
  value = rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LFloat::pr_str(bool pretty) const {
  return std::to_string(value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LFloat::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) &&
         value == s__ccast(LFloat,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LFloat::cmp(const d::Data* rhs) const {
  if (typeid(*this)==typeid(*rhs)) {
    auto v= s__ccast(LFloat,rhs)->value;
    return value==v ? 0 : value > v ? 1 : -1;
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LInt::withMeta(d::DslValue m) const {
  return new LInt(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::Number LInt::number() const {
  return d::Number(value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LInt::LInt(const LInt& rhs, d::DslValue m) : LValue (m) {
  value = rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LInt::pr_str(bool pretty) const {
  return std::to_string(value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LInt::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) &&
         value == s__ccast(LInt,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LInt::cmp(const d::Data* rhs) const {
  if (typeid(*this)==typeid(*rhs)) {
    auto v= s__ccast(LInt,rhs)->value;
    return value==v ? 0 : value > v ? 1 : -1;
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LChar::pr_str(bool p) const {
  return stdstr { value };
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LChar::cmp(const d::Data* rhs) const {
  if (typeid(*this) == typeid(*rhs)) {
    auto c = s__ccast(LChar,rhs)->value;
    return value==c ? 0 : value > c ? 1 : -1;
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LChar::LChar(const LChar& rhs, d::DslValue m) : LValue(m) {
  value = rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LChar::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) &&
         value == s__ccast(LChar,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LChar::withMeta(d::DslValue m) const {
  return new LChar(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LAtom::pr_str(bool p) const {
  return "atom(" + value->pr_str(p) + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LAtom::LAtom(d::DslValue v) : value(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LAtom::LAtom(const LAtom& rhs, d::DslValue m) : LValue(m) {
  value= rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LAtom::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) && value->equals(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LAtom::cmp(const d::Data* rhs) const {
  if (typeid(*this)==typeid(*rhs)) {
    auto a= s__ccast(LAtom,rhs)->value;
    return value->compare(a.ptr());
  }
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LAtom::withMeta(d::DslValue m) const {
  return new LAtom(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString::LString(const LString& rhs, d::DslValue m) : LValue(m) {
  value=rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString::LString(const stdstr& s) : value(s) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LString::pr_str(bool p) const {
  return p ? encoded() : value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LString::encoded() const {
  return escape(value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LString::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs) &&
         value == s__ccast(LString,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LString::cmp(const d::Data* rhs) const {
  if (typeid(*this) == typeid(*rhs)) {
    auto s = s__ccast(LString,rhs)->value;
    return value.compare(s);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::withMeta(d::DslValue m) const {
  return new LString(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LString::isEmpty() const { return value.empty(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LString::count() const { return value.size(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::first() const {
  return value.size() == 0 ? NIL_VAL() : CHAR_VAL(value[0]);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::rest() const {
  if (value.size() == 0) {
    return EMPTY_LIST();
  } else {
    stdstr s { value.c_str() + 1 };
    return LString(s).seq();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LString::contains(d::DslValue key) const {
  auto n= cast_int(key,1)->impl();
  return n >= 0 && n < value.size();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::seq() const {
  VVec out;
  for (auto i=value.begin(), e=value.end(); i != e; ++i) {
    s__conj(out, CHAR_VAL(*i));
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::nth(int pos) const {
  if (pos < 0 || pos >= value.size()) {
    RAISE(IndexOOB, "Index out of range: %d.\n", pos);
  }
  return CHAR_VAL(value[pos]);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(const stdstr& s) {
  int del=127;
  char c = (char) del;
  value = stdstr { c } + s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(const LKeyword& rhs, d::DslValue m) : LValue(m) {
  value= rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LKeyword::pr_str(bool p) const {
  stdstr s { value };
  s[0]= ':';
  return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LKeyword::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) &&
         value == s__ccast(LKeyword,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LKeyword::cmp(const d::Data* rhs) const {
  if (typeid(*this)==typeid(*rhs)) {
    auto k= s__ccast(LKeyword,rhs)->value;
    return value.compare(k);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LKeyword::withMeta(d::DslValue m) const {
  return new LKeyword(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol::LSymbol(const LSymbol& rhs, d::DslValue m) : LValue(m) {
  value=rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol::LSymbol(const stdstr& s) : value(s) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LSymbol::pr_str(bool p) const { return value; }

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void LSymbol::rename(const stdstr& n) {
  value=n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSymbol::eval(Lisper*, d::DslFrame e) {
  if (auto r= e->get(value); r.isSome()) {
    return r;
  }
  RAISE(NoSuchVar, "No such symbol %s.\n", C_STR(value));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSymbol::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs) &&
         value == s__ccast(LSymbol,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSymbol::cmp(const d::Data* rhs) const {
  if (typeid(*this) == typeid(*rhs)) {
    auto s= s__ccast(LSymbol,rhs)->value;
    return value.compare(s);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSymbol::withMeta(d::DslValue m) const {
  return new LSymbol(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential::LSequential(const LSequential& rhs, d::DslValue m) : LValue(m) {
  s__ccat(values,rhs.values);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential::LSequential(VSlice chunk) {
  appendAll(chunk,0,values);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential::LSequential(VVec& chunk) {
  s__ccat(values,chunk);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LSequential::pr_str(bool pretty) const {
  stdstr out;
  for (auto b = values.cbegin(), e=values.cend(); b != e; ++b) {
    if (!out.empty()) { out += " "; }
    out += (*b)->pr_str(pretty);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void LSequential::evalEach(Lisper* e, d::DslFrame env, VVec& out) const {
  for (auto& i : values) {
    if (auto r= e->EVAL(i, env); r.isSome()) {
      s__conj(out, r);
    } else {
      RAISE(BadEval, "%s.\n", C_STR(i->pr_str(1)));
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::isEmpty() const { return values.empty(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSequential::count() const { return values.size(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSequential::nth(int pos) const {
  return (pos >= 0 && pos < values.size()) ? values[pos] : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::eq(const d::Data* v) const {
  const LSequential* rhs = P_NIL;
  auto sz= count();
  if (typeid(A_LIST) == typeid(*v)) {
    rhs= s__ccast(LSequential,v);
  }
  else if (typeid(A_VEC) == typeid(*v)) {
    rhs= s__ccast(LSequential,v);
  }
  if (E_NIL(rhs) || sz != rhs->count()) {
    return false;
  }
  //ok,let's try
  auto i=0; for (; i < sz; ++i) {
    if (!(nth(i)->equals(rhs->nth(i).ptr())))
    break;
  }
  return i >= sz;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSequential::cmp(const d::Data* v) const {
  const LSequential* rhs = P_NIL;
  auto sz= count();
  if (typeid(A_LIST) == typeid(*v)) {
    rhs= s__ccast(LSequential,v);
  }
  else if (typeid(A_VEC) == typeid(*v)) {
    rhs= s__ccast(LSequential,v);
  }
  if (rhs) {
    auto rc= rhs->count();
    if (sz != rc) { return (sz > rc) ? 1 : -1; }
  }
  return pr_str().compare(v->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSequential::first() const {
  return count() == 0 ? NIL_VAL() : nth(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSequential::rest() const {
  VVec out;
  if (count() > 0)
    for (auto b=values.cbegin()+1,e=values.cend(); b != e; ++b) {
      s__conj(out, *b);
    }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSequential::seq() const {
  auto p = const_cast<LSequential*>(this);
  return LIST_VAL(p->values);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::contains(d::DslValue key) const {
  auto n= cast_int(key,1)->impl();
  return n >= 0 && n < count();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(const LList& rhs, d::DslValue m) : LSequential(rhs, m) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(VSlice v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(VVec& v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::DslValue v) { s__conj(values,v); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::DslValue v1,d::DslValue v2) {
  s__conj(values,v1);
  s__conj(values,v2);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::DslValue v1,d::DslValue v2, d::DslValue v3) {
  s__conj(values,v1);
  s__conj(values,v2);
  s__conj(values,v3);
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LList::pr_str(bool pretty) const {
  return "(" + LSequential::pr_str(pretty) + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LList::eval(Lisper* e, d::DslFrame env) {
  if (values.size() == 0) { return d::DslValue(this); }
  VVec out;
  evalEach(e, env, out);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LList::conj(VSlice args) const {
  VVec out;
  if (args.size() > 0) {
    for (auto i= 1; (args.begin != args.end-i); ++i) {
      s__conj(out, *(args.end-i));
    }
    s__conj(out, *args.begin);
  }
  s__ccat(out, values);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LList::withMeta(d::DslValue m) const {
  return d::DslValue(new LList(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(const LVec& rhs, d::DslValue m) : LSequential(rhs, m) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(VSlice v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(VVec& v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(d::DslValue v) { s__conj(values,v); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(d::DslValue v1,d::DslValue v2) {
  s__conj(values,v1);
  s__conj(values,v2);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(d::DslValue v1,d::DslValue v2, d::DslValue v3) {
  s__conj(values,v1);
  s__conj(values,v2);
  s__conj(values,v3);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LVec::eval(Lisper* e, d::DslFrame env) {
  VVec out;
  evalEach(e,env, out);
  return VEC_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LVec::pr_str(bool pretty) const {
  return "[" + LSequential::pr_str(pretty) + "]";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LVec::conj(VSlice more) const {
  VVec out;
  s__ccat(out,values);
  if (more.size() > 0) {
    for (auto i=0; more.begin+i != more.end; ++i) {
      s__conj(out, *(more.begin+i));
    }
  }
  return VEC_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LVec::withMeta(d::DslValue m) const {
  return new LVec(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(VSlice more) {
  int c = more.size();
  ASSERT(a::is_even(more.size()),
         "Expected even n# of args, got %d", c);
  for (auto i = more.begin; i != more.end; i += 2) {
    values[hash_key(*i)] = HASH_VAL(*i, *(i+1));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(VVec& v) : LHash(VSlice(v)) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash() {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(const std::map<stdstr,VPair>& m) {
  for (auto& x : m) {
    values[x.first] = x.second;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(const LHash& rhs, d::DslValue m) : LValue(m) {
  for (auto& x : rhs.values) {
    values[x.first]= x.second;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::assoc(VSlice more) const {
  int c = more.size();
  ASSERT(a::is_even(c),
         "Expected even n# of args, got %d", c);
  std::map<stdstr,VPair> m(values);
  for (auto i = more.begin; i != more.end; i += 2) {
    m[hash_key(*i)] = HASH_VAL(*i,*(i+1));
  }
  return MAP_VAL(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::dissoc(VSlice more) const {
  std::map<stdstr,VPair> m(values);
  for (auto i= more.begin; i != more.end; ++i) {
    m.erase(hash_key(*i));
  }
  return MAP_VAL(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::isEmpty() const { return values.empty(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LHash::count() const { return values.size();  }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::nth(int pos) const {
  auto q= seq();
  auto s = cast_list(q,1);
  if (auto z= s->count(); z > 0 && pos >= 0 && pos < z) {
    return s->nth(pos);
  } else {
    return NIL_VAL();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::seq() const {
  VVec out;
  for (auto x=values.begin(), e=values.end(); x != e; ++x) {
    auto p= x->second;
    s__conj(out, VEC_VAL2(p.first, p.second));
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::first() const {
  return nth(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::rest() const {
  auto s = s__cast(LList,seq().ptr());
  if (auto z= s->count(); z > 1) {
    VVec out;
    for (auto i=1; i < z; ++i) {
      s__conj(out, s->nth(i));
    }
    return LIST_VAL(out);
  } else {
    return EMPTY_LIST();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::contains(d::DslValue key) const {
  return values.find(hash_key(key)) != values.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::get(d::DslValue key) const {
  auto i = values.find(hash_key(key));
  return (i != values.end()) ? i->second.second : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::eval(Lisper* p, d::DslFrame e) {
  std::map<stdstr,VPair> out;
  for (auto& it : values) {
    out[it.first] = HASH_VAL(p->EVAL(it.second.first,e),
                             p->EVAL(it.second.second, e));
  }
  return MAP_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::keys() const {
  VVec keys;
  for (auto& x : values) {
    s__conj(keys, x.second.first);
  }
  return LIST_VAL(keys);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::vals() const {
  VVec out;
  for (auto& x : values) {
    s__conj(out,x.second.second);
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LHash::pr_str(bool pretty) const {
  stdstr out;

  for (auto& x : values) {
    if (!out.empty()) {
      out += ",";
    }
    out += x.second.first->pr_str(pretty) + " " + x.second.second->pr_str(pretty);
  }

  return "{" + out + "}";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::eq(const d::Data* rhs) const {

  if (!(typeid(*this) == typeid(*rhs))) { return false; }

  auto const &rvs = s__ccast(LHash,rhs)->values;
  auto sz = values.size();

  if (sz != rvs.size()) { return false; }

  auto i=0;
  for (auto p=values.begin(), e=values.end(); p != e; ++p,++i) {
    auto r= rvs.find(p->first);
    if (r != rvs.end()) {
      auto ro= *r;
      if (p->second.first->equals(ro.second.first.ptr()) &&
          p->second.second->equals(ro.second.second.ptr()))
        continue;
    }
    break;
  }
  return i >= sz;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LHash::cmp(const d::Data* rhs) const {

  const LHash* rs= (typeid(*this) == typeid(*rhs))
                   ? s__ccast(LHash,rhs) : nullptr;
  auto sz = values.size();
  if (rs) {
    auto rc= rs->count();
    if (sz != rc) {
      return sz > rc ? 1 : -1;
    }
  }

  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::withMeta(d::DslValue m) const {
  return new LHash(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative::LNative(const LNative& rhs, d::DslValue m) : LFunction(m) {
  _name=rhs._name;
  fn=rhs.fn;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative::LNative(const stdstr& name, Invoker p) : LFunction(name), fn(p) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LNative::pr_str(bool pretty) const {
  return "(native)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LNative::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs) ? (this == rhs) : false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LNative::cmp(const d::Data* rhs) const {
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LNative::withMeta(d::DslValue m) const {
  return new LNative(*this, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LNative::invoke(Lisper* p, VSlice args) {
  return fn(p, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LNative::invoke(Lisper* p) {
  VVec v;
  return invoke(p, VSlice(v));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LLambda::withMeta(d::DslValue m) const {
  return new LLambda(*this,m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static int L_SEED=0;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LLambda::pr_str(bool pretty) const {
  return "(lambda)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame LLambda::bindContext(VSlice args) {
  auto fm= new d::Frame(pr_str(1), env);
  auto z=params.size();
  auto len= args.size();
  auto i=0, j=0;
  // run through parameters...
  for (; i < z; ++i) {
    auto k= params[i];
    if (k == "&") {
      // var-args, next must be the last one
      // e.g. [a b c & x]
      ASSERT1((i+1 == (z-1)));
      VVec x;
      appendAll(args,j,x);
      fm->set(params[i+1], LIST_VAL(x));
      j=len;
      i= z;
      break;
    }
    if (!(j < len))
      throw BadArity(z,len);
    fm->set(k, *(args.begin + j));
    ++j;
  }
  // make sure arg count matches param count
  ASSERT1(j==len && i== z);
  return fm;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LLambda::invoke(Lisper* p, VSlice args) {
  return p->EVAL(body, bindContext(args));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LLambda::invoke(Lisper* p) {
  VVec out;
  return invoke(p, VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda::LLambda(const StrVec& _args, d::DslValue body, d::DslFrame env)
: LFunction("anon#" + std::to_string(++L_SEED)) {
  s__ccat(params, _args);
  this->body = body;
  this->env= env;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda::LLambda(const stdstr& n,
    const StrVec& _args, d::DslValue body, d::DslFrame env) : LFunction(n) {
  s__ccat(params, _args);
  this->env=env;
  this->body=body;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda::LLambda(const LLambda& rhs, d::DslValue m) : LFunction(m) {
  _name=rhs._name;
  s__ccat(params, rhs.params);
  env=rhs.env;
  body=rhs.body;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LLambda::eq(const d::Data* rhs) const {
  if (!(typeid(*this) == typeid(*rhs))) { return false; }
  auto x= s__ccast(LLambda,rhs);
  return _name == x->_name &&
      a::equals<stdstr>(params, x->params) &&
         body.ptr() == x->body.ptr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LLambda::cmp(const d::Data* rhs) const {
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const StrVec& args, d::DslValue body, d::DslFrame env)
  : LLambda(args, body,env) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const stdstr& n, const StrVec& args, d::DslValue body, d::DslFrame env)
  : LLambda(n, args, body, env) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const LMacro& rhs, d::DslValue m) : LLambda(rhs, m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LMacro::pr_str(bool pretty) const {
  return "(macro)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LMacro::cmp(const d::Data* rhs) const {
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LMacro::eq(const d::Data* rhs) const {
  return LLambda::eq(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LMacro::withMeta(d::DslValue m) const {
  return new LMacro(*this,m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool lessThan(d::DslValue a, d::DslValue b) {
  auto p= b.ptr();
  return a->equals(p) ? false : (a->compare(p) < 0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(VSlice more) : LSet() {
  for (auto i = more.begin; i != more.end; ++i) {
    values->insert(*i);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(VVec& v) : LSet(VSlice(v)) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(d::DslValue m) : LValue(m) {
  values=new std::set<d::DslValue,SetCompare> { &lessThan};
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet() {
  values=new std::set<d::DslValue,SetCompare> { &lessThan};
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::~LSet() {
  DEL_PTR(values);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(const std::set<d::DslValue,SetCompare>& m) : LSet() {
  for (auto& x : m) {
    values->insert(x);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(const LSet& rhs, d::DslValue m) : LSet(m) {
  for (auto& x : *(rhs.values)) {
    values->insert(x);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSet::conj(VSlice more) const {
  std::set<d::DslValue,SetCompare> m(*values);
  for (auto i = more.begin; i != more.end; ++i) {
    m.insert(*i);
  }
  return SET_VAL(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSet::disj(VSlice more) const {
  std::set<d::DslValue,SetCompare> m(*values);
  for (auto i= more.begin; i != more.end; ++i) {
    m.erase(*i);
  }
  return SET_VAL(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSet::isEmpty() const { return values->empty(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSet::count() const { return values->size();  }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSet::nth(int pos) const {
  auto q= seq();
  auto s = cast_list(q,1);
  if (auto z= s->count(); z > 0 && pos >= 0 && pos < z) {
    return s->nth(pos);
  } else {
    return NIL_VAL();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSet::seq() const {
  VVec out;
  for (auto& x : *values) {
    s__conj(out, x);
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSet::first() const {
  return nth(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSet::rest() const {
  auto s = s__cast(LList,seq().ptr());
  if (auto z= s->count(); z > 1) {
    VVec out;
    for (auto i=1; i < z; ++i) {
      s__conj(out, s->nth(i));
    }
    return LIST_VAL(out);
  } else {
    return EMPTY_LIST();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSet::contains(d::DslValue key) const {
  return values->find(key) != values->end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSet::get(d::DslValue key) const {
  auto i = values->find(key);
  return (i != values->end()) ? *i : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSet::eval(Lisper* p, d::DslFrame e) {
  std::set<d::DslValue,SetCompare> out(lessThan);
  for (auto& it : *values) {
    out.insert(it);
  }
  return SET_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LSet::pr_str(bool pretty) const {
  stdstr out;

  for (auto& x : *values) {
    if (!out.empty()) {
      out += ",";
    }
    out += x->pr_str();
  }

  return "#{" + out + "}";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSet::eq(const d::Data* rhs) const {

  if (!(typeid(*this) == typeid(*rhs))) { return false; }

  auto const &rvs = s__ccast(LSet,rhs)->values;
  auto sz = values->size();

  if (sz != rvs->size()) { return false; }

  auto i=0;
  for (auto p=values->begin(), e=values->end(); p != e; ++p,++i) {
    auto r= rvs->find(*p);
    if (r != rvs->end()) {
      continue;
    }
    break;
  }
  return i >= sz;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSet::cmp(const d::Data* rhs) const {
  const LSet* rs= (typeid(*this) == typeid(*rhs))
                  ? s__ccast(LSet,rhs) : nullptr;
  auto sz = values->size();
  if (rs) {
    auto rc = rs->count();
    if (sz != rc) {
      return sz < rc ? -1 : 1;
    }
  }

  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSet::withMeta(d::DslValue m) const {
  return new LSet(*this, m);
}









//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

