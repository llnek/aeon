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

#include "lexer.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace a= czlab::aeon;
namespace d= czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define FN_VAL(n, f) LibFunc::make(n, f)
#define NUMBER_VAL(n) BNumber::make(n)
#define STRING_VAL(s) BStr::make(s)
#define FALSE_VAL() BNumber::make(0)
#define TRUE_VAL() BNumber::make(1)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BValue : public d::Data {

  virtual bool equals(d::DslValue rhs) const {
    ASSERT1(rhs); return eq(rhs);
  }

  virtual int compare(d::DslValue rhs) const {
    ASSERT1(rhs); return cmp(rhs);
  }

  virtual stdstr pr_str(bool p = 0) const = 0;
  virtual ~BValue() {}

  protected:

  BValue() {}

  virtual bool eq(d::DslValue) const=0;
  virtual int cmp(d::DslValue) const=0;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef d::DslValue (*Invoker) (d::IEvaluator*, d::VSlice);
typedef std::pair<int,int> CheckPt;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Function : public BValue {

  virtual d::DslValue invoke(d::IEvaluator*, d::VSlice)=0;
  virtual d::DslValue invoke(d::IEvaluator*)=0;
  stdstr name() const { return _name; }

  protected:

  virtual ~Function() {}

  stdstr _name;
  Function() {}
  Function(cstdstr& n) : _name(n) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LibFunc : public Function {

  virtual d::DslValue invoke(d::IEvaluator*, d::VSlice);
  virtual d::DslValue invoke(d::IEvaluator*);
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LibFunc() {}

  static d::DslValue make() {
    return WRAP_VAL(new LibFunc());
  }

  static d::DslValue make(cstdstr& name, Invoker f) {
    return WRAP_VAL(new LibFunc(name,f));
  }

  //internal use only
  LibFunc() {fn=P_NIL;}

  protected:

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  LibFunc(cstdstr& name, Invoker);
  Invoker fn;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lambda : public Function {

  virtual d::DslValue invoke(d::IEvaluator*, d::VSlice);
  virtual d::DslValue invoke(d::IEvaluator*);
  virtual stdstr pr_str(bool p=0) const;

  static d::DslValue make(cstdstr& name, StrVec& pms, d::DslAst body) {
    return WRAP_VAL(new Lambda(name,pms,body));
  }

  virtual ~Lambda() {}

  //internal use only
  Lambda() { }

  protected:

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  StrVec params;
  d::DslAst body;
  Lambda(cstdstr&, StrVec&, d::DslAst);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BArray : public BValue {

  static d::DslValue make(const IntVec& v) {
    return WRAP_VAL(new BArray(v));
  }

  static d::DslValue make() {
    return WRAP_VAL(new BArray());
  }

  d::DslValue set(d::VSlice, d::DslValue);
  d::DslValue get(d::VSlice);

  virtual stdstr pr_str(bool p=0) const;

  virtual ~BArray();

  // internal use only
  BArray() { value=P_NIL;}

  protected:

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
  BArray(const IntVec&);
  int index(d::VSlice);

  d::ValVec* value;
  IntVec ranges;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BNumber : public BValue {

  static d::DslValue make(double d) {
    return WRAP_VAL(new BNumber(d));
  }

  static d::DslValue make(int n) {
    return WRAP_VAL(new BNumber(n));
  }

  static d::DslValue make(llong n) {
    return WRAP_VAL(new BNumber(n));
  }

  static d::DslValue make() {
    return WRAP_VAL(new BNumber());
  }

  double getFloat() const { return isInt() ? (double)num.n : num.r; }
  llong getInt() const { return isInt() ? num.n : (llong) num.r; }

  void setFloat(double d) { num.r=d; }
  void setInt(llong n) { num.n=n; }

  bool isInt() const { return type== d::T_INTEGER;}

  bool isZero() const {
    return type==d::T_INTEGER ? getInt()==0 : a::fuzzy_zero(getFloat()); }

  bool isNeg() const {
    return type==d::T_INTEGER ? getInt() < 0 : getFloat() < 0.0; }

  bool isPos() const {
    return type==d::T_INTEGER ? getInt() > 0 : getFloat() > 0.0; }

  virtual stdstr pr_str(bool p=0) const {
    return isInt() ? N_STR(getInt()) : N_STR(getFloat());
  }

  BNumber() : type(d::T_INTEGER) { num.n=0; }

  protected:

  explicit BNumber(double d) : type(d::T_REAL) { num.r=d; }
  explicit BNumber(int n) : type(d::T_INTEGER) { num.n=n; }
  BNumber(llong n) : type(d::T_INTEGER) { num.n=n; }

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
  bool match(const BNumber*) const;

  int type;
  union {
    llong n; double r; } num;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BStr : public BValue {

  static d::DslValue make(const Tchar* s) {
    return WRAP_VAL(new BStr(s));
  }

  static d::DslValue make(cstdstr& s) {
    return WRAP_VAL(new BStr(s));
  }

  virtual stdstr pr_str(bool p=0) const {
    return p ? "\"" + value + "\"" : value;
  }

  stdstr impl() const { return value; }

  // internal use only
  BStr() {}

  protected:

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  BStr(cstdstr& s) : value(s) {}
  BStr(const Tchar* s) : value(s) {}

  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoopInfo;
typedef std::shared_ptr<ForLoopInfo> DslFLInfo;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoopInfo {

  static DslFLInfo make(cstdstr& v, int n, int p) {
    return DslFLInfo(new ForLoopInfo(v,n,p));
  }

  int beginOffset, endOffset;
  int begin, end;
  stdstr var;
  d::DslValue init;
  d::DslValue step;
  DslFLInfo outer;

  private:

  ForLoopInfo(cstdstr& v, int n, int p) {
    var=v; begin=n; end=0;
    beginOffset=p;
    endOffset=0;
  }

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Basic : public d::IEvaluator, public d::IAnalyzer {

  //evaluator
  virtual d::DslValue setValueEx(cstdstr&, d::DslValue);
  virtual d::DslValue setValue(cstdstr&, d::DslValue);
  virtual d::DslValue getValue(cstdstr&) const;
  virtual d::DslFrame pushFrame(cstdstr&);
  virtual d::DslFrame popFrame();
  virtual d::DslFrame peekFrame() const;

  void writeString(cstdstr&);
  void writeFloat(double);
  void writeInt(llong);
  void writeln();
  stdstr readString();
  double readFloat();
  llong readInt();

  //analyzer
  virtual d::DslSymbol search(cstdstr&) const;
  virtual d::DslSymbol find(cstdstr&) const;
  virtual d::DslTable pushScope(cstdstr&);
  virtual d::DslTable popScope();
  virtual d::DslSymbol define(d::DslSymbol);

  void installProgram(const std::map<int,int>&);
  void uninstall();

  void addLambda(d::DslValue);

  void addData(d::DslValue);
  d::DslValue readData();
  void restore();

  void init_counters();
  void finz_counters();

  void halt() { running =false; }
  bool isOn() const { return running; }

  int jumpSub(int target, int from, int pos);
  int retSub();
  int jumpFor(DslFLInfo);
  int endFor(DslFLInfo);
  int jump(int line);

  int poffset() { auto p= progOffset; progOffset=0; return p;}
  int pc() const { return progCounter; };
  int incr_pc() { return ++progCounter; }

  DslFLInfo getCurForLoop() const { return forLoop; }
  DslFLInfo getForLoop(int c) const;

  // used during analysis
  void xrefForNext(cstdstr&, int n, int pos);
  void xrefForNext(int n, int pos);
  void addForLoop(DslFLInfo);

  Basic(const Tchar* src);
  d::DslValue interpret();
  virtual ~Basic() {}

  private:

  std::map<int,DslFLInfo> forBegins;
  std::map<int,DslFLInfo> forEnds;

  std::stack<CheckPt> gosubReturns;
  std::map<int,int> lines;

  std::map<stdstr,d::DslValue> defs;

  d::ValVec dataSlots;
  int dataPtr;

  const Tchar* source;
  DslFLInfo forLoop;
  bool running;
  int progCounter;
  int progOffset;

  d::DslFrame stack;
  d::DslTable symbols;
  void init_lambdas();
  void check(d::DslAst);
  d::DslFrame root_env();
  d::DslValue eval(d::DslAst);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue op_math(d::DslValue, int op, d::DslValue);
d::DslValue expected(cstdstr&, d::DslValue);
void ensure_data_type(cstdstr&, d::DslValue);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BNumber* cast_number(d::DslValue, int panic=0);
BStr* cast_string(d::DslValue, int panic=0);
BArray* cast_array(d::DslValue, int panic=0);
Lambda* cast_lambda(d::DslValue, int panic=0);
LibFunc* cast_native(d::DslValue, int panic=0);



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

