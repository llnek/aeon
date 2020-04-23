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
#define TO_VAL(x) DCAST(LValue,x)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative A_NATIVE;
LLambda A_LAMBDA;
LMacro A_MACRO;
LAtom A_ATOM;
LKeyword A_KEYWORD;
LList A_LIST;
LVec A_VEC;
LHash A_MAP;
LSet A_SET;
LNumber  A_NUMBER {0};
LChar A_CHAR;
LNil A_NIL;
LTrue A_TRUE;
LFalse A_FALSE;
LString A_STR;
LSymbol A_SYMB;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool is_same(const d::Data* lhs, const d::Data* rhs) {
  return typeid(*lhs) == typeid(*rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool truthy(d::DValue v) { return TO_VAL(v)->truthy(); }

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
d::DValue expected(const stdstr& m, d::DValue v) {
  RAISE(d::BadArg,
        "Expected `%s`, got %s.\n", C_STR(m), C_STR(v->pr_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr hash_key(d::DValue s) {
  if (!s) {
    RAISE(a::NPError, "Failed to hash key for map.%s","\n");
  }
  return s->pr_str();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CASTXXX(T,v,panic,object,msg) do { \
  if (auto p= v.get(); typeid(object)==typeid(*p)) { return s__cast(T,p); } \
  if (panic) expected(msg, v); \
  return P_NIL; } while (0)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNil* cast_nil(d::DValue v, int panic) {
  CASTXXX(LNil,v,panic,A_NIL,"nil");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LChar* cast_char(d::DValue v, int panic) {
  CASTXXX(LChar,v,panic,A_CHAR,"char");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNumber* cast_number(d::DValue v, int panic) {
  CASTXXX(LNumber,v,panic,A_NUMBER,"number");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol* cast_symbol(d::DValue v, int panic) {
  CASTXXX(LSymbol,v,panic,A_SYMB,"symbol");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString* cast_string(d::DValue v, int panic) {
  CASTXXX(LString,v,panic,A_STR,"string");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword* cast_keyword(d::DValue v, int panic) {
  CASTXXX(LKeyword,v,panic,A_KEYWORD,"keyword");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList* cast_list(d::DValue v, int panic) {
  CASTXXX(LList,v,panic,A_LIST,"list");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec* cast_vec(d::DValue v, int panic) {
  CASTXXX(LVec,v,panic,A_VEC,"vector");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet* cast_set(d::DValue v, int panic) {
  CASTXXX(LSet,v,panic,A_SET,"set");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash* cast_map(d::DValue v, int panic) {
  CASTXXX(LHash,v,panic,A_MAP,"hashmap");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda* cast_lambda(d::DValue v, int panic) {
  CASTXXX(LLambda,v,panic,A_LAMBDA,"lambda");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro* cast_macro(d::DValue v, int panic) {
  CASTXXX(LMacro,v,panic,A_MACRO,"macro");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative* cast_native(d::DValue v, int panic) {
  CASTXXX(LNative,v,panic,A_NATIVE,"native");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LAtom* cast_atom(d::DValue v, int panic) {
  CASTXXX(LAtom,v,panic,A_ATOM,"atom");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential* cast_sequential(d::DValue v, int panic) {
  if (auto r= cast_list(v); r) return s__cast(LSequential,r);
  if (auto r= cast_vec(v); r) return s__cast(LSequential,r);
  if (panic) expected("sequenctial", v);
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeqable* cast_seqable(d::DValue v, int panic) {
  if (auto r= cast_vec(v); r) { return s__cast(LSeqable,r); }
  if (auto r= cast_set(v); r) { return s__cast(LSeqable,r); }
  if (auto r= cast_map(v); r) { return s__cast(LSeqable,r); }
  if (auto r= cast_list(v); r) { return s__cast(LSeqable,r); }
  if (auto r= cast_string(v); r) { return s__cast(LSeqable,r); }
  if (panic) expected("seq'qble", v);
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFunction* cast_function(d::DValue v, int panic) {
  if (auto a= cast_lambda(v); a) return s__cast(LFunction,a);
  if (auto a= cast_macro(v); a) return s__cast(LFunction,a);
  if (auto a= cast_native(v); a) return s__cast(LFunction,a);

  if (panic)
    expected("function", v);

  return NULL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool scan_numbers(d::VSlice vs) {
  auto r=false;
  for (auto i= 0; (vs.begin+i) != vs.end; ++i) {
    auto x= *(vs.begin+i);
    auto n= cast_number(x,0);
    if (E_NIL(n)) { expected("number", x); }
    if (!n->isInt()) { r= true; }
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
d::DValue LValue::withMeta(d::DValue m) const {
  RAISE(d::Unsupported, "%s: not supported here.", "with-meta");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LValue::compare(d::DValue rhs) const {
  ASSERT1(rhs);
  return cmp(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LValue::equals(d::DValue rhs) const {
  ASSERT1(rhs);
  return eq(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LFalse::cmp(d::DValue rhs) const {
  return d::is_same(rhs, this)
         ? 0 : pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LFalse::pr_str(bool p) const {
  return "false";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LFalse::withMeta(d::DValue m) const {
  return d::DValue(new LFalse(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LFalse::eq(d::DValue rhs) const {
  return d::is_same(rhs, this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFalse::LFalse(const LFalse&, d::DValue m) : LValue(m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LTrue::LTrue(const LTrue&, d::DValue m) : LValue(m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LTrue::pr_str(bool p) const  { return "true"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LTrue::withMeta(d::DValue m) const {
  return d::DValue(new LTrue(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LTrue::cmp(d::DValue rhs) const {
  return d::is_same(rhs, this)
         ? 0 : pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LTrue::eq(d::DValue rhs) const {
  return d::is_same(rhs,this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNil::LNil(const LNil&, d::DValue m) : LValue(m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LNil::pr_str(bool p) const  { return "nil"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LNil::withMeta(d::DValue m) const {
  return d::DValue(new LNil(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LNil::eq(d::DValue rhs) const {
  return d::is_same(rhs, this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LNil::cmp(d::DValue rhs) const {
  return d::is_same(rhs, this)
         ? 0 : pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LNumber::withMeta(d::DValue m) const {
  return d::DValue(new LNumber(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LNumber::eval(Lisper*, d::DFrame) {
  return d::DValue(new LNumber(*this, d::DValue(P_NIL)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNumber::LNumber(const LNumber& rhs, d::DValue m) : LValue (m) {
  _type= rhs._type;
  num = rhs.num;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNumber::LNumber(double d) {
  _type= d::T_REAL;
  num.r=d;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNumber::LNumber(llong n) {
  _type= d::T_INTEGER;
  num.n=n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNumber::LNumber(int n) {
  _type= d::T_INTEGER;
  num.n=n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LNumber::pr_str(bool) const {
  return isInt() ? N_STR(num.n) : N_STR(num.r);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LNumber::eq(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    auto p= cast_number(rhs,1);
    return isInt() == p->isInt() &&
         a::fuzzy_equals(getFloat(), p->getFloat());
  } else {
    return false;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LNumber::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    auto f= cast_number(rhs,1)->getFloat();
    auto f2= getFloat();
    return a::fuzzy_equals(f, f2) ? 0 : (f2 > f ? 1 : -1);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LChar::pr_str(bool p) const {
  return stdstr { value };
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LChar::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    auto c = s__ccast(LChar, rhs.get())->value;
    return value==c ? 0 : value > c ? 1 : -1;
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LChar::LChar(const LChar& rhs, d::DValue m) : LValue(m) {
  value = rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LChar::eq(d::DValue rhs) const {
  return d::is_same(rhs, this) &&
         value == cast_char(rhs,1)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LChar::withMeta(d::DValue m) const {
  return d::DValue(new LChar(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LAtom::pr_str(bool p) const {
  return "atom(" + value->pr_str(p) + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LAtom::LAtom(d::DValue v) : value(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LAtom::LAtom(const LAtom& rhs, d::DValue m) : LValue(m) {
  value= rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LAtom::eq(d::DValue rhs) const {
  return d::is_same(rhs, this) && value->equals(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LAtom::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    return value->compare(cast_atom(rhs,1)->value);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LAtom::withMeta(d::DValue m) const {
  return d::DValue(new LAtom(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString::LString(const LString& rhs, d::DValue m) : LValue(m) {
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
bool LString::eq(d::DValue rhs) const {
  return d::is_same(rhs, this) &&
         value == cast_string(rhs,1)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LString::cmp(d::DValue rhs) const {
  if (d::is_same(rhs, this)) {
    return value.compare(cast_string(rhs,1)->value);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LString::withMeta(d::DValue m) const {
  return d::DValue(new LString(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LString::isEmpty() const { return value.empty(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LString::count() const { return value.size(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LString::first() const {
  return value.size() == 0 ? NIL_VAL() : CHAR_VAL(value[0]);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LString::rest() const {
  if (value.size() == 0) {
    return EMPTY_LIST();
  } else {
    stdstr s { value.c_str() + 1 };
    return LString(s).seq();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LString::contains(d::DValue key) const {
  auto n= cast_number(key,1)->getInt();
  return n >= 0 && n < value.size();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LString::seq() const {
  d::ValVec out;
  for (auto i=value.begin(), e=value.end(); i != e; ++i) {
    s__conj(out, CHAR_VAL(*i));
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LString::nth(int pos) const {
  if (pos < 0 || pos >= value.size()) {
    RAISE(d::IndexOOB, "Index out of range: %d.\n", pos);
  }
  return CHAR_VAL(value[pos]);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(const LKeyword& rhs, d::DValue m) : LValue(m) {
  value= rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(const stdstr& s) {
  int del=127;
  char c = (char) del;
  value = stdstr { c } + s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LKeyword::pr_str(bool p) const {
  stdstr s { value };
  s[0]= ':';
  return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LKeyword::eq(d::DValue rhs) const {
  return d::is_same(rhs, this) &&
         value == cast_keyword(rhs,1)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LKeyword::cmp(d::DValue rhs) const {
  if (d::is_same(rhs,this)) {
    return value.compare(cast_keyword(rhs,1)->value);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LKeyword::withMeta(d::DValue m) const {
  return d::DValue(new LKeyword(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol::LSymbol(const LSymbol& rhs, d::DValue m) : LValue(m) {
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
d::DValue LSymbol::eval(Lisper*, d::DFrame e) {
  if (auto r= e->get(value); r) {
    return r;
  }
  RAISE(d::NoSuchVar, "No such symbol %s.\n", C_STR(value));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSymbol::eq(d::DValue rhs) const {
  return d::is_same(rhs,this) &&
         value == cast_symbol(rhs,1)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSymbol::cmp(d::DValue rhs) const {
  if (d::is_same(rhs,this)) {
    return value.compare(cast_symbol(rhs,1)->value);
  } else {
    return pr_str().compare(rhs->pr_str());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSymbol::withMeta(d::DValue m) const {
  return d::DValue(new LSymbol(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential::LSequential(const LSequential& rhs, d::DValue m) : LValue(m) {
  s__ccat(values, rhs.values);
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
stdstr LSequential::pr_str(bool pretty) const {
  stdstr out;
  for (auto b = values.cbegin(), e=values.cend(); b != e; ++b) {
    if (!out.empty()) { out += " "; }
    out += (*b)->pr_str(pretty);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void LSequential::evalEach(Lisper* e, d::DFrame env, d::ValVec& out) const {
  for (auto& i : values) {
    if (auto r= e->EVAL(i, env); r) {
      s__conj(out, r);
    } else {
      RAISE(d::BadEval, "%s.\n", C_STR(i->pr_str(1)));
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::isEmpty() const { return values.empty(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSequential::count() const { return values.size(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSequential::nth(int pos) const {
  return (pos >= 0 && pos < values.size()) ? values[pos] : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::eq(d::DValue rhs) const {
  const LSequential* p = P_NIL;
  auto sz= count();
  if (d::is_same(rhs,&A_LIST)) {
    p= cast_sequential(rhs,1);
  }
  else
  if (d::is_same(rhs,&A_VEC)) {
    p= cast_sequential(rhs,1);
  }
  if (E_NIL(p) || sz != p->count()) {
    return false;
  }
  //ok,let's try
  auto i=0; for (; i < sz; ++i) {
    if (!(nth(i)->equals(p->nth(i))))
    break;
  }
  return i >= sz;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSequential::cmp(d::DValue rhs) const {
  const LSequential* p = P_NIL;
  auto sz= count();
  if (d::is_same(rhs,&A_LIST)) {
    p= cast_sequential(rhs,1);
  }
  else
  if (d::is_same(rhs,&A_VEC)) {
    p= cast_sequential(rhs,1);
  }
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
  auto n= cast_number(key,1)->getInt();
  return n >= 0 && n < count();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(const LList& rhs, d::DValue m) : LSequential(rhs, m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::VSlice v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::ValVec& v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::DValue v) { s__conj(values,v); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::DValue v1,d::DValue v2) {
  s__conj(values,v1);
  s__conj(values,v2);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(d::DValue v1,d::DValue v2, d::DValue v3) {
  s__conj(values,v1);
  s__conj(values,v2);
  s__conj(values,v3);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LList::pr_str(bool pretty) const {
  return "(" + LSequential::pr_str(pretty) + ")";
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
    for (auto i= 1; (args.begin != args.end-i); ++i) {
      s__conj(out, *(args.end-i));
    }
    s__conj(out, *args.begin);
  }
  s__ccat(out, values);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LList::withMeta(d::DValue m) const {
  return d::DValue(new LList(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(const LVec& rhs, d::DValue m) : LSequential(rhs, m) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(d::VSlice v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(d::ValVec& v) : LSequential(v) {}

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
stdstr LVec::pr_str(bool pretty) const {
  return "[" + LSequential::pr_str(pretty) + "]";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LVec::conj(d::VSlice more) const {
  d::ValVec out;
  s__ccat(out,values);
  if (more.size() > 0) {
    for (auto i=0; more.begin+i != more.end; ++i) {
      s__conj(out, *(more.begin+i));
    }
  }
  return VEC_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LVec::withMeta(d::DValue m) const {
  return d::DValue(new LVec(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(d::VSlice more) {
  int c = more.size();
  ASSERT(a::is_even(more.size()),
         "Expected even n# of args, got %d", c);
  for (auto i = more.begin; i != more.end; i += 2) {
    values[hash_key(*i)] = HASH_VAL(*i, *(i+1));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(d::ValVec& v) : LHash(d::VSlice(v)) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash() {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(const std::map<stdstr,VPair>& m) {
  for (auto& x : m) {
    values[x.first] = x.second;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(const LHash& rhs, d::DValue m) : LValue(m) {
  for (auto& x : rhs.values) {
    values[x.first]= x.second;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::assoc(d::VSlice more) const {
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
d::DValue LHash::dissoc(d::VSlice more) const {
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
d::DValue LHash::nth(int pos) const {
  auto q= seq();
  auto s = cast_list(q,1);
  if (auto z= s->count(); z > 0 && pos >= 0 && pos < z) {
    return s->nth(pos);
  } else {
    return NIL_VAL();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::seq() const {
  d::ValVec out;
  for (auto x=values.begin(), e=values.end(); x != e; ++x) {
    auto p= x->second;
    s__conj(out, VEC_VAL2(p.first, p.second));
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::first() const {
  return nth(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::rest() const {
  auto s = s__cast(LList,seq().get());
  if (auto z= s->count(); z > 1) {
    d::ValVec out;
    for (auto i=1; i < z; ++i) {
      s__conj(out, s->nth(i));
    }
    return LIST_VAL(out);
  } else {
    return EMPTY_LIST();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::contains(d::DValue key) const {
  return s__contains(values, hash_key(key));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::get(d::DValue key) const {
  auto i = values.find(hash_key(key));
  return (i != values.end()) ? i->second.second : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::eval(Lisper* p, d::DFrame e) {
  std::map<stdstr,VPair> out;
  for (auto& it : values) {
    out[it.first] = HASH_VAL(p->EVAL(it.second.first,e),
                             p->EVAL(it.second.second, e));
  }
  return MAP_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::keys() const {
  d::ValVec keys;
  for (auto& x : values) {
    s__conj(keys, x.second.first);
  }
  return LIST_VAL(keys);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LHash::vals() const {
  d::ValVec out;
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
bool LHash::eq(d::DValue rhs) const {

  if (!d::is_same(rhs,this)) { return false; }

  auto const &rvs = cast_map(rhs,1)->values;
  auto sz = values.size();

  if (sz != rvs.size()) { return false; }

  auto i=0;
  for (auto p=values.begin(), e=values.end(); p != e; ++p,++i) {
    auto r= rvs.find(p->first);
    if (r != rvs.end()) {
      auto ro= *r;
      if (p->second.first->equals(ro.second.first) &&
          p->second.second->equals(ro.second.second))
        continue;
    }
    break;
  }
  return i >= sz;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LHash::cmp(d::DValue rhs) const {

  const LHash* rs= d::is_same(rhs,this) ? cast_map(rhs,1) : P_NIL;
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
d::DValue LHash::withMeta(d::DValue m) const {
  return d::DValue(new LHash(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative::LNative(const LNative& rhs, d::DValue m) : LFunction(m) {
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
bool LNative::eq(d::DValue rhs) const {
  return d::is_same(rhs,this) ? (this == rhs.get()) : false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LNative::cmp(d::DValue rhs) const {
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LNative::withMeta(d::DValue m) const {
  return d::DValue(new LNative(*this, m));
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
d::DValue LLambda::withMeta(d::DValue m) const {
  return d::DValue(new LLambda(*this,m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static int L_SEED=0;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LLambda::pr_str(bool pretty) const {
  return "(lambda)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame LLambda::bindContext(d::VSlice args) {
  auto fm= d::Frame::make(pr_str(1), env);
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
: LFunction("anon#" + std::to_string(++L_SEED)) {
  s__ccat(params, _args);
  this->body = body;
  this->env= env;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda::LLambda(const stdstr& n,
    const StrVec& _args, d::DValue body, d::DFrame env) : LFunction(n) {
  s__ccat(params, _args);
  this->env=env;
  this->body=body;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda::LLambda(const LLambda& rhs, d::DValue m) : LFunction(m) {
  _name=rhs._name;
  s__ccat(params, rhs.params);
  env=rhs.env;
  body=rhs.body;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LLambda::eq(d::DValue rhs) const {
  if (!d::is_same(rhs,this)) { return false; }
  auto x= cast_lambda(rhs,1);
  return _name == x->_name &&
      a::equals<stdstr>(params, x->params) &&
         body.get() == x->body.get();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LLambda::cmp(d::DValue rhs) const {
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const stdstr& n, const StrVec& args, d::DValue body, d::DFrame env)
  : LLambda(n, args, body, env) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const StrVec& args, d::DValue body, d::DFrame env)
  : LLambda(args, body,env) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const LMacro& rhs, d::DValue m) : LLambda(rhs, m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LMacro::pr_str(bool pretty) const {
  return "(macro)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LMacro::cmp(d::DValue rhs) const {
  return pr_str().compare(rhs->pr_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LMacro::eq(d::DValue rhs) const {
  return LLambda::eq(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LMacro::withMeta(d::DValue m) const {
  return d::DValue(new LMacro(*this,m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool lessThan(d::DValue a, d::DValue b) {
  return a->equals(b) ? false : (a->compare(b) < 0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(d::VSlice more) : LSet() {
  for (auto i = more.begin; i != more.end; ++i) {
    values->insert(*i);
  }
}

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
  for (auto& x : m) {
    values->insert(x);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(const LSet& rhs, d::DValue m) : LSet(m) {
  for (auto& x : *(rhs.values)) {
    values->insert(x);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::conj(d::VSlice more) const {
  std::set<d::DValue,SetCompare> m(*values);
  for (auto i = more.begin; i != more.end; ++i) {
    m.insert(*i);
  }
  return SET_VAL(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::disj(d::VSlice more) const {
  std::set<d::DValue,SetCompare> m(*values);
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
d::DValue LSet::nth(int pos) const {
  auto q= seq();
  auto s= cast_list(q,1);
  if (auto z= s->count(); z > 0 && pos >= 0 && pos < z) {
    return s->nth(pos);
  } else {
    return NIL_VAL();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::seq() const {
  d::ValVec out;
  for (auto& x : *values) {
    s__conj(out, x);
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::first() const { return nth(0); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue LSet::rest() const {
  auto s = s__cast(LList,seq().get());
  if (auto z= s->count(); z > 1) {
    d::ValVec out;
    for (auto i=1; i < z; ++i) {
      s__conj(out, s->nth(i));
    }
    return LIST_VAL(out);
  } else {
    return EMPTY_LIST();
  }
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
bool LSet::eq(d::DValue rhs) const {

  if (!d::is_same(rhs,this)) { return false; }

  auto const &rvs = cast_set(rhs,1)->values;
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
int LSet::cmp(d::DValue rhs) const {
  const LSet* rs= d::is_same(rhs,this) ? cast_set(rhs,1) : P_NIL;
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
d::DValue LSet::withMeta(d::DValue m) const {
  return d::DValue(new LSet(*this, m));
}









//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

