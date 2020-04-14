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
#define HASH_VAL(k,v) std::pair<d::DslValue,d::DslValue>(k,v)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef bool (*SetCompare) (d::DslValue, d::DslValue);
typedef std::pair<d::DslValue, d::DslValue> VPair;
struct Lisper;
typedef d::DslValue (*Invoker) (Lisper*, d::VSlice);

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
  d::DslValue macroExpand(d::DslValue ast, d::DslFrame env);
  d::DslValue syntaxQuote(d::DslValue ast, d::DslFrame env);
  d::DslValue evalAst(d::DslValue ast, d::DslFrame env);
  d::DslValue EVAL(d::DslValue ast, d::DslFrame env);
  std::pair<int,d::DslValue> READ(const stdstr&);
  stdstr PRINT(d::DslValue);
  Lisper() { seed=0; }
  ~Lisper() {}

  private:

  int seed;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LValue : public d::Data {

  virtual d::DslValue eval(Lisper*, d::DslFrame) = 0;
  virtual d::DslValue withMeta(d::DslValue) const;
  virtual bool equals(d::DslValue) const;
  virtual int compare(d::DslValue) const;

  virtual bool truthy() const { return true; }
  d::DslValue meta() const { return metaObj; }
  virtual ~LValue() {}

  protected:

  virtual bool eq(d::DslValue) const = 0;
  virtual int cmp(d::DslValue) const = 0;

  LValue(d::DslValue m) : metaObj(m) {}
  LValue() {}

  d::DslValue metaObj;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSeqable {

  virtual bool contains(d::DslValue) const = 0;
  virtual d::DslValue first() const = 0;
  virtual d::DslValue rest() const = 0;
  virtual d::DslValue seq() const = 0;
  virtual d::DslValue nth(int) const = 0;
  virtual bool isEmpty() const = 0;
  virtual int count() const = 0 ;
  virtual ~LSeqable() {}

  protected:

  LSeqable() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFalse : public LValue {

  static d::DslValue make() { return d::DslValue(new LFalse()); }

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual bool truthy() const { return false; }

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LFalse());
  }

  virtual ~LFalse() {}
  LFalse() {}

  protected:

  LFalse(const LFalse&, d::DslValue);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LTrue : public LValue {

  static d::DslValue make() { return d::DslValue(new LTrue()); }

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LTrue());
  }

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LTrue() {}
  LTrue() {}

  protected:

  LTrue(const LTrue&, d::DslValue);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNil : public LValue {

  static d::DslValue make() { return d::DslValue(new LNil()); }

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LNil());
  }

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual bool truthy() const { return false; }

  virtual ~LNil() {}
  LNil() {}

  protected:

  LNil(const LNil&, d::DslValue);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LChar : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LChar(value));
  }

  static d::DslValue make(Tchar c) {
    return d::DslValue(new LChar(c));
  }

  Tchar impl() { return value; }
  virtual ~LChar() {}

  LChar() { value=0;}

  protected:

  LChar(const LChar&, d::DslValue);
  LChar(Tchar c) { value = c; }

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  Tchar value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LAtom : public LValue {

  d::DslValue reset(d::DslValue x) { return (value = x); }
  d::DslValue deref() const { return value; }

  static d::DslValue make(d::DslValue v) {
    return d::DslValue(new LAtom(v));
  }

  static d::DslValue make() {
    return d::DslValue(new LAtom());
  }

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LAtom(value));
  }

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  LAtom() { value= NIL_VAL();}
  virtual ~LAtom() {}

  protected:

  LAtom(const LAtom&, d::DslValue);
  LAtom(d::DslValue);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  d::DslValue value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNumber : public LValue {

  double getFloat() const { return isInt() ? (double)num.n : num.r; }
  llong getInt() const { return isInt() ? num.n : (llong)num.r; }
  bool isInt() const { return _type == d::T_INTEGER; }

  static d::DslValue make(double d) {
    return d::DslValue(new LNumber(d));
  }

  static d::DslValue make(llong n) {
    return d::DslValue(new LNumber(n));
  }

  static d::DslValue make(int n) {
    return d::DslValue(new LNumber(n));
  }

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual d::DslValue eval(Lisper*, d::DslFrame);
  virtual stdstr pr_str(bool p=0) const;

  virtual ~LNumber() {}
  explicit LNumber(int);

  protected:

  LNumber(const LNumber&, d::DslValue);
  explicit LNumber(double);
  LNumber(llong);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  int _type;
  union {
    double r; llong n; } num;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LString : public LValue, public LSeqable {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  static d::DslValue make(const stdstr& s) {
    return d::DslValue(new LString(s));
  }

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LString(value));
  }

  virtual ~LString() {}

  stdstr encoded() const;
  stdstr impl() const { return value; }

  virtual bool contains(d::DslValue) const;
  virtual d::DslValue nth(int) const;
  virtual d::DslValue first() const;
  virtual d::DslValue rest() const;
  virtual d::DslValue seq() const;
  virtual bool isEmpty() const;
  virtual int count() const;
  LString() {}

  protected:

  LString(const LString&, d::DslValue);
  LString(const stdstr&);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LKeyword : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  static d::DslValue make(const stdstr& s) {
    return d::DslValue(new LKeyword(s));
  }

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LKeyword(value));
  }

  stdstr impl() const { return value; }

  virtual ~LKeyword() {}
  LKeyword() {};

  protected:

  LKeyword(const LKeyword&, d::DslValue);
  LKeyword(const stdstr&);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSymbol : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  static d::DslValue make(const stdstr& s) {
    return d::DslValue(new LSymbol(s));
  }

  virtual d::DslValue eval(Lisper*, d::DslFrame);

  stdstr impl() const { return value; }
  void rename(const stdstr&);

  virtual ~LSymbol() {}
  LSymbol() {}

  protected:

  LSymbol(const LSymbol& rhs, d::DslValue);
  LSymbol(const stdstr&);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSequential : public LValue, public LSeqable {

  void evalEach(Lisper*, d::DslFrame, d::ValVec&) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DslValue conj(d::VSlice) const = 0;

  virtual bool contains(d::DslValue) const;
  virtual d::DslValue nth(int) const;
  virtual d::DslValue first() const;
  virtual d::DslValue rest() const;
  virtual d::DslValue seq() const;
  virtual bool isEmpty() const;
  virtual int count() const;

  protected:

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
  virtual ~LSequential() {}

  LSequential(const LSequential& rhs, d::DslValue);
  LSequential(d::VSlice chunk);
  LSequential(d::ValVec&);
  LSequential() {}

  d::ValVec values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LList : public LSequential {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame);

  virtual d::DslValue conj(d::VSlice) const;
  virtual ~LList() {}

  static d::DslValue make(d::DslValue v) {
    return d::DslValue(new LList(v));
  }

  static d::DslValue make(d::DslValue v1, d::DslValue v2) {
    return d::DslValue(new LList(v1, v2));
  }

  static d::DslValue make(d::DslValue v1, d::DslValue v2, d::DslValue v3) {
    return d::DslValue(new LList(v1, v2, v3));
  }

  static d::DslValue make(d::ValVec& v) {
    return d::DslValue(new LList(v));
  }

  static d::DslValue make(d::VSlice s) {
    return d::DslValue(new LList(s));
  }

  static d::DslValue make() {
    return d::DslValue(new LList());
  }

  LList() {}

  protected:

  LList(d::DslValue,d::DslValue,d::DslValue);
  LList(const LList& rhs, d::DslValue);
  LList(d::DslValue);
  LList(d::DslValue,d::DslValue);
  LList(d::ValVec&);
  LList(d::VSlice);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LVec : public LSequential {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame);
  virtual d::DslValue conj(d::VSlice) const;

  static d::DslValue make(d::DslValue v1, d::DslValue v2, d::DslValue v3) {
    return d::DslValue(new LVec(v1, v2, v3));
  }

  static d::DslValue make(d::DslValue v1, d::DslValue v2) {
    return d::DslValue(new LVec(v1, v2));
  }

  static d::DslValue make(d::DslValue v1) {
    return d::DslValue(new LVec(v1));
  }

  static d::DslValue make(d::ValVec& v) {
    return d::DslValue(new LVec(v));
  }

  static d::DslValue make(d::VSlice s) {
    return d::DslValue(new LVec(s));
  }

  static d::DslValue make() {
    return d::DslValue(new LVec());
  }

  virtual ~LVec() {}
  LVec() {}

  protected:

  LVec(d::DslValue,d::DslValue,d::DslValue);
  LVec(const LVec& rhs, d::DslValue);
  LVec(d::DslValue);
  LVec(d::DslValue,d::DslValue);
  LVec(d::VSlice);
  LVec(d::ValVec&);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSet : public LValue, public LSeqable {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame);
  d::DslValue get(d::DslValue) const;

  virtual bool contains(d::DslValue) const;
  virtual d::DslValue first() const;
  virtual d::DslValue rest() const;
  virtual d::DslValue seq() const;
  virtual d::DslValue nth(int) const;
  virtual bool isEmpty() const;
  virtual int count() const;

  static d::DslValue make(const std::set<d::DslValue,SetCompare>& s) {
    return d::DslValue(new LSet(s));
  }

  static d::DslValue make(d::DslValue v) {
    return d::DslValue(new LSet(v));
  }

  static d::DslValue make(d::VSlice s) {
    return d::DslValue(new LSet(s));
  }

  static d::DslValue make(d::ValVec& v) {
    return d::DslValue(new LSet(v));
  }

  static d::DslValue make() {
    return d::DslValue(new LSet());
  }

  d::DslValue conj(d::VSlice) const;
  d::DslValue disj(d::VSlice) const;

  virtual ~LSet();
  LSet();

  protected:

  LSet(const std::set<d::DslValue,SetCompare>&);
  LSet(const LSet& rhs, d::DslValue);
  LSet(d::DslValue);
  LSet(d::VSlice);
  LSet(d::ValVec&);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
  std::set<d::DslValue,SetCompare>* values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LHash : public LValue, public LSeqable {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame);
  d::DslValue get(d::DslValue) const;

  virtual bool contains(d::DslValue) const;
  virtual d::DslValue first() const;
  virtual d::DslValue rest() const;
  virtual d::DslValue seq() const;
  virtual d::DslValue nth(int) const;
  virtual bool isEmpty() const;
  virtual int count() const;

  static d::DslValue make(const std::map<stdstr, VPair>& s) {
    return d::DslValue(new LHash(s));
  }

  static d::DslValue make(d::VSlice s) {
    return d::DslValue(new LHash(s));
  }

  static d::DslValue make(d::ValVec& v) {
    return d::DslValue(new LHash(v));
  }

  static d::DslValue make() {
    return d::DslValue(new LHash());
  }

  d::DslValue dissoc(d::VSlice) const;
  d::DslValue assoc(d::VSlice) const;

  d::DslValue keys() const;
  d::DslValue vals() const;

  virtual ~LHash() {}
  LHash();

  protected:

  LHash(const std::map<stdstr, VPair>&);
  LHash(const LHash& rhs, d::DslValue);
  LHash(d::VSlice);
  LHash(d::ValVec&);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
  std::map<stdstr,VPair> values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFunction : public LValue {

  virtual d::DslValue invoke(Lisper*, d::VSlice) = 0;
  virtual d::DslValue invoke(Lisper*) = 0;

  stdstr name() const { return _name; }

  protected:

  virtual ~LFunction() {}

  stdstr _name;
  LFunction(const stdstr& n) : _name(n) {}
  LFunction(d::DslValue m) : LValue(m) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LLambda : public LFunction {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LLambda() {}

  virtual d::DslValue invoke(Lisper*, d::VSlice);
  virtual d::DslValue invoke(Lisper*);

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LLambda(*this, d::DslValue(P_NIL)));
  }

  static d::DslValue make(const stdstr& n, const StrVec& s, d::DslValue v, d::DslFrame f) {
    return d::DslValue(new LLambda(n, s, v, f));
  }

  static d::DslValue make(const StrVec& s, d::DslValue v, d::DslFrame f) {
    return d::DslValue(new LLambda(s, v, f));
  }

  static d::DslValue make() {
    return d::DslValue(new LLambda());
  }

  d::DslFrame bindContext(d::VSlice);
  LLambda() : LFunction("") {}

  d::DslValue body;
  StrVec params;
  d::DslFrame env;

  protected:

  LLambda(const stdstr&, const StrVec&, d::DslValue, d::DslFrame);
  LLambda(const StrVec&, d::DslValue, d::DslFrame);
  LLambda(const LLambda&, d::DslValue);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LMacro : public LLambda {

  static d::DslValue make(const stdstr& n, const StrVec& s, d::DslValue v, d::DslFrame f) {
    return d::DslValue(new LMacro(n, s, v, f));
  }

  static d::DslValue make(const StrVec& s, d::DslValue v, d::DslFrame f) {
    return d::DslValue(new LMacro(s, v, f));
  }

  static d::DslValue make() {
    return d::DslValue(new LMacro());
  }

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LMacro(*this, d::DslValue(P_NIL)));
  }

  virtual ~LMacro() {}

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual bool isMacro() const { return true; }
  virtual stdstr pr_str(bool p=0) const;
  LMacro() {}

  protected:

  LMacro(const stdstr&, const StrVec&, d::DslValue, d::DslFrame);
  LMacro(const StrVec&, d::DslValue, d::DslFrame);
  LMacro(const LMacro&, d::DslValue);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNative : public LFunction {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LNative() {}

  virtual d::DslValue invoke(Lisper*, d::VSlice);
  virtual d::DslValue invoke(Lisper*);

  static d::DslValue make(const stdstr& name, Invoker f) {
    return d::DslValue(new LNative(name, f));
  }

  static d::DslValue make() {
    return d::DslValue(new LNative());
  }

  virtual d::DslValue eval(Lisper*, d::DslFrame) {
    return d::DslValue(new LNative(*this, d::DslValue(P_NIL)));
  }

  LNative() : LFunction("") { S_NIL(fn); }

  protected:

  LNative(const stdstr& name, Invoker);
  LNative(const LNative&, d::DslValue);

  Invoker fn;

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue expected(const stdstr&, d::DslValue);
void appendAll(d::VSlice, int, d::ValVec&);
void appendAll(d::VSlice, int, int, d::ValVec&);
void appendAll(LSeqable*, d::ValVec&);
void appendAll(LSeqable*, int, d::ValVec&);
void appendAll(LSeqable*, int, int, d::ValVec&);
bool truthy(d::DslValue);

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool scan_numbers(d::VSlice);
LAtom* cast_atom(d::DslValue, int panic=0);
LNil* cast_nil(d::DslValue, int panic=0);
LHash* cast_map(d::DslValue, int panic=0);
LSet* cast_set(d::DslValue, int panic=0);
LNumber* cast_number(d::DslValue, int panic=0);
LChar* cast_char(d::DslValue, int panic=0);
LSymbol* cast_symbol(d::DslValue, int panic=0);
LList* cast_list(d::DslValue, int panic=0);
LVec* cast_vec(d::DslValue, int panic=0);
LSequential* cast_sequential(d::DslValue, int panic=0);
LSeqable* cast_seqable(d::DslValue, int panic=0);
LLambda* cast_lambda(d::DslValue, int panic=0);
LMacro* cast_macro(d::DslValue, int panic=0);
LNative* cast_native(d::DslValue, int panic=0);
LString* cast_string(d::DslValue, int panic=0);
LKeyword* cast_keyword(d::DslValue, int panic=0);
LFunction* cast_function(d::DslValue, int panic=0);










//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

