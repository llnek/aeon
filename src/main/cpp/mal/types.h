#pragma once
#include "mal.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::lisp {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LValue : public d::SValue, public d::IAst {

  virtual d::ExprValue eval(d::IEvaluator*) = 0;
  virtual std::string toString() const = 0;
  virtual bool equals(const d::SValue*) const;
  virtual ~LValue() {}

  virtual d::ExprValue withMeta(d::ExprValue ) const;
  d::ExprValue meta() const;

  LValue(d::ExprValue meta);
  LValue();

  protected:

  virtual int compare(const d::SValue* rhs) const = 0;
  d::ExprValue metaObj;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LInt : public LValue {
  LInt(llong n);
  LInt(const LInt& rhs, d::ExprValue m);
  virtual std::string toString() const;
  llong impl() const;
  virtual int compare(const d::SValue* rhs) const;
  virtual d::ExprValue withMeta(d::ExprValue ) const;
  private:
  llong value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LStrk : public LValue {
  LStrk(const LStrk&, d::ExprValue m);
  LStrk(const std::string&)
  std::string impl() const;
  private:
  std::string value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LString : public LStrk {

  virtual d::ExprValue withMeta(d::ExprValue) const;
  virtual int compare(const d::SValue* rhs) const;
  virtual std::string toString() const;

  LString(const LString& rhs, d::ExprValue m);
  LString(const std::string&);
  std::string encoded() const;

};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LKeyword : public LStrk {
  LKeyword(const std::string& s);
  LKeyword(const LKeyword& rhst, d::ExprValue m);
  virtual std::string toString() const;
  virtual int compare(const d::SValue* rhs) const;
  virtual d::ExprValue withMeta(d::ExprValue ) const;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSymbol : public LStrk {
  LSymbol(const LSymbol& rhs, d::ExprValue m);
  LSymbol(const std::string& s);
  virtual std::string toString() const;
  virtual d::ExprValue eval(d::IEvaluator*);
  virtual int compare(const d::SValue* rhs) const;
  virtual d::ExprValue withMeta(d::ExprValue) const;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSeq : public LValue {
  LSeq(const LSeq& rhs, d::ExprValue m);
  LSeq(VVecIter begin, VVecIter end);
  LSeq(ValueVec* );
  virtual ~malSequence();
  virtual std::string toString() const;

  ValueVec* evalItems(d::IEvaluator*) const;

  int count() const;
  bool isEmpty() const;
  d::ExprValue item(int pos) const;

  VVecIter begin() const;
  VVecIter end() const;

  virtual d::ExprValue conj(VVecIter begin, VVecIter end) const = 0;
  virtual int compare(const d::SValue* rhs) const;
  virtual d::ExprValue rest() const;
  d::ExprValue first() const;

  private:

  ValueVec* values;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LList : public LSeq {
  LList(ValueVec*);
  LList(VVecIter begin, VVecIter end);
  LList(const LList& rhs, d::ExprValue m);

  virtual std::string toString() const;
  virtual d::ExprValue eval(d::IEvaluator*);

  virtual d::ExprValue conj(VVecIter, VVecIter) const;
  virtual d::ExprValue withMeta(d::ExprValue) const;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LVec : public LSeq {
  LVec(ValueVec*);
  LVec(VVecIter, VVecIter);
  LVec(const LVec& rhs, d::ExprValue m);

  virtual d::ExprValue eval(d::IEvaluator*);
  virtual std::string toString() const;

  virtual d::ExprValue conj(VVecIter, VVecIter) const;
  virtual d::ExprValue withMeta(d::ExprValue) const;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSet : public LValue {

  LSet(VVecIter, VVecIter, bool isEvaluated);
  LSet(const std::set<d::ExprValue>&);
  LSet(const LSet& rhs, d::ExprValue m);

  d::ExprValue conj(VVecIter, VVecIter) const;
  d::ExprValue disj(VVecIter, VVecIter) const;
  bool contains(d::ExprValue) const;
  d::ExprValue eval(d::IEvaluator);

  virtual std::string toString() const;

  virtual int compare(const d::SValue* rhs) const;

  virtual d::ExprValue withMeta(d::ExprValue) const;

  private:
  std::set<d::ExprValue> values;
  bool m_isEvaluated;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LDict : public LValue {

  LDict(const std::map<std::string,d::ExprValue>&);
  LDict(VVecIter, VVecIter, bool isEvaluated);
  LDict(const LDict& rhs, d::ExprValue m);

  d::ExprValue assoc(VVecIter, VVecIter) const;
  d::ExprValue dissoc(VVecIter, VVecIter) const;
  bool contains(d::ExprValue) const;
  d::ExprValue eval(d::IEvaluator);
  d::ExprValue get(d::ExprValue) const;
  d::ExprValue keys() const;
  d::ExprValue values() const;

  virtual std::string toString() const;

  virtual int compare(const d::SValue* rhs) const;

  virtual d::ExprValue withMeta(d::ExprValue) const;

  private:
  std::map<std::string,d::ExprValue> values;
  bool m_isEvaluated;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

