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
#define FLOAT_VAL(n) czlab::dsl::DslValue(new LFloat(n))
#define INT_VAL(n) czlab::dsl::DslValue(new LInt(n))
#define EMPTY_LIST() czlab::dsl::DslValue(new LList())
#define CHAR_VAL(c) czlab::dsl::DslValue(new LChar(c))
#define ATOM_VAL(a) czlab::dsl::DslValue(new LAtom(a))
#define FALSE_VAL() czlab::dsl::DslValue(new LFalse())
#define TRUE_VAL() czlab::dsl::DslValue(new LTrue())
#define NIL_VAL() czlab::dsl::DslValue(new LNil())
#define VEC_VAL(v) czlab::dsl::DslValue(new LVec(v))
#define VEC_VAL2(p1,p2) czlab::dsl::DslValue(new LVec(p1,p2))

#define LIST_VAL(v) czlab::dsl::DslValue(new LList(v))
#define LIST_VAL2(p1,p2) czlab::dsl::DslValue(new LList(p1,p2))
#define LIST_VAL3(p1,p2,p3) czlab::dsl::DslValue(new LList(p1,p2,p3))
#define MAP_VAL(v) czlab::dsl::DslValue(new LHash(v))
#define SET_VAL(v) czlab::dsl::DslValue(new LSet(v))
#define KEYWORD_VAL(s) czlab::dsl::DslValue(new LKeyword(s))
#define STRING_VAL(s) czlab::dsl::DslValue(new LString(s))
#define SYMBOL_VAL(s) czlab::dsl::DslValue(new LSymbol(s))
#define BOOL_VAL(b) ((b) ? TRUE_VAL() : FALSE_VAL())
#define FN_VAL(n, f) czlab::dsl::DslValue(new LNative(n, f))
#define MACRO_VAL(n,p,b,e) czlab::dsl::DslValue(new LMacro(n,p,b,e))
#define LAMBDA_VAL(n,p,b,e) czlab::dsl::DslValue(new LLambda(n,p,b,e))

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

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual bool equals(const d::Data*) const;
  virtual int compare(const d::Data*) const;
  d::DslValue meta() const;

  // most things are true.
  virtual bool truthy() const { return true; }

  // eval this s-expression, or form.
  virtual d::DslValue eval(Lisper*, d::DslFrame);
  virtual ~LValue() {}
  LValue() {}

  protected:

  virtual bool eq(const d::Data*) const = 0;
  virtual int cmp(const d::Data*) const = 0;
  LValue(d::DslValue m) : metaObj(m) {}
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

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual bool truthy() const { return false; }

  LFalse(const LFalse&, d::DslValue);
  LFalse() {}
  virtual ~LFalse() {}

  protected:

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LTrue : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  LTrue(const LTrue&, d::DslValue);
  LTrue() {}
  virtual ~LTrue() {}

  protected:
  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNil : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual bool truthy() const { return false; }

  LNil(const LNil&, d::DslValue);
  LNil() {}
  virtual ~LNil() {}

  protected:

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LChar : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  LChar(const LChar&, d::DslValue);
  LChar(Tchar c) { value = c; }
  Tchar impl() { return value; }
  virtual ~LChar() {}

  protected:

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
  Tchar value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LAtom : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  LAtom(const LAtom&, d::DslValue);
  LAtom() { value= NIL_VAL();}
  LAtom(d::DslValue);
  virtual ~LAtom() {}

  d::DslValue deref() const { return value; }
  d::DslValue reset(d::DslValue x) { return (value = x); }

  protected:
  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
  d::DslValue value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFloat : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual d::Number number() const;

  LFloat(const LFloat& rhs, d::DslValue);
  LFloat(double d) : value(d) {}
  double impl() const { return value; };
  virtual ~LFloat() {}

  protected:

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
  double value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LInt : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual d::Number number() const;

  llong impl() const { return value; };
  LInt(const LInt& rhs, d::DslValue);
  LInt(llong n) : value(n) {}
  virtual ~LInt() {}

  protected:
  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
  llong value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LString : public LValue, public LSeqable {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  LString(const LString&, d::DslValue);
  LString(const stdstr&);
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

  protected:
  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LKeyword : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  stdstr impl() const { return value; }

  LKeyword(const LKeyword&, d::DslValue);
  LKeyword(const stdstr& s);
  virtual ~LKeyword() {}

  protected:
  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSymbol : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual d::DslValue eval(Lisper*, d::DslFrame);
  stdstr impl() const { return value; }

  void rename(const stdstr&);

  LSymbol(const LSymbol& rhs, d::DslValue);
  LSymbol(const stdstr& s);
  virtual ~LSymbol() {}

  protected:
  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
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

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
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

  LList(const LList& rhs, d::DslValue);
  LList(d::DslValue);
  LList(d::DslValue,d::DslValue);
  LList(d::DslValue,d::DslValue,d::DslValue);
  LList(d::ValVec&);
  LList(d::VSlice);
  LList() {}

  virtual d::DslValue conj(d::VSlice) const;
  virtual ~LList() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LVec : public LSequential {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame);

  LVec(const LVec& rhs, d::DslValue);
  LVec(d::DslValue);
  LVec(d::DslValue,d::DslValue);
  LVec(d::DslValue,d::DslValue,d::DslValue);
  LVec(d::VSlice);
  LVec(d::ValVec&);
  LVec() {}
  virtual d::DslValue conj(d::VSlice) const;
  virtual ~LVec() {}
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

  LSet(const std::set<d::DslValue,SetCompare>&);
  LSet(const LSet& rhs, d::DslValue);
  LSet(d::DslValue);
  LSet(d::VSlice);
  LSet(d::ValVec&);
  LSet();

  d::DslValue conj(d::VSlice) const;
  d::DslValue disj(d::VSlice) const;

  virtual ~LSet();

  protected:

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
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

  LHash(const std::map<stdstr, VPair>&);
  LHash(const LHash& rhs, d::DslValue);
  LHash(d::VSlice);
  LHash(d::ValVec&);
  LHash();

  d::DslValue dissoc(d::VSlice) const;
  d::DslValue assoc(d::VSlice) const;

  d::DslValue keys() const;
  d::DslValue vals() const;

  virtual ~LHash() {}

  protected:

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
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

  LLambda(const stdstr&, const StrVec&, d::DslValue, d::DslFrame);
  LLambda(const StrVec&, d::DslValue, d::DslFrame);
  LLambda(const LLambda&, d::DslValue);
  LLambda() : LFunction("") {}

  d::DslFrame bindContext(d::VSlice);

  d::DslValue body;
  StrVec params;
  d::DslFrame env;

  protected:

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LMacro : public LLambda {

  LMacro(const stdstr&, const StrVec&, d::DslValue, d::DslFrame);
  LMacro(const StrVec&, d::DslValue, d::DslFrame);
  LMacro(const LMacro&, d::DslValue);
  LMacro() {}
  virtual ~LMacro() {}

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual bool isMacro() const { return true; }
  virtual stdstr pr_str(bool p=0) const;

  protected:

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNative : public LFunction {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LNative() {}

  virtual d::DslValue invoke(Lisper*, d::VSlice);
  virtual d::DslValue invoke(Lisper*);

  LNative(const stdstr& name, Invoker);
  LNative(const LNative&, d::DslValue);
  LNative() : LFunction("") { S_NIL(fn); }

  protected:

  Invoker fn;

  virtual bool eq(const d::Data*) const;
  virtual int cmp(const d::Data*) const;
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
bool cast_numeric(d::VSlice, d::NumberVec&);
LAtom* cast_atom(d::DslValue, int panic=0);
LNil* cast_nil(d::DslValue, int panic=0);
LHash* cast_map(d::DslValue, int panic=0);
LSet* cast_set(d::DslValue, int panic=0);
LFloat* cast_float(d::DslValue, int panic=0);
LInt* cast_int(d::DslValue, int panic=0);
LChar* cast_char(d::DslValue, int panic=0);
LSymbol* cast_symbol(d::DslValue, int panic=0);
LList* cast_list(d::DslValue, int panic=0);
LVec* cast_vec(d::DslValue, int panic=0);
LSequential* cast_seq(d::DslValue, int panic=0);
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

