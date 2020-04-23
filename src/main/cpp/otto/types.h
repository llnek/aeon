#pragma once
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


#include "../dsl/dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::otto {
namespace a= czlab::aeon;
namespace d= czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define HASH_VAL(k,v) std::pair<d::DValue,d::DValue>(k,v)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef bool (*SetCompare) (d::DValue, d::DValue);
typedef std::pair<d::DValue, d::DValue> VPair;
struct Lisper;
typedef d::DValue (*Invoker) (Lisper*, d::VSlice);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define NUMBER_VAL(n) LNumber::make(n)
#define EMPTY_LIST() LList::make()
#define CHAR_VAL(c) LChar::make(c)
#define ATOM_VAL(a) LAtom::make(a)
#define FALSE_VAL() LFalse::make()
#define TRUE_VAL() LTrue::make()
#define NIL_VAL() LNil::make()
#define VEC_VAL(v) LVec::make(v)
#define VEC_VAL2(p1,p2) LVec::make(p1,p2)

#define LIST_VAL(v) LList::make(v)
#define LIST_VAL2(p1,p2) LList::make(p1,p2)
#define LIST_VAL3(p1,p2,p3) LList::make(p1,p2,p3)
#define MAP_VAL(v) LHash::make(v)
#define SET_VAL(v) LSet::make(v)
#define KEYWORD_VAL(s) LKeyword::make(s)
#define STRING_VAL(s) LString::make(s)
#define SYMBOL_VAL(s) LSymbol::make(s)
#define FN_VAL(n, f) LNative::make(n, f)
#define MACRO_VAL(n,p,b,e) LMacro::make(n,p,b,e)
#define LAMBDA_VAL(n,p,b,e) LLambda::make(n,p,b,e)
#define BOOL_VAL(b) ((b) ? TRUE_VAL() : FALSE_VAL())

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lisper {
  d::DValue macroExpand(d::DValue ast, d::DFrame env);
  d::DValue syntaxQuote(d::DValue ast, d::DFrame env);
  d::DValue evalAst(d::DValue ast, d::DFrame env);
  d::DValue EVAL(d::DValue ast, d::DFrame env);
  std::pair<int,d::DValue> READ(const stdstr&);
  stdstr PRINT(d::DValue);
  Lisper() { seed=0; }
  ~Lisper() {}

  private:

  int seed;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LValue : public d::Data {

  virtual d::DValue eval(Lisper*, d::DFrame) = 0;
  virtual d::DValue withMeta(d::DValue) const;
  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  virtual bool truthy() const { return true; }
  d::DValue meta() const { return metaObj; }
  virtual ~LValue() {}

  protected:

  virtual bool eq(d::DValue) const = 0;
  virtual int cmp(d::DValue) const = 0;

  LValue(d::DValue m) : metaObj(m) {}
  LValue() {}

  d::DValue metaObj;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSeqable {

  virtual bool contains(d::DValue) const = 0;
  virtual d::DValue first() const = 0;
  virtual d::DValue rest() const = 0;
  virtual d::DValue seq() const = 0;
  virtual d::DValue nth(int) const = 0;
  virtual bool isEmpty() const = 0;
  virtual int count() const = 0 ;
  virtual ~LSeqable() {}

  protected:

  LSeqable() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFalse : public LValue {

  static d::DValue make() { return d::DValue(new LFalse()); }

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual bool truthy() const { return false; }

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LFalse());
  }

  virtual ~LFalse() {}
  LFalse() {}

  protected:

  LFalse(const LFalse&, d::DValue);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LTrue : public LValue {

  static d::DValue make() { return d::DValue(new LTrue()); }

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LTrue());
  }

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LTrue() {}
  LTrue() {}

  protected:

  LTrue(const LTrue&, d::DValue);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNil : public LValue {

  static d::DValue make() { return d::DValue(new LNil()); }

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LNil());
  }

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual bool truthy() const { return false; }

  virtual ~LNil() {}
  LNil() {}

  protected:

  LNil(const LNil&, d::DValue);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LChar : public LValue {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LChar(value));
  }

  static d::DValue make(Tchar c) {
    return d::DValue(new LChar(c));
  }

  Tchar impl() { return value; }
  virtual ~LChar() {}

  LChar() { value=0;}

  protected:

  LChar(const LChar&, d::DValue);
  LChar(Tchar c) { value = c; }

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  Tchar value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LAtom : public LValue {

  d::DValue reset(d::DValue x) { return (value = x); }
  d::DValue deref() const { return value; }

  static d::DValue make(d::DValue v) {
    return d::DValue(new LAtom(v));
  }

  static d::DValue make() {
    return d::DValue(new LAtom());
  }

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LAtom(value));
  }

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;

  LAtom() { value= NIL_VAL();}
  virtual ~LAtom() {}

  protected:

  LAtom(const LAtom&, d::DValue);
  LAtom(d::DValue);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  d::DValue value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNumber : public LValue {

  double getFloat() const { return isInt() ? (double)num.n : num.r; }
  llong getInt() const { return isInt() ? num.n : (llong)num.r; }
  bool isInt() const { return _type == d::T_INTEGER; }

  static d::DValue make(double d) {
    return d::DValue(new LNumber(d));
  }

  static d::DValue make(llong n) {
    return d::DValue(new LNumber(n));
  }

  static d::DValue make(int n) {
    return d::DValue(new LNumber(n));
  }

  virtual d::DValue withMeta(d::DValue) const;
  virtual d::DValue eval(Lisper*, d::DFrame);
  virtual stdstr pr_str(bool p=0) const;

  virtual ~LNumber() {}
  explicit LNumber(int);

  protected:

  LNumber(const LNumber&, d::DValue);
  explicit LNumber(double);
  LNumber(llong);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  int _type;
  union {
    double r; llong n; } num;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LString : public LValue, public LSeqable {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;

  static d::DValue make(const stdstr& s) {
    return d::DValue(new LString(s));
  }

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LString(value));
  }

  virtual ~LString() {}

  stdstr encoded() const;
  stdstr impl() const { return value; }

  virtual bool contains(d::DValue) const;
  virtual d::DValue nth(int) const;
  virtual d::DValue first() const;
  virtual d::DValue rest() const;
  virtual d::DValue seq() const;
  virtual bool isEmpty() const;
  virtual int count() const;
  LString() {}

  protected:

  LString(const LString&, d::DValue);
  LString(const stdstr&);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LKeyword : public LValue {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;

  static d::DValue make(const stdstr& s) {
    return d::DValue(new LKeyword(s));
  }

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LKeyword(value));
  }

  stdstr impl() const { return value; }

  virtual ~LKeyword() {}
  LKeyword() {};

  protected:

  LKeyword(const LKeyword&, d::DValue);
  LKeyword(const stdstr&);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSymbol : public LValue {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;

  static d::DValue make(const stdstr& s) {
    return d::DValue(new LSymbol(s));
  }

  virtual d::DValue eval(Lisper*, d::DFrame);

  stdstr impl() const { return value; }
  void rename(const stdstr&);

  virtual ~LSymbol() {}
  LSymbol() {}

  protected:

  LSymbol(const LSymbol& rhs, d::DValue);
  LSymbol(const stdstr&);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSequential : public LValue, public LSeqable {

  void evalEach(Lisper*, d::DFrame, d::ValVec&) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DValue conj(d::VSlice) const = 0;

  virtual bool contains(d::DValue) const;
  virtual d::DValue nth(int) const;
  virtual d::DValue first() const;
  virtual d::DValue rest() const;
  virtual d::DValue seq() const;
  virtual bool isEmpty() const;
  virtual int count() const;

  protected:

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
  virtual ~LSequential() {}

  LSequential(const LSequential& rhs, d::DValue);
  LSequential(d::VSlice chunk);
  LSequential(d::ValVec&);
  LSequential() {}

  d::ValVec values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LList : public LSequential {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DValue eval(Lisper*, d::DFrame);

  virtual d::DValue conj(d::VSlice) const;
  virtual ~LList() {}

  static d::DValue make(d::DValue v) {
    return d::DValue(new LList(v));
  }

  static d::DValue make(d::DValue v1, d::DValue v2) {
    return d::DValue(new LList(v1, v2));
  }

  static d::DValue make(d::DValue v1, d::DValue v2, d::DValue v3) {
    return d::DValue(new LList(v1, v2, v3));
  }

  static d::DValue make(d::ValVec& v) {
    return d::DValue(new LList(v));
  }

  static d::DValue make(d::VSlice s) {
    return d::DValue(new LList(s));
  }

  static d::DValue make() {
    return d::DValue(new LList());
  }

  LList() {}

  protected:

  LList(d::DValue,d::DValue,d::DValue);
  LList(const LList& rhs, d::DValue);
  LList(d::DValue);
  LList(d::DValue,d::DValue);
  LList(d::ValVec&);
  LList(d::VSlice);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LVec : public LSequential {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DValue eval(Lisper*, d::DFrame);
  virtual d::DValue conj(d::VSlice) const;

  static d::DValue make(d::DValue v1, d::DValue v2, d::DValue v3) {
    return d::DValue(new LVec(v1, v2, v3));
  }

  static d::DValue make(d::DValue v1, d::DValue v2) {
    return d::DValue(new LVec(v1, v2));
  }

  static d::DValue make(d::DValue v1) {
    return d::DValue(new LVec(v1));
  }

  static d::DValue make(d::ValVec& v) {
    return d::DValue(new LVec(v));
  }

  static d::DValue make(d::VSlice s) {
    return d::DValue(new LVec(s));
  }

  static d::DValue make() {
    return d::DValue(new LVec());
  }

  virtual ~LVec() {}
  LVec() {}

  protected:

  LVec(d::DValue,d::DValue,d::DValue);
  LVec(const LVec& rhs, d::DValue);
  LVec(d::DValue);
  LVec(d::DValue,d::DValue);
  LVec(d::VSlice);
  LVec(d::ValVec&);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSet : public LValue, public LSeqable {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DValue eval(Lisper*, d::DFrame);
  d::DValue get(d::DValue) const;

  virtual bool contains(d::DValue) const;
  virtual d::DValue first() const;
  virtual d::DValue rest() const;
  virtual d::DValue seq() const;
  virtual d::DValue nth(int) const;
  virtual bool isEmpty() const;
  virtual int count() const;

  static d::DValue make(const std::set<d::DValue,SetCompare>& s) {
    return d::DValue(new LSet(s));
  }

  static d::DValue make(d::DValue v) {
    return d::DValue(new LSet(v));
  }

  static d::DValue make(d::VSlice s) {
    return d::DValue(new LSet(s));
  }

  static d::DValue make(d::ValVec& v) {
    return d::DValue(new LSet(v));
  }

  static d::DValue make() {
    return d::DValue(new LSet());
  }

  d::DValue conj(d::VSlice) const;
  d::DValue disj(d::VSlice) const;

  virtual ~LSet();
  LSet();

  protected:

  LSet(const std::set<d::DValue,SetCompare>&);
  LSet(const LSet& rhs, d::DValue);
  LSet(d::DValue);
  LSet(d::VSlice);
  LSet(d::ValVec&);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
  std::set<d::DValue,SetCompare>* values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LHash : public LValue, public LSeqable {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DValue eval(Lisper*, d::DFrame);
  d::DValue get(d::DValue) const;

  virtual bool contains(d::DValue) const;
  virtual d::DValue first() const;
  virtual d::DValue rest() const;
  virtual d::DValue seq() const;
  virtual d::DValue nth(int) const;
  virtual bool isEmpty() const;
  virtual int count() const;

  static d::DValue make(const std::map<stdstr, VPair>& s) {
    return d::DValue(new LHash(s));
  }

  static d::DValue make(d::VSlice s) {
    return d::DValue(new LHash(s));
  }

  static d::DValue make(d::ValVec& v) {
    return d::DValue(new LHash(v));
  }

  static d::DValue make() {
    return d::DValue(new LHash());
  }

  d::DValue dissoc(d::VSlice) const;
  d::DValue assoc(d::VSlice) const;

  d::DValue keys() const;
  d::DValue vals() const;

  virtual ~LHash() {}
  LHash();

  protected:

  LHash(const std::map<stdstr, VPair>&);
  LHash(const LHash& rhs, d::DValue);
  LHash(d::VSlice);
  LHash(d::ValVec&);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
  std::map<stdstr,VPair> values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFunction : public LValue {

  virtual d::DValue invoke(Lisper*, d::VSlice) = 0;
  virtual d::DValue invoke(Lisper*) = 0;

  stdstr name() const { return _name; }

  protected:

  virtual ~LFunction() {}

  stdstr _name;
  LFunction(const stdstr& n) : _name(n) {}
  LFunction(d::DValue m) : LValue(m) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LLambda : public LFunction {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LLambda() {}

  virtual d::DValue invoke(Lisper*, d::VSlice);
  virtual d::DValue invoke(Lisper*);

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LLambda(*this, d::DValue(P_NIL)));
  }

  static d::DValue make(const stdstr& n, const StrVec& s, d::DValue v, d::DFrame f) {
    return d::DValue(new LLambda(n, s, v, f));
  }

  static d::DValue make(const StrVec& s, d::DValue v, d::DFrame f) {
    return d::DValue(new LLambda(s, v, f));
  }

  static d::DValue make() {
    return d::DValue(new LLambda());
  }

  d::DFrame bindContext(d::VSlice);
  LLambda() : LFunction("") {}

  d::DValue body;
  StrVec params;
  d::DFrame env;

  protected:

  LLambda(const stdstr&, const StrVec&, d::DValue, d::DFrame);
  LLambda(const StrVec&, d::DValue, d::DFrame);
  LLambda(const LLambda&, d::DValue);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LMacro : public LLambda {

  static d::DValue make(const stdstr& n, const StrVec& s, d::DValue v, d::DFrame f) {
    return d::DValue(new LMacro(n, s, v, f));
  }

  static d::DValue make(const StrVec& s, d::DValue v, d::DFrame f) {
    return d::DValue(new LMacro(s, v, f));
  }

  static d::DValue make() {
    return d::DValue(new LMacro());
  }

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LMacro(*this, d::DValue(P_NIL)));
  }

  virtual ~LMacro() {}

  virtual d::DValue withMeta(d::DValue) const;
  virtual bool isMacro() const { return true; }
  virtual stdstr pr_str(bool p=0) const;
  LMacro() {}

  protected:

  LMacro(const stdstr&, const StrVec&, d::DValue, d::DFrame);
  LMacro(const StrVec&, d::DValue, d::DFrame);
  LMacro(const LMacro&, d::DValue);

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNative : public LFunction {

  virtual d::DValue withMeta(d::DValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LNative() {}

  virtual d::DValue invoke(Lisper*, d::VSlice);
  virtual d::DValue invoke(Lisper*);

  static d::DValue make(const stdstr& name, Invoker f) {
    return d::DValue(new LNative(name, f));
  }

  static d::DValue make() {
    return d::DValue(new LNative());
  }

  virtual d::DValue eval(Lisper*, d::DFrame) {
    return d::DValue(new LNative(*this, d::DValue(P_NIL)));
  }

  LNative() : LFunction("") { S_NIL(fn); }

  protected:

  LNative(const stdstr& name, Invoker);
  LNative(const LNative&, d::DValue);

  Invoker fn;

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(const stdstr&, d::DValue);
void appendAll(d::VSlice, int, d::ValVec&);
void appendAll(d::VSlice, int, int, d::ValVec&);
void appendAll(LSeqable*, d::ValVec&);
void appendAll(LSeqable*, int, d::ValVec&);
void appendAll(LSeqable*, int, int, d::ValVec&);
bool truthy(d::DValue);

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool scan_numbers(d::VSlice);
LAtom* cast_atom(d::DValue, int panic=0);
LNil* cast_nil(d::DValue, int panic=0);
LHash* cast_map(d::DValue, int panic=0);
LSet* cast_set(d::DValue, int panic=0);
LNumber* cast_number(d::DValue, int panic=0);
LChar* cast_char(d::DValue, int panic=0);
LSymbol* cast_symbol(d::DValue, int panic=0);
LList* cast_list(d::DValue, int panic=0);
LVec* cast_vec(d::DValue, int panic=0);
LSequential* cast_sequential(d::DValue, int panic=0);
LSeqable* cast_seqable(d::DValue, int panic=0);
LLambda* cast_lambda(d::DValue, int panic=0);
LMacro* cast_macro(d::DValue, int panic=0);
LNative* cast_native(d::DValue, int panic=0);
LString* cast_string(d::DValue, int panic=0);
LKeyword* cast_keyword(d::DValue, int panic=0);
LFunction* cast_function(d::DValue, int panic=0);










//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

