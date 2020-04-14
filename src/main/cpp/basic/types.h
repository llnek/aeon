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

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CAST(t,x) s__cast(t,x.get())

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BValue : public d::Data {

  virtual bool equals(d::DslValue rhs) const {
    ASSERT1(rhs);
    return eq(rhs);
  }

  virtual int compare(d::DslValue rhs) const {
    ASSERT1(rhs);
    return cmp(rhs);
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
typedef std::pair<llong,llong> CheckPt;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Function : public BValue {

  virtual d::DslValue invoke(d::IEvaluator*, d::VSlice)=0;
  virtual d::DslValue invoke(d::IEvaluator*)=0;
  stdstr name() const { return _name; }

  protected:

  virtual ~Function() {}

  stdstr _name;
  Function() {}
  Function(const stdstr& n) : _name(n) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LibFunc : public Function {

  virtual d::DslValue invoke(d::IEvaluator*, d::VSlice);
  virtual d::DslValue invoke(d::IEvaluator*);
  virtual stdstr pr_str(bool p=0) const;
  virtual ~LibFunc() {}

  static d::DslValue make(const stdstr& name, Invoker f) {
    return d::DslValue(new LibFunc(name,f));
  }

  static d::DslValue make() {
    return d::DslValue(new LibFunc());
  }

  LibFunc() {fn=P_NIL;}

  protected:

  LibFunc(const stdstr& name, Invoker);

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  Invoker fn;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BArray : public BValue {

  static d::DslValue make(const std::vector<llong>& v) {
    return d::DslValue(new BArray(v));
  }

  static d::DslValue make() {
    return d::DslValue(new BArray());
  }

  d::DslValue set(d::VSlice, d::DslValue);
  d::DslValue get(d::VSlice);

  virtual stdstr pr_str(bool p=0) const;

  virtual ~BArray();

  BArray() { value=P_NIL;}

  protected:

  BArray(const std::vector<llong>&);

  std::vector<llong> ranges;
  d::ValVec* value;
  llong index(d::VSlice);
  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BNumber : public BValue {

  static d::DslValue make(double d) {
    return d::DslValue(new BNumber(d));
  }

  static d::DslValue make(int n) {
    return d::DslValue(new BNumber(n));
  }

  static d::DslValue make(llong n) {
    return d::DslValue(new BNumber(n));
  }

  static d::DslValue make() {
    return d::DslValue(new BNumber());
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
    return isInt() ? std::to_string(getInt()) : std::to_string(getFloat());
  }

  BNumber() : type(d::T_INTEGER) {
    num.n=0;
  }

  protected:

  explicit BNumber(double d) : type(d::T_REAL) {
    num.r=d;
  }
  explicit BNumber(int n) : type(d::T_INTEGER) {
    num.n=n;
  }

  BNumber(llong n) : type(d::T_INTEGER) {
    num.n=n;
  }


  bool match(const BNumber* rhs) const {
    return (isInt() && rhs->isInt())
             ? getInt() == rhs->getInt()
             : a::fuzzy_equals(getFloat(), rhs->getFloat());
  }

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;

  int type;
  union {
    llong n; double r; } num;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BStr : public BValue {

  virtual stdstr pr_str(bool p=0) const { return value; }

  static d::DslValue make(const stdstr& s) {
    return d::DslValue(new BStr(s));
  }

  static d::DslValue make(const char* s) {
    return d::DslValue(new BStr(s));
  }

  stdstr impl() const { return value; }


  BStr() {}

  protected:

  BStr(const stdstr& s) : value(s) {}
  BStr(const char* s) : value(s) {}

  stdstr value;

  virtual bool eq(d::DslValue) const;
  virtual int cmp(d::DslValue) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoopInfo;
typedef std::shared_ptr<ForLoopInfo> DslFLInfo;
struct ForLoopInfo {

  static DslFLInfo make(const stdstr& v, llong n, llong p) {
    return DslFLInfo(new ForLoopInfo(v,n,p));
  }

  llong beginOffset, endOffset;
  llong begin, end;
  stdstr var;
  d::DslValue init;
  d::DslValue step;
  DslFLInfo outer;

  private:
  ForLoopInfo(const stdstr& v, llong n, llong p) {
    var=v; begin=n; end=0;
    beginOffset=p;
    endOffset=0;
  }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Basic : public d::IEvaluator, public d::IAnalyzer {

  //evaluator
  virtual d::DslValue setValueEx(const stdstr&, d::DslValue);
  virtual d::DslValue setValue(const stdstr&, d::DslValue);
  virtual d::DslValue getValue(const stdstr&) const;
  virtual d::DslFrame pushFrame(const stdstr& name);
  virtual d::DslFrame popFrame();
  virtual d::DslFrame peekFrame() const;

  void writeString(const stdstr&);
  void writeFloat(double);
  void writeInt(llong);
  void writeln();
  stdstr readString();
  double readFloat();
  llong readInt();

  //analyzer
  virtual d::DslSymbol search(const stdstr&) const;
  virtual d::DslSymbol find(const stdstr&) const;
  virtual d::DslTable pushScope(const stdstr& name);
  virtual d::DslTable popScope();
  virtual d::DslSymbol define(d::DslSymbol);

  void installProgram(const std::map<llong,llong>&);
  void uninstall();

  void init_counters();
  void finz_counters();

  void halt() { running =false; }
  bool isOn() const { return running; }

  llong jumpSub(llong target, llong from, llong pos);
  llong retSub();
  llong jumpFor(DslFLInfo);
  llong endFor(DslFLInfo);
  llong jump(llong line);

  llong poffset() { auto p= progOffset; progOffset=0; return p;}
  llong pc() const { return progCounter; };
  llong incr_pc() { return ++progCounter; }

  DslFLInfo getCurForLoop() const { return forLoop; }
  DslFLInfo getForLoop(llong c) const;

  // used during analysis
  void xrefForNext(const stdstr&, llong n, llong pos);
  void addForLoop(DslFLInfo);

  Basic(const char* src);
  d::DslValue interpret();
  virtual ~Basic() {}

  private:

  std::map<llong,DslFLInfo> forBegins;
  std::map<llong,DslFLInfo> forEnds;

  std::stack<CheckPt> gosubReturns;
  std::map<llong,llong> lines;

  DslFLInfo forLoop;
  bool running;
  const char* source;

  llong progCounter;
  llong progOffset;

  d::DslFrame stack;
  d::DslTable symbols;
  void check(d::DslAst);
  d::DslFrame root_env();
  d::DslValue eval(d::DslAst);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue expected(const stdstr&, d::DslValue);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BNumber* cast_number(d::DslValue, int panic=0);
BStr* cast_string(d::DslValue, int panic=0);
BArray* cast_array(d::DslValue, int panic=0);
LibFunc* cast_native(d::DslValue, int panic=0);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue op_math(d::DslValue, int op, d::DslValue);




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

