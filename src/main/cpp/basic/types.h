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

#include <cmath>
#include "lexer.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace a= czlab::aeon;
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define NUMBER_VAL(n) czlab::dsl::DslValue(new BNumber(n))
#define STRING_VAL(s) czlab::dsl::DslValue(new BStr(s))
#define FN_VAL(n, f) czlab::dsl::DslValue(new LibFunc(n, f))
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CAST(t,x) s__cast(t,x.ptr())
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BValue : public d::Data {
  // Abstract class to store data value in parsers.
  virtual bool equals(const d::Data* rhs) const {
    return X_NIL(rhs) && eq(rhs);
  }
  virtual int compare(const d::Data* rhs) const {
    return E_NIL(rhs) ? 1 : cmp(rhs);
  }
  virtual stdstr pr_str(bool p = 0) const = 0;
  virtual ~BValue() {}

  protected:

  BValue() {}

  virtual bool eq(const d::Data*) const=0;
  virtual int cmp(const d::Data*) const=0;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef d::DslValue (*Invoker) (d::IEvaluator*, d::VSlice);

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

  LibFunc(const stdstr& name, Invoker);
  LibFunc() {fn=NULL;}

  protected:

  virtual bool eq(const d::Data* rhs) const {
    return typeid(*this) == typeid(*rhs) &&
           fn == s__cast(const LibFunc,rhs)->fn;
  }
  virtual int cmp(const d::Data* rhs) const {
    if (typeid(*this) == typeid(*rhs)) {
      auto v2= s__cast(const LibFunc,rhs)->fn;
      return fn==v2 ? 0 : pr_str(0).compare(rhs->pr_str(0));
    } else {
      return pr_str(0).compare(rhs->pr_str(0));
    }
  }

  Invoker fn;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BNumber : public BValue {
  explicit BNumber(double d) : type(d::T_REAL) { u.r=d; }
  BNumber(llong n) : type(d::T_INTEGER) { u.n=n; }
  BNumber(int n) : type(d::T_INTEGER) { u.n=n; }
  BNumber() : type(d::T_INTEGER) { u.n=0; }

  double getFloat() const { return isInt() ? (double)u.n : u.r; }
  llong getInt() const { return isInt() ? u.n : (llong) u.r; }

  void setFloat(double d) { u.r=d; }
  void setInt(llong n) { u.n=n; }

  bool isInt() const { return type== d::T_INTEGER;}
  bool isZero() const { return type==d::T_INTEGER ? u.n==0 : u.r==0.0; }
  bool isNeg() const { return type==d::T_INTEGER ? u.n < 0 : u.r < 0.0; }
  bool isPos() const { return type==d::T_INTEGER ? u.n > 0 : u.r > 0.0; }


  virtual stdstr pr_str(bool p=0) const {
    return isInt() ? std::to_string(u.n) : std::to_string(u.r);
  }

  protected:

  bool match(const BNumber* rhs) const {
    return (isInt() && rhs->isInt())
             ? getInt() == rhs->getInt()
             : a::fuzzy_equals(getFloat(), rhs->getFloat());
  }

  virtual bool eq(const d::Data* other) const {
    return typeid(*this) == typeid(*other)
           ? match(s__cast(const BNumber,other)) : false;
  }

  virtual int cmp(const d::Data* other) const {
    auto ok= typeid(*this) == typeid(*other);
    if (ok) {
      auto rhs= s__cast(const BNumber,other);
      return match(rhs) ? 0 : (getFloat() > rhs->getFloat() ? 1 : -1);
    } else {
      return pr_str(0).compare(other->pr_str(0));
    }
  }

  int type;
  union { llong n; double r; } u;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BStr : public BValue {
  virtual stdstr pr_str(bool p=0) const { return value; }
  BStr(const stdstr& s) : value(s) {}
  BStr(const char* s) : value(s) {}

  stdstr impl() const { return value; }
  protected:
  stdstr value;

  virtual bool eq(const Data* rhs) const {
    return X_NIL(rhs) &&
           typeid(*this) == typeid(*rhs) &&
           value == s__cast(const BStr,rhs)->value;
  }
  virtual int cmp(const d::Data* rhs) const {
    if (typeid(*this) == typeid(*rhs)) {
      auto v2= s__cast(const BStr,rhs)->value;
      return value.compare(v2);
    } else {
      return pr_str(0).compare(rhs->pr_str(0));
    }
  }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoopInfo;
typedef a::RefPtr<ForLoopInfo> DslForLoop;
struct ForLoopInfo : public a::Counted {
  ForLoopInfo(const stdstr& v, llong n) {
    var=v; begin=n; end=0;
  }
  llong begin, end;
  d::DslValue init;
  d::DslValue step;
  stdstr var;
  DslForLoop outer;
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

  stdstr readString();
  double readFloat();
  llong readInt();
  void writeString(const stdstr&);
  void writeFloat(double);
  void writeInt(llong);
  void writeln();

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

  llong jumpSub(llong line);
  llong retSub();
  llong jumpFor(DslForLoop);
  llong endFor(DslForLoop);
  llong jump(llong line);
  llong pc() const { return progCounter; };
  llong incr_pc() { return ++progCounter; }

  DslForLoop getCurForLoop() const { return forLoop; }
  DslForLoop getForLoop(llong c) const;

  // used during analysis
  void xrefForNext(const stdstr&, llong n);
  void addForLoop(DslForLoop f);

  Basic(const char* src);
  d::DslValue interpret();
  virtual ~Basic() {}

  private:

  std::map<llong,DslForLoop> forBegins;
  std::map<llong,DslForLoop> forEnds;

  std::stack<llong> gosubReturns;
  std::map<llong,llong> lines;
  DslForLoop forLoop;
  bool running;
  const char* source;
  llong progCounter;
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
LibFunc* cast_native(d::DslValue, int panic=0);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue op_math(BNumber* lhs, int op, BNumber* rhs);




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

