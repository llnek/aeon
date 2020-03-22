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
LNative A_NATIVE;
LLambda A_LAMBDA;
LMacro A_MACRO;
LAtom A_ATOM;
LKeyword A_KEYWORD { "" };
LList A_LIST;
LVec A_VEC;
LHash A_MAP;
LFloat A_FLOAT { 0.0 };
LInt A_INT { 0 };
LChar A_CHAR {'\0'};
LNil A_NIL;
LTrue A_TRUE;
LFalse A_FALSE;
LString A_STR { "" };
LSymbol A_SYMB { "" };
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool truthy(d::DslValue v) {
  return s__cast(LValue,v.ptr())->truthy();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEqual(int wanted, int got, const stdstr& fn) {
  if (wanted != got)
    RAISE(BadArity,
          "%s requires %d args, got %d.\n", fn.c_str(), wanted, got);
  return got;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preMin(int min, int got, const stdstr& fn) {
  if (got < min)
    RAISE(BadArity,
          "%s requires %d args, got %d.\n", fn.c_str(), min, got);
  return got;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preNonZero(int c, const stdstr& fn) {
  if (c == 0)
    RAISE(BadArity,
          "%s requires some args, got %d.\n", fn.c_str(), c);
  return c;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEven(int c, const stdstr& fn) {
  if (!a::is_even(c))
    RAISE(BadArity,
          "%s requires even args, got %d.\n", fn.c_str(), c);
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(LSeqable* s, int from, VVec& out) {
  for (auto i=from, e=s->count(); i < e; ++i) {
    s__conj(out,s->nth(i));
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(LSeqable* s, VVec& out) {
  appendAll(s,0,out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(VSlice args, int from, VVec& out) {
  auto i= from;
  while ((args.begin+i) != args.end) {
    s__conj(out, *(args.begin+i));
    ++i;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue expected(const stdstr& m, d::DslValue v) {
  RAISE(BadArg,
      "Expected `%s`, got %s.\n",
      m.c_str(), v.ptr()->toString().c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr hash_key(const d::DslValue& s) {
  if (auto x = s__cast(LValue, s.ptr()); x->keyable()) {
    return x->toString();
  } else {
    return (expected("String or Keyword", s), "");
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNil* cast_nil(d::DslValue v, int panic) {
  if (auto p= v.ptr(); typeid(A_NIL)==typeid(*p)) {
    return s__cast(LNil,p);
  }
  if (panic != 0) { expected("nil", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LChar* cast_char(d::DslValue v, int panic) {
  if (auto p= v.ptr(); typeid(A_CHAR)==typeid(*p)) {
    return s__cast(LChar,p);
  }
  if (panic != 0) { expected("char", v); }
  return P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFloat* cast_float(d::DslValue v, int panic) {
  if (auto p= v.ptr(); typeid(A_FLOAT)==typeid(*p))
    if (auto r = s__cast(LFloat,p); X_NIL(r)) { return r; }
  if (panic != 0) { expected("float", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LInt* cast_int(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_INT)==typeid(*p))
    if (auto r = s__cast(LInt,p); X_NIL(r)) { return r; }
  if (panic) { expected("int", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol* cast_symbol(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_SYMB)==typeid(*p))
    if (auto r= s__cast(LSymbol,p); X_NIL(r)) { return r;}
  if (panic) { expected("symbol", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString* cast_string(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_STR)==typeid(*p))
    if (auto r= s__cast(LString,p); X_NIL(r)) { return r; }
  if (panic) { expected("string", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword* cast_keyword(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_KEYWORD)==typeid(*p))
    if (auto r=s__cast(LKeyword,p); X_NIL(r)) { return r;}
  if (panic) { expected("keyword", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList* cast_list(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_LIST)==typeid(*p))
    if (auto r= s__cast(LList,p); X_NIL(r)) { return r;}
  if (panic) { expected("list", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec* cast_vec(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_VEC)==typeid(*p))
    if (auto r= s__cast(LVec,p); X_NIL(r)) { return r; }
  if (panic) { expected("vector", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential* cast_seq(d::DslValue v, int panic) {
  if (auto r= cast_vec(v,0); X_NIL(r)) { return s__cast(LSequential,r); }
  if (auto r= cast_list(v,0); X_NIL(r)) { return s__cast(LSequential,r); }
  if (panic) { expected("sequenctial", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeqable* cast_seqable(d::DslValue v, int panic) {
  if (auto r= cast_vec(v,0); X_NIL(r)) { return s__cast(LSeqable,r); }
  if (auto r= cast_map(v,0); X_NIL(r)) { return s__cast(LSeqable,r); }
  if (auto r= cast_list(v,0); X_NIL(r)) { return s__cast(LSeqable,r); }
  if (auto r= cast_string(v,0); X_NIL(r)) { return s__cast(LSeqable,r); }
  if (panic) { expected("seq'qble", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash* cast_map(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_MAP)==typeid(*p))
    if (auto r= s__cast(LHash,p); X_NIL(r)) { return r; }
  if (panic) { expected("hashmap", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambda* cast_lambda(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_LAMBDA)==typeid(*p))
    if (auto r= s__cast(LLambda,p); X_NIL(r)) { return r; }
  if (panic) { expected("lambda", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro* cast_macro(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_MACRO)==typeid(*p))
    if (auto r=s__cast(LMacro,p); X_NIL(r)) { return r; }
  if (panic) { expected("macro", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative* cast_native(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_NATIVE)==typeid(*p))
    if (auto r=s__cast(LNative, p); X_NIL(r)) { return r;}
  if (panic) { expected("native", v); }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFunction* cast_function(d::DslValue v, int panic) {
  LFunction* f= P_NIL;
  if (auto a= cast_lambda(v,0); X_NIL(a)) {
    f= s__cast(LFunction,a);
  }
  else
  if (auto a= cast_macro(v,0); X_NIL(a)) {
    f= s__cast(LFunction,a);
  }
  else
  if (auto a= cast_native(v,0); X_NIL(a)) {
    f= s__cast(LFunction,a);
  }

  if (E_NIL(f) && panic) {
    expected("Function", v);
  }

  return f;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LAtom* cast_atom(d::DslValue v, int panic) {
  if (auto p=v.ptr(); typeid(A_ATOM)==typeid(*p)) {
    return s__cast(LAtom,p);
  }
  if (panic) expected("Atom",v);
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool cast_numeric(VSlice vs, NumberSlots& out) {
  auto r=false;
  for (auto i= 0; (vs.begin+i) != vs.end; ++i) {
    auto x= *(vs.begin+i);
    auto f= cast_float(x);
    auto n= cast_int(x);
    if (E_NIL(n) && E_NIL(f)) {
      expected("numeric", x);
    }
    if (n) {
      s__conj(out, d::NumberSlot(n->impl()));
    }
    else
    if (f) {
      r= true;
      s__conj(out, d::NumberSlot(f->impl()));
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
d::DslValue LValue::withMeta(d::DslValue& m) const {
  RAISE(d::SemanticError,"%s: not supported here.", "with-meta");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LValue::meta() const {
  return metaObj.isNull() ? nil_value() : metaObj;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LValue::eval(Lisper*, d::DslFrame e) {
  // by default, eval to self.
  return d::DslValue(this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LValue::equals(const d::Data* rhs) const {
  return X_NIL(rhs) && eq(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LFalse::toString(bool p) const {
  return "false";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LFalse::withMeta(d::DslValue& m) const {
  return d::DslValue(new LFalse(*this,m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LFalse::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFalse::LFalse(const LFalse& rhs, const d::DslValue& m) : LValue(m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LTrue::LTrue(const LTrue& rhs, const d::DslValue& m) : LValue(m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LTrue::toString(bool p) const  {
  return "true";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LTrue::withMeta(d::DslValue& m) const {
  return d::DslValue(new LTrue(*this,m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LTrue::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNil::LNil(const LNil& rhs, const d::DslValue& m) : LValue(m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LNil::toString(bool p) const  {
  return "nil";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LNil::withMeta(d::DslValue& m) const {
  return d::DslValue(new LNil(*this,m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LNil::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LFloat::withMeta(d::DslValue& m) const {
  return d::DslValue(new LFloat(*this, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::NumberSlot LFloat::number() const {
  return d::NumberSlot(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFloat::LFloat(const LFloat& rhs, const d::DslValue& m) : LValue (m) {
  value = rhs.value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LFloat::toString(bool pretty) const {
  return std::to_string(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LFloat::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) &&
         value == s__cast(const LFloat,rhs)->value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LInt::withMeta(d::DslValue& m) const {
  return d::DslValue(new LInt(*this, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::NumberSlot LInt::number() const {
  return d::NumberSlot(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LInt::LInt(const LInt& rhs, const d::DslValue& m) : LValue (m) {
  value = rhs.value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LInt::toString(bool pretty) const {
  return std::to_string(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LInt::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) &&
         value == s__cast(const LInt,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LChar::toString(bool p) const {
  return stdstr { value };
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LChar::LChar(const LChar& rhs, const d::DslValue& m) : LValue(m) {
  value = rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LChar::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) &&
         value == s__cast(const LChar,rhs)->value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LChar::withMeta(d::DslValue& m) const {
  return d::DslValue(new LChar(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LAtom::toString(bool p) const {
  return "atom(" + value->toString(p) + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LAtom::LAtom(d::DslValue v) : value(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LAtom::LAtom(const LAtom& rhs, const d::DslValue& m) : LValue(m) {
  value= rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LAtom::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) && value->equals(rhs);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LAtom::withMeta(d::DslValue& m) const {
  return d::DslValue(new LAtom(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString::LString(const LString& rhs, const d::DslValue& m) : LValue(m) {
  value=rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString::LString(const stdstr& s) : value(s) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LString::toString(bool p) const {
  return p ? encoded() : value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LString::encoded() const {
  return escape(value);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LString::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs) &&
         value == s__cast(const LString,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::withMeta(d::DslValue& m) const {
  return d::DslValue(new LString(*this, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LString::isEmpty() const { return value.empty(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LString::count() const { return value.size(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::first() const {
  if (value.size() == 0) { return nil_value(); }
  return char_value(value[0]);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::rest() const {
  if (value.size() == 0) {
    return empty_list();
  } else {
    stdstr s { value.c_str() + 1 };
    return LString(s).seq();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LString::contains(const d::DslValue& key) const {
  auto n= cast_int(key,1)->impl();
  return n >= 0 && n < value.size();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::seq() const {
  VVec out;
  for (auto i=value.begin(), e=value.end(); i != e; ++i) {
    s__conj(out, char_value(*i));
  }
  return list_value(VSlice(out));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::nth(int pos) const {
  if (pos < 0 || pos >= value.size()) {
    RAISE(IndexOOB, "Index out of range: %d.\n", pos);
  }
  return char_value(value[pos]);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(const stdstr& s) {
  int del=127;
  char c = (char) del;
  value = stdstr { c } + s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(const LKeyword& rhs, const d::DslValue& m) : LValue(m) {
  value= rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LKeyword::toString(bool p) const {
  stdstr s { value };
  s[0]= ':';
  return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LKeyword::eq(const d::Data* rhs) const {
  return typeid(*this)==typeid(*rhs) &&
         value == s__cast(const LKeyword,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LKeyword::withMeta(d::DslValue& m) const {
  return d::DslValue(new LKeyword(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol::LSymbol(const LSymbol& rhs, const d::DslValue& m) : LValue(m) {
  value=rhs.value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol::LSymbol(const stdstr& s) : value(s) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LSymbol::toString(bool p) const {
  return value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSymbol::eval(Lisper*, d::DslFrame e) {
  if (auto r= e->get(value); r.isSome()) {
    return r;
  }
  RAISE(NoSuchVar, "No such symbol %s.\n", value.c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSymbol::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs) &&
         value == s__cast(const LSymbol,rhs)->value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSymbol::withMeta(d::DslValue& m) const {
  return d::DslValue(new LSymbol(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential::LSequential(const LSequential& rhs, const d::DslValue& m) : LValue(m) {
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
stdstr LSequential::toString(bool pretty) const {
  stdstr out;
  for (auto b = values.cbegin(), e=values.cend(); b != e; ++b) {
    if (!out.empty()) { out += " "; }
    out += (*b)->toString(pretty);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void LSequential::evalEach(Lisper* e, d::DslFrame env, VVec& out) const {
  for (auto& i : values) {
    if (auto r= e->EVAL(i, env); r.isSome()) {
      s__conj(out, r);
    } else {
      RAISE(BadEval, "%s.\n", i->toString(true).c_str());
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::isEmpty() const { return values.empty(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSequential::count() const { return values.size(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSequential::nth(int pos) const {
  return (pos >= 0 && pos < values.size()) ? values[pos] : nil_value();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::eq(const d::Data* v) const {
  const LSequential* rhs = P_NIL;
  if (typeid(A_LIST) == typeid(*v)) {
    rhs= s__cast(const LSequential,v);
  }
  else if (typeid(A_VEC) == typeid(*v)) {
    rhs= s__cast(const LSequential,v);
  }
  if (auto sz = count(); rhs && sz == rhs->count()) {
    for (auto i = 0; i < sz; ++i) {
      if (!(nth(i)->equals(rhs->nth(i).ptr())))
      return false;
    }
    return true;
  }
  return false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSequential::first() const {
  return count() == 0 ? nil_value() : nth(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSequential::rest() const {
  VVec out;
  if (count() > 0)
    for (auto b=values.cbegin()+1,e=values.cend(); b != e; ++b) {
      s__conj(out, *b);
    }
  return list_value(VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSequential::seq() const {
  auto p = const_cast<LSequential*>(this);
  return list_value(VSlice(p->values));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSequential::contains(const d::DslValue& key) const {
  auto n= cast_int(key,1)->impl();
  return n >= 0 && n < count();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(const LList& rhs, const d::DslValue& m) : LSequential(rhs, m) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(VSlice v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(VVec& v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LList::toString(bool pretty) const {
  return "(" + LSequential::toString(pretty) + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LList::eval(Lisper* e, d::DslFrame env) {
  if (values.size() == 0) { return d::DslValue(this); }
  VVec out;
  evalEach(e, env, out);
  return list_value(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LList::conj(VSlice chunk) const {
  VVec out;
  std::reverse_copy(chunk.begin, chunk.end, out.end());
  std::copy(values.begin(), values.end(), out.end());
  return list_value(VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LList::withMeta(d::DslValue& m) const {
  return d::DslValue(new LList(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(const LVec& rhs, const d::DslValue& m) : LSequential(rhs, m) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(VSlice v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(VVec& v) : LSequential(v) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LVec::eval(Lisper* e, d::DslFrame env) {
  VVec out;
  evalEach(e,env, out);
  return vector_value(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LVec::toString(bool pretty) const {
  return "[" + LSequential::toString(pretty) + "]";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LVec::conj(VSlice more) const {
  VVec out;
  std::copy(values.begin(), values.end(), out.end());
  std::copy(more.begin, more.end, out.end());
  return vector_value(VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LVec::withMeta(d::DslValue& m) const {
  return d::DslValue(new LVec(*this, m));
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
LHash::LHash() {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(const std::map<stdstr,VPair>& m) {
  for (auto& x : m) {
    values[x.first] = x.second;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(const LHash& rhs, const d::DslValue& m) : LValue(m) {
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
  return map_value(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::dissoc(VSlice more) const {
  std::map<stdstr,VPair> m(values);
  for (auto i= more.begin; i != more.end; ++i) {
    m.erase(hash_key(*i));
  }
  return map_value(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::isEmpty() const { return values.empty(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LHash::count() const { return values.size();  }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::nth(int pos) const {
  auto s = s__cast(LList,seq().ptr());
  if (auto z= s->count(); z > 0 && pos >= 0 && pos < z) {
    return s->nth(pos);
  } else {
    return nil_value();
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::seq() const {
  VVec out;
  for (auto x=values.begin(), e=values.end(); x != e; ++x) {
    auto p= x->second;
    VVec tmp { p.first, p.second };
    s__conj(out, vector_value(VSlice(tmp)));
  }
  return list_value(VSlice(out));
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
    return list_value(VSlice(out));
  } else {
    return empty_list();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::contains(const d::DslValue& key) const {
  return values.find(hash_key(key)) != values.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::get(d::DslValue& key) const {
  auto i = values.find(hash_key(key));
  if (i != values.end()) {
    return i->second.second;
  } else {
    return nil_value();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::eval(Lisper* p, d::DslFrame e) {
  std::map<stdstr,VPair> out;
  for (auto& it : values) {
    out[it.first] = HASH_VAL(p->EVAL(it.second.first,e),
                             p->EVAL(it.second.second, e));
  }
  return d::DslValue(new LHash(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::keys() const {
  VVec keys;
  for (auto& x : values) {
    s__conj(keys, x.second.first);
  }
  return list_value(VSlice(keys));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::vals() const {
  VVec out;
  for (auto& x : values) {
    s__conj(out,x.second.second);
  }
  return list_value(VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LHash::toString(bool pretty) const {
  stdstr out;

  for (auto& x : values) {
    if (!out.empty()) {
      out += ",";
    }
    out += x.second.first->toString(pretty) + " " + x.second.second->toString(pretty);
  }

  return "{" + out + "}";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::eq(const d::Data* rhs) const {
  if (!(typeid(*this) == typeid(*rhs))) { return false; }
  auto const &rvs = s__cast(const LHash,rhs)->values;

  if (values.size() != rvs.size()) { return false; }

  for (auto i = values.begin(),
            e = values.end(),
            j = rvs.begin(); i != e; ++i,++j) {
    if ( (*i).second.first != (*j).second.first) { return false; }
    if (! (*i).second.second->equals((*j).second.second.ptr())) { return false; }
  }

  return true;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::withMeta(d::DslValue& m) const {
  return d::DslValue(new LHash(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative::LNative(const LNative& rhs, const d::DslValue& m) : LFunction(m) {
  _name=rhs._name;
  fn=rhs.fn;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNative::LNative(const stdstr& name, Invoker* p) : LFunction(name), fn(p) {

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LNative::toString(bool pretty) const {
  return "(native)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LNative::eq(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs) ? (this == rhs) : false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LNative::withMeta(d::DslValue& m) const {
  return d::DslValue(new LNative(*this, m));
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
d::DslValue LLambda::withMeta(d::DslValue& m) const {
  return d::DslValue(new LLambda(*this,m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static int L_SEED=0;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LLambda::toString(bool pretty) const {
  return "(lambda)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame LLambda::bindContext(VSlice args) {
  auto fm= new d::Frame(toString(true), env);
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
      fm->set(params[i+1], list_value(VSlice(x)),true);
      j=len;
      i= z;
      break;
    }
    ASSERT1(j < len);
    fm->set(k, *(args.begin + j), true);
    ++j;
  }
  // make sure arg count matches param count
  ASSERT1(j==len && i== z);
  return d::DslFrame(fm);
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
LLambda::LLambda(const LLambda& rhs, const d::DslValue& m) : LFunction(m) {
  _name=rhs._name;
  s__ccat(params, rhs.params);
  env=rhs.env;
  body=rhs.body;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LLambda::eq(const d::Data* rhs) const {
  if (!(typeid(*this) == typeid(*rhs))) { return false; }
  auto x= s__cast(const LLambda,rhs);
  return _name == x->_name &&
      a::equals<stdstr>(params, x->params) &&
         body.ptr() == x->body.ptr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const StrVec& args, d::DslValue body, d::DslFrame env)
  : LLambda(args, body,env) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const stdstr& n, const StrVec& args, d::DslValue body, d::DslFrame env)
  : LLambda(n, args, body,env) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro::LMacro(const LMacro& rhs, const d::DslValue& m)
  : LLambda(rhs, m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LMacro::toString(bool pretty) const {
  return "(macro)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LMacro::eq(const d::Data* rhs) const {
  return LLambda::eq(rhs);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LMacro::withMeta(d::DslValue& m) const {
  return d::DslValue(new LMacro(*this,m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

