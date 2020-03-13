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
typedef std::vector<d::DslValue> ValueVec;
typedef d::DslValue (Func)(ValueVec&);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue false_value();
d::DslValue nil_value();
d::DslValue true_value();
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoSuchVarError : public a::Exception {
  NoSuchVarError(const stdstr& m) : a::Exception (m) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LValue : public d::Data {

  virtual d::DslValue withMeta(d::DslValue& ) const;
  virtual bool equals(const d::Data*) const;
  d::DslValue meta() const;

  virtual bool hashable() const {
    return false;
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual ~LValue() {}

  protected:

  virtual int compare(const d::Data* rhs) const = 0;

  LValue(d::DslValue& m) : metaObj(m) {}
  LValue() {}

  d::DslValue metaObj;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFalse : public LValue {

  virtual stdstr toString(bool pretty) const;
  virtual ~LFalse() {}
  LFalse() {}

  protected:
  virtual int compare(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LTrue : public LValue {

  virtual stdstr toString(bool pretty) const;
  virtual ~LTrue() {}
  LTrue() {}

  protected:
  virtual int compare(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNil : public LValue {

  virtual stdstr toString(bool pretty) const;
  virtual ~LNil() {}
  LNil() {}

  protected:
  virtual int compare(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNumber : public LValue {
  virtual d::NumberSlot number() const = 0;
  virtual ~LNumber() {}
  protected:
  LNumber(d::DslValue&);
  LNumber() {};
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LInt : public LNumber {

  virtual d::DslValue withMeta(d::DslValue& ) const;
  virtual d::NumberSlot number() const {
    return d::NumberSlot(value);
  }
  LInt(const LInt& rhs, d::DslValue&);
  LInt(llong n) : value(n) {};
  virtual stdstr toString(bool pretty) const;
  llong impl() const { return value; };

  protected:
  virtual int compare(const d::Data*) const;
  llong value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFloat : public LNumber {

  virtual d::DslValue withMeta(d::DslValue& ) const;
  virtual stdstr toString(bool pretty) const;
  LFloat(const LFloat& rhs, d::DslValue&);
  LFloat(double n) : value(n) {};
  double impl() const { return value; };

  virtual d::NumberSlot number() const {
    return d::NumberSlot(value);
  }

  protected:
  virtual int compare(const d::Data*) const;
  double value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LStrk : public LValue {
  virtual stdstr impl() const { return value; }
  protected:
  LStrk(const LStrk&, d::DslValue&);
  LStrk(stdstr& s) : value(s) {}
  virtual int compare(const d::Data* rhs) const;
  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LString : public LStrk {
  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool hashable() const { return true; }
  LString(const LString& rhs, d::DslValue&);
  LString(stdstr&);
  stdstr encoded() const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LKeyword : public LStrk {
  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual bool hashable() const { return true; }
  LKeyword(stdstr& s);
  LKeyword(const LKeyword& rhs, d::DslValue&);
  protected:
  virtual int compare(const d::Data* rhs) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSymbol : public LStrk {
  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual d::DslValue eval(d::IEvaluator*);
  LSymbol(const LSymbol& rhs, d::DslValue&);
  LSymbol(stdstr& s);
  protected:
  virtual int compare(const d::Data*) const;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSeq : public LValue {

  virtual ~LSeq();
  virtual stdstr toString(bool pretty) const;

  d::DslValue get(int pos) const;
  int count() const;
  bool isEmpty() const;

  virtual d::DslValue conj(ValueVec& chunk) const = 0;
  virtual d::DslValue rest() const;
  virtual d::DslValue first() const;

  protected:

  LSeq(const LSeq& rhs, d::DslValue&);
  LSeq(ValueVec& chunk);

  virtual int compare(const d::Data*) const;
  ValueVec evalEach(d::IEvaluator*) const;
  ValueVec values;

  private:

  LSeq();

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LList : public LSeq {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual d::DslValue conj(ValueVec&) const;

  virtual stdstr toString(bool pretty) const;
  virtual d::DslValue eval(d::IEvaluator*);

  LList(ValueVec&);
  LList(const LList& rhs, d::DslValue&);

  virtual ~LList() {}

  private:

  LList();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LVec : public LSeq {
  virtual d::DslValue conj(ValueVec& chunk) const;
  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual d::DslValue eval(d::IEvaluator*);

  LVec(const LVec& rhs, d::DslValue& m);
  LVec(ValueVec&);

  virtual ~LVec() {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LHash : public LValue {

  LHash(std::map<stdstr, d::DslValue>&);
  LHash(ValueVec&);
  LHash(const LHash& rhs, d::DslValue&);

  d::DslValue assoc(ValueVec&) const;
  d::DslValue dissoc(ValueVec&) const;
  bool contains(d::DslValue&) const;
  d::DslValue eval(d::IEvaluator*);
  d::DslValue get(d::DslValue&) const;
  d::DslValue keys() const;
  d::DslValue vals() const;

  virtual stdstr toString(bool pretty) const;
  virtual d::DslValue withMeta(d::DslValue&) const;

  private:
  virtual int compare(const d::Data* rhs) const;
  std::map<stdstr,d::DslValue> values;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNativeFn : public LValue {

  virtual d::DslValue withMeta(d::DslValue&) const;
  virtual stdstr toString(bool pretty) const;
  virtual ~LNativeFn() {}

  LNativeFn(const stdstr& name, Func*);
  LNativeFn(const LNativeFn&, d::DslValue&);

  d::DslValue apply(ValueVec&);

  protected:
  virtual int compare(const d::Data* rhs) const;

  private:
  stdstr _name;
  Func* fn;
  LNativeFn();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lisper : public d::IEvaluator {
  virtual d::DslValue setValue(const stdstr&, d::DslValue, bool localOnly);
  virtual d::DslValue getValue(const stdstr&);
  virtual bool containsSymbol(const stdstr&);
  virtual d::DslFrame pushFrame(const stdstr& name);
  virtual d::DslFrame popFrame();
  virtual d::DslFrame peekFrame();
  virtual ~Lisper() {}
  Lisper();

  d::DslValue eval(d::DslValue);

  private:
  d::DslFrame env;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

