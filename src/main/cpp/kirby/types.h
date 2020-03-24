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
namespace czlab::kirby {
namespace a= czlab::aeon;
namespace d= czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define HASH_VAL(k,v) std::pair<d::DslValue,d::DslValue>(k,v)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::pair<d::DslValue,d::DslValue> VPair;
typedef std::vector<d::Number> NumberVec;
typedef std::vector<d::DslValue> VVec;
typedef VVec::iterator VIter;
struct VSlice {
  VSlice(VVec& v) { begin=v.begin(); end=v.end(); }
  VSlice(VIter b, VIter e) : begin(b), end(e) {}
  int size() { return std::distance(begin, end); }
  VIter begin, end;
};
struct Lisper;
typedef d::DslValue (*Invoker)(Lisper*, VSlice);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoSuchVar : public a::Exception {
  NoSuchVar(const stdstr& m) : a::Exception (m) {}
};
struct BadArg : public a::Exception {
  BadArg(const stdstr& m) : a::Exception(m) {}
};
struct DivByZero : public a::Exception {
  DivByZero(const stdstr& m) : a::Exception(m) {}
};
struct BadArity : public a::Exception {
  BadArity(int wanted, int got)
    : a::Exception("Expected " + std::to_string(wanted) + " args, got " + std::to_string(got) + ".\n") {}
  BadArity(const stdstr& m) : a::Exception(m) {}
};
struct BadEval : public a::Exception {
  BadEval(const stdstr& m) : a::Exception(m) {}
};
struct IndexOOB : public a::Exception {
  IndexOOB(const stdstr& m) : a::Exception(m) {}
};
struct Unsupported : public a::Exception {
  Unsupported(const stdstr& m) : a::Exception(m) {}
};

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
  d::DslValue meta() const;

  // not everything can act as key in map.
  virtual bool keyable() const { return false; }
  // most things are true.
  virtual bool truthy() const { return true; }
  // not everything is seq'able.
  virtual bool seqable() const { return false; }

  // eval this s-expression, or form.
  virtual d::DslValue eval(Lisper*, d::DslFrame);
  virtual ~LValue() {}
  LValue() {}

  protected:

  virtual bool eq(const d::Data*) const = 0;
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

  protected:

  ~LSeqable() {}
  LSeqable() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFalse : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool truthy() const { return false; }

  LFalse(const LFalse&, d::DslValue);
  LFalse() {}
  virtual ~LFalse() {}

  protected:

  virtual bool eq(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LTrue : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  LTrue(const LTrue&, d::DslValue);
  LTrue() {}
  virtual ~LTrue() {}

  protected:
  virtual bool eq(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNil : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool truthy() const { return false; }

  LNil(const LNil&, d::DslValue);
  LNil() {}
  virtual ~LNil() {}

  protected:

  virtual bool eq(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LChar : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool p) const;

  LChar(const LChar&, d::DslValue);
  LChar(Tchar c) { value = c; }
  Tchar impl() { return value; }
  virtual ~LChar() {}

  protected:

  virtual bool eq(const d::Data* ) const;
  Tchar value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LAtom : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool p) const;

  LAtom(const LAtom&, d::DslValue);
  LAtom() { value= NIL_VAL();}
  LAtom(d::DslValue);
  virtual ~LAtom() {}

  d::DslValue deref() const { return value; }
  d::DslValue reset(d::DslValue x) { return (value = x); }

  protected:
  virtual bool eq(const d::Data* ) const;
  d::DslValue value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFloat : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  virtual d::Number number() const;

  LFloat(const LFloat& rhs, d::DslValue);
  LFloat(double d) : value(d) {}
  double impl() const { return value; };
  virtual ~LFloat() {}

  protected:

  virtual bool eq(const d::Data* rhs) const;
  double value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LInt : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  virtual d::Number number() const;

  llong impl() const { return value; };
  LInt(const LInt& rhs, d::DslValue);
  LInt(llong n) : value(n) {}
  virtual ~LInt() {}

  protected:
  virtual bool eq(const d::Data* rhs) const;
  llong value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LString : public LValue, public LSeqable {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;

  virtual bool keyable() const { return true; }
  virtual bool seqable() const { return true; }

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
  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LKeyword : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool keyable() const { return true; }
  stdstr impl() const { return value; }

  LKeyword(const LKeyword&, d::DslValue);
  LKeyword(const stdstr& s);
  virtual ~LKeyword() {}

  protected:
  virtual bool eq(const d::Data*) const;
  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSymbol : public LValue {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  virtual d::DslValue eval(Lisper*, d::DslFrame);
  stdstr impl() const { return value; }

  LSymbol(const LSymbol& rhs, d::DslValue);
  LSymbol(const stdstr& s);
  virtual ~LSymbol() {}

  protected:
  virtual bool eq(const d::Data*) const;
  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSequential : public LValue, public LSeqable {

  void evalEach(Lisper*, d::DslFrame, VVec&) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool seqable() const { return true; }

  virtual d::DslValue conj(VSlice) const = 0;

  virtual bool contains(d::DslValue) const;
  virtual d::DslValue nth(int) const;
  virtual d::DslValue first() const;
  virtual d::DslValue rest() const;
  virtual d::DslValue seq() const;
  virtual bool isEmpty() const;
  virtual int count() const;

  protected:

  virtual bool eq(const d::Data*) const;
  virtual ~LSequential() {}

  LSequential(const LSequential& rhs, d::DslValue);
  LSequential(VSlice chunk);
  LSequential(VVec&);
  LSequential() {}

  VVec values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LList : public LSequential {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame);

  LList(const LList& rhs, d::DslValue);
  LList(d::DslValue);
  LList(d::DslValue,d::DslValue);
  LList(d::DslValue,d::DslValue,d::DslValue);
  LList(VVec&);
  LList(VSlice);
  LList() {}

  virtual d::DslValue conj(VSlice) const;

  virtual ~LList() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LVec : public LSequential {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;

  virtual d::DslValue eval(Lisper*, d::DslFrame);

  LVec(const LVec& rhs, d::DslValue);
  LVec(d::DslValue);
  LVec(d::DslValue,d::DslValue);
  LVec(d::DslValue,d::DslValue,d::DslValue);
  LVec(VSlice);
  LVec(VVec&);
  LVec() {}
  virtual d::DslValue conj(VSlice) const;

  virtual ~LVec() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LHash : public LValue, public LSeqable {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool seqable() const { return true; }

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
  LHash(VSlice);
  LHash(VVec&);
  LHash();

  d::DslValue dissoc(VSlice) const;
  d::DslValue assoc(VSlice) const;

  d::DslValue keys() const;
  d::DslValue vals() const;

  protected:

  virtual bool eq(const d::Data*) const;
  std::map<stdstr,VPair> values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFunction : public LValue {

  virtual d::DslValue invoke(Lisper*, VSlice) = 0;
  virtual d::DslValue invoke(Lisper*) = 0;

  stdstr name() const { return _name; }

  protected:
  stdstr _name;
  LFunction(const stdstr& n) : _name(n) {}
  LFunction(d::DslValue m) : LValue(m) {}
  virtual ~LFunction() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LLambda : public LFunction {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  virtual ~LLambda() {}

  virtual d::DslValue invoke(Lisper*, VSlice);
  virtual d::DslValue invoke(Lisper*);

  LLambda(const stdstr&, const StrVec&, d::DslValue, d::DslFrame);
  LLambda(const StrVec&, d::DslValue, d::DslFrame);
  LLambda(const LLambda&, d::DslValue);
  LLambda() : LFunction("") {}

  d::DslFrame bindContext(VSlice);

  d::DslValue body;
  StrVec params;
  d::DslFrame env;

  protected:
  virtual bool eq(const d::Data* rhs) const;
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
  stdstr toString(bool pretty) const;

  protected:
  virtual bool eq(const d::Data* rhs) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNative : public LFunction {

  virtual d::DslValue withMeta(d::DslValue) const;
  virtual stdstr toString(bool pretty) const;
  virtual ~LNative() {}

  virtual d::DslValue invoke(Lisper*, VSlice);
  virtual d::DslValue invoke(Lisper*);

  LNative(const stdstr& name, Invoker);
  LNative(const LNative&, d::DslValue);
  LNative() : LFunction("") { S_NIL(fn); }

  protected:

  virtual bool eq(const d::Data*) const;
  Invoker fn;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEqual(int wanted, int got, const stdstr& fn);
int preMin(int min, int got, const stdstr& fn);
int preNonZero(int c, const stdstr& fn);
int preEven(int c, const stdstr& fn);
d::DslValue expected(const stdstr&, d::DslValue);
void appendAll(VSlice, int, VVec&);
void appendAll(VSlice, int, int, VVec&);
void appendAll(LSeqable*, VVec&);
void appendAll(LSeqable*, int, VVec&);
void appendAll(LSeqable*, int, int, VVec&);
bool truthy(d::DslValue);

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool cast_numeric(VSlice, NumberVec&);
LAtom* cast_atom(d::DslValue, int panic=0);
LNil* cast_nil(d::DslValue, int panic=0);
LHash* cast_map(d::DslValue, int panic=0);
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

