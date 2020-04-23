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
#define NUMBER_VAL(n) BNumber::make(n)
#define STRING_VAL(s) BStr::make(s)
#define CHAR_VAL(s) BChar::make(s)
#define FALSE_VAL() BNumber::make(0)
#define TRUE_VAL() BNumber::make(1)
#define FN_VAL(n, f) LibFunc::make(n, f)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BValue : public d::Data {

  virtual bool equals(d::DValue rhs) const {
    ASSERT1(rhs); return eq(rhs);
  }

  virtual int compare(d::DValue rhs) const {
    ASSERT1(rhs); return cmp(rhs);
  }

  virtual stdstr pr_str(bool p = 0) const = 0;
  virtual ~BValue() {}

  protected:

  BValue() {}

  virtual bool eq(d::DValue) const=0;
  virtual int cmp(d::DValue) const=0;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef d::DValue (*Invoker) (d::IEvaluator*, d::VSlice);
typedef std::pair<int,int> CheckPt;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Function : public BValue {

  virtual d::DValue invoke(d::IEvaluator*, d::VSlice)=0;
  virtual d::DValue invoke(d::IEvaluator*)=0;
  stdstr name() const { return _name; }

  protected:

  virtual ~Function() {}

  stdstr _name;
  Function() {}
  Function(cstdstr& n) : _name(n) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LibFunc : public Function {

  virtual d::DValue invoke(d::IEvaluator*, d::VSlice);
  virtual d::DValue invoke(d::IEvaluator*);
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LibFunc() {}

  static d::DValue make() {
    return WRAP_VAL(LibFunc);
  }

  static d::DValue make(cstdstr& name, Invoker f) {
    return WRAP_VAL(LibFunc,name,f);
  }

  //internal use only
  LibFunc() {fn=P_NIL;}

  protected:

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  LibFunc(cstdstr& name, Invoker);
  Invoker fn;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lambda : public Function {

  virtual d::DValue invoke(d::IEvaluator*, d::VSlice);
  virtual d::DValue invoke(d::IEvaluator*);
  virtual stdstr pr_str(bool p=0) const;

  static d::DValue make(cstdstr& name, StrVec& pms, d::DAst body) {
    return WRAP_VAL(Lambda, name,pms,body);
  }

  virtual ~Lambda() {}

  //internal use only
  Lambda() { }

  protected:

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  StrVec params;
  d::DAst body;
  Lambda(cstdstr&, StrVec&, d::DAst);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BArray : public BValue {

  static d::DValue make(const IntVec& v) {
    return WRAP_VAL(BArray,v);
  }

  static d::DValue make() {
    return WRAP_VAL(BArray);
  }

  d::DValue set(d::VSlice, d::DValue);
  d::DValue get(d::VSlice);

  virtual stdstr pr_str(bool p=0) const;

  virtual ~BArray();

  // internal use only
  BArray() { value=P_NIL;}

  protected:

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
  BArray(const IntVec&);
  int index(d::VSlice);

  d::ValVec* value;
  IntVec ranges;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BNumber : public BValue {

  static d::DValue make(double d) {
    return WRAP_VAL(BNumber,d);
  }

  static d::DValue make(int n) {
    return WRAP_VAL(BNumber,n);
  }

  static d::DValue make(llong n) {
    return WRAP_VAL(BNumber,n);
  }

  static d::DValue make() {
    return WRAP_VAL(BNumber);
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

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;
  bool match(const BNumber*) const;

  int type;
  union {
    llong n; double r; } num;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BChar : public BValue {

  static d::DValue make(const Tchar c) {
    return WRAP_VAL(BChar,c);
  }

  virtual stdstr pr_str(bool p=0) const {
    stdstr s;
    return s + value;
  }

  Tchar impl() const { return value; }

  // internal use only
  BChar() { value=0;}

  protected:

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

  BChar(const Tchar c) : value(c) {}

  Tchar value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BStr : public BValue {

  static d::DValue make(const Tchar* s) {
    return WRAP_VAL(BStr,s);
  }

  static d::DValue make(cstdstr& s) {
    return WRAP_VAL(BStr,s);
  }

  virtual stdstr pr_str(bool p=0) const {
    return p ? "\"" + value + "\"" : value;
  }

  stdstr impl() const { return value; }

  // internal use only
  BStr() {}

  protected:

  virtual bool eq(d::DValue) const;
  virtual int cmp(d::DValue) const;

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
  d::DValue init;
  d::DValue step;
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
  virtual d::DValue setValueEx(cstdstr&, d::DValue);
  virtual d::DValue setValue(cstdstr&, d::DValue);
  virtual d::DValue getValue(cstdstr&) const;
  virtual d::DFrame pushFrame(cstdstr&);
  virtual d::DFrame popFrame();
  virtual d::DFrame peekFrame() const;

  void writeString(cstdstr&);
  void writeFloat(double);
  void writeInt(llong);
  void writeln();
  stdstr readString();
  double readFloat();
  llong readInt();

  //analyzer
  virtual d::DSymbol search(cstdstr&) const;
  virtual d::DSymbol find(cstdstr&) const;
  virtual d::DTable pushScope(cstdstr&);
  virtual d::DTable popScope();
  virtual d::DSymbol define(d::DSymbol);

  void installProgram(const std::map<int,int>&);
  void uninstall();

  void addLambda(d::DValue);

  void addData(d::DValue);
  d::DValue readData();
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
  d::DValue interpret();
  virtual ~Basic() {}

  private:

  std::map<int,DslFLInfo> forBegins;
  std::map<int,DslFLInfo> forEnds;

  std::stack<CheckPt> gosubReturns;
  std::map<int,int> lines;

  std::map<stdstr,d::DValue> defs;

  d::ValVec dataSlots;
  int dataPtr;

  const Tchar* source;
  DslFLInfo forLoop;
  bool running;
  int progCounter;
  int progOffset;

  d::DFrame stack;
  d::DTable symbols;
  void init_lambdas();
  void check(d::DAst);
  d::DFrame root_env();
  d::DValue eval(d::DAst);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue op_math(d::DValue, int op, d::DValue);
d::DValue expected(cstdstr&, d::DValue);
void ensure_data_type(cstdstr&, d::DValue);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BNumber* cast_number(d::DValue, int panic=0);
BStr* cast_string(d::DValue, int panic=0);
BChar* cast_char(d::DValue, int panic=0);
BArray* cast_array(d::DValue, int panic=0);
Lambda* cast_lambda(d::DValue, int panic=0);
LibFunc* cast_native(d::DValue, int panic=0);



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

