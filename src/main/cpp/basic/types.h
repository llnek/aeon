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
#define FLOAT_VAL(n) czlab::dsl::DslValue(new BReal(n))
#define INT_VAL(n) czlab::dsl::DslValue(new BInt(n))
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
struct BInt : public BValue {
  virtual stdstr pr_str(bool p) const { return std::to_string(value); }
  BInt(llong n) { value=n; }
  llong impl() const { return value; }
  protected:
  llong value;
  virtual bool eq(const d::Data* rhs) const {
    return typeid(*this) == typeid(*rhs) &&
           value == s__cast(const BInt,rhs)->value;
  }
  virtual int cmp(const d::Data* rhs) const {
    if (typeid(*this) == typeid(*rhs)) {
      auto v2= s__cast(const BInt,rhs)->value;
      return value==v2 ? 0 : (value > v2 ? 1 : -1);
    } else {
      return pr_str(0).compare(rhs->pr_str(0));
    }
  }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BReal : public BValue {
  virtual stdstr pr_str(bool p=0) const { return std::to_string(value); }
  BReal(double d)  { value=d; }

  double impl() const { return value; }

  protected:
  double value;
  virtual bool eq(const Data* rhs) const {
    return X_NIL(rhs) &&
           typeid(*this) == typeid(*rhs) &&
           a::fuzzy_equals(value, s__cast(const BReal,rhs)->value);
  }
  virtual int cmp(const d::Data* rhs) const {
    if (typeid(*this) == typeid(*rhs)) {
      auto v2= s__cast(const BReal,rhs)->value;
      return value==v2 ? 0 : (value > v2 ? 1 : -1);
    } else {
      return pr_str(0).compare(rhs->pr_str(0));
    }
  }
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

  llong move_pc(llong pos) { return (progCounter += pos);}
  llong pc() const { return progCounter; };
  llong incr_pc() { return ++progCounter; }
  void init_counters() { progCounter= -1; }
  void finz_counters() { progCounter = -1; }

  Basic(const char* src);
  d::DslValue interpret();
  virtual ~Basic() {}

  private:
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
bool cast_numeric(d::VSlice, d::NumberVec&);
BReal* cast_float(d::DslValue, int panic=0);
BInt* cast_int(d::DslValue, int panic=0);
BStr* cast_string(d::DslValue, int panic=0);
LibFunc* cast_native(d::DslValue, int panic=0);
BReal* op_num(double);
BInt* op_num(llong);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
d::DslValue op_math(d::Number lhs, int op, d::Number rhs) {
  T x = lhs.isInt() ? lhs.getInt() : lhs.getFloat();
  T y = rhs.isInt() ? rhs.getInt() : rhs.getFloat();
  T res;
  switch (op) {
    case T_INT_DIV:
      ASSERT1(lhs.isInt() && rhs.isInt());
      ASSERT1(rhs.getInt() != 0);
      res = (lhs.getInt() / rhs.getInt());
    break;
    case d::T_PLUS:
      res = x + y;
    break;
    case d::T_MINUS:
      res = x - y;
    break;
    case d::T_MULT:
      res = x * y;
    break;
    case d::T_DIV:
      if (rhs.isInt()) { ASSERT1(rhs.getInt() != 0); }
      else { ASSERT1(!a::fuzzy_zero(rhs.getFloat())); }
      res = x / y;
    break;
    case T_MOD:
      if (lhs.isInt() && rhs.isInt()) {
        res = (lhs.getInt() % rhs.getInt());
      } else {
        res = ::fmod(x,y);
      }
    break;
    case T_POWER:
      res = ::pow(x,y);
    break;
  }
  return op_num(res);
}





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

