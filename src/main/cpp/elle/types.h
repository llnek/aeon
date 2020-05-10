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
namespace czlab::elle {
namespace a= czlab::aeon;
namespace d= czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Scheme;
typedef d::DValue (*Invoker) (Scheme*, d::VSlice);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr&, d::DValue, d::Addr);
d::DValue expected(cstdstr&, d::DValue);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
T* vcast(d::DValue v) {
  T obj;
  if (auto p= v.get(); p &&
      typeid(obj)==typeid(*p))
    return s__cast(T,p); else return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
T* vcast(d::DValue v, d::Addr mark) {
  T obj;
  if (auto p= v.get(); p &&
      typeid(obj)==typeid(*p)) return s__cast(T,p);
  if (_1(mark) == 0 &&
      _2(mark) == 0)
    expected(obj.rtti(), v);
  else expected(obj.rtti(), v,mark);
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Scheme {
  d::DValue EVAL(d::DValue, d::DFrame);
  std::pair<int,d::DValue> READ(cstdstr&);
  stdstr PRINT(d::DValue);
  Scheme() { seed=0; }
  ~Scheme() {}

  private:

  int seed;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SValue : public d::Data {

  virtual d::DValue eval(Scheme*, d::DFrame) = 0;
  virtual stdstr rtti() const { return pr_str(0); }
  d::Addr addr() const { return loc; }
  virtual bool truthy() const { return true; }
  virtual ~SValue() {}

  protected:

  SValue(d::Addr a) { loc=a; }
  SValue() {}
  d::Addr loc;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SFalse : public SValue {

  virtual stdstr pr_str(bool=0) const { return "#f"; }
  virtual bool truthy() const { return false; }

  virtual int compare(d::DValue rhs) const {
    return d::is_same(rhs, this)
           ? 0 : pr_str(0).compare(rhs->pr_str(0));
  }

  virtual d::DValue eval(Scheme*,d::DFrame) {
    return SFalse::make();
  }

  virtual bool equals(d::DValue rhs) const {
    return d::is_same(rhs,this);
  }

  static d::DValue make() {
    return _singleton;
  }

  virtual ~SFalse() {}
  SFalse() {}

  private:

  static d::DValue _singleton;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct STrue : public SValue {

  virtual stdstr pr_str(bool=0) const { return "#t"; }

  virtual int compare(d::DValue rhs) const {
    return d::is_same(rhs, this)
           ? 0
           : pr_str(0).compare(rhs->pr_str(0));
  }

  virtual d::DValue eval(Scheme*,d::DFrame) {
    return STrue::make();
  }

  virtual bool equals(d::DValue rhs) const {
    return d::is_same(rhs,this);
  }

  static d::DValue make() {
    return _singleton;
  }

  virtual ~STrue() {}
  STrue() {}

  private:

  static d::DValue _singleton;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SNumber : public SValue {

  double getFloat() const { return isInt() ? (double)num.n : num.r; }
  llong getInt() const { return isInt() ? num.n : (llong)num.r; }
  bool isInt() const { return _type == d::T_INT; }

  static d::DValue make(double d) { return WRAP_VAL(SNumber,d); }
  static d::DValue make(llong n) { return WRAP_VAL(SNumber,n); }
  static d::DValue make(int n) { return WRAP_VAL(SNumber,n); }

  static d::DValue make(d::DToken t) {
    return WRAP_VAL(SNumber,t);
  }

  virtual bool equals(d::DValue rhs) const {
    if (!d::is_same(rhs, this))
      return false;
    else
    { auto p= vcast<SNumber>(rhs);
      return isInt() == p->isInt() &&
             a::fuzzy_equals(getFloat(), p->getFloat()); }
  }

  virtual int compare(d::DValue rhs) const {
    if (!d::is_same(rhs, this))
      return pr_str().compare(rhs->pr_str());
    else
    { auto f= vcast<SNumber>(rhs)->getFloat();
      auto f2= getFloat();
      return a::fuzzy_equals(f, f2) ? 0 : (f2 > f ? 1 : -1); }
  }

  virtual d::DValue eval(Scheme*, d::DFrame) {
    return WRAP_VAL(SNumber, this);
  }

  virtual stdstr pr_str(bool p=0) const {
    return isInt()
           ? N_STR(num.n) : N_STR(num.r);
  }

  virtual ~SNumber() {}
  explicit SNumber(int nn) { _type=d::T_INT; num.n=nn; }
  SNumber() { _type=d::T_INT; num.n=0; }

  protected:

  SNumber(const SNumber* rhs) {
    _type=rhs->_type;
    num=rhs->num;
  }

  SNumber(d::DToken t) : SValue(t->addr()) {
    if (t->type()== d::T_REAL) {
      _type=d::T_REAL; num.r=t->getFloat();
    }
    else
    if (t->type()== d::T_INT) {
      _type=d::T_INT; num.n=t->getInt();
    }
  }

  explicit SNumber(double d) { _type=d::T_REAL; num.r=d; }
  SNumber(llong nn) { _type=d::T_INT; num.n=nn; }

  int _type;
  union { double r; llong n; } num;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SString : public SValue {

  virtual d::DValue eval(Scheme*, d::DFrame) {
    return WRAP_VAL(SString,value);
  }

  virtual stdstr pr_str(bool p=0) const {
    return p ? encoded() : value;
  }

  static d::DValue make(d::DToken t) {
    return WRAP_VAL(SString,t);
  }

  static d::DValue make(cstdstr& s) {
    return WRAP_VAL(SString,s);
  }

  virtual bool equals(d::DValue rhs) const {
    return d::is_same(rhs, this) &&
           value == vcast<SString>(rhs)->value;
  }

  virtual int compare(d::DValue rhs) const {
    if (!d::is_same(rhs, this))
      return pr_str().compare(rhs->pr_str());
    else
      return value.compare(vcast<SString>(rhs)->value);
  }

  virtual bool isEmpty() const { return value.empty(); }
  virtual int count() const { return value.size(); }

  virtual d::DValue nth(int) const;
  stdstr impl() const { return value; }
  stdstr encoded() const;

  virtual ~SString() {}
  SString() {}

  protected:

  SString(d::DToken t) : SValue(t->addr()) { value=t->getStr(); }
  SString(cstdstr& s) { value=s; }

  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SChar : public SValue {

  virtual d::DValue eval(Scheme*, d::DFrame) {
    return WRAP_VAL(SChar,value);
  }

  virtual stdstr pr_str(bool p=0) const {
    auto s= stdstr{value};
    return p ? "#\\"+s : s;
  }

  static d::DValue make(d::Addr a,Tchar c) {
    return WRAP_VAL(SChar,a,c);
  }
  static d::DValue make(Tchar c) {
    return WRAP_VAL(SChar,c);
  }

  virtual bool equals(d::DValue rhs) const {
    return d::is_same(rhs, this) &&
           value == vcast<SChar>(rhs)->value;
  }

  virtual int compare(d::DValue rhs) const {
    if (!d::is_same(rhs, this))
      return pr_str().compare(rhs->pr_str());
    else
    { auto c = vcast<SChar>(rhs)->value;
      return value==c ? 0 : value > c ? 1 : -1; }
  }

  Tchar impl() { return value; }
  virtual ~SChar() {}

  SChar() { value=0;}

  protected:
  SChar(d::Addr a,Tchar c) : SValue(a) { value = c; }
  SChar(Tchar c) { value = c; }
  Tchar value;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SSymbol : public SValue {

  virtual stdstr pr_str(bool p=0) const {
    return value;
  }

  static d::DValue make(d::Addr a,cstdstr& s) {
    return WRAP_VAL(SSymbol,a,s);
  }
  static d::DValue make(d::DToken t) {
    return WRAP_VAL(SSymbol,t);
  }

  static d::DValue make(cstdstr& s) {
    return WRAP_VAL(SSymbol,s);
  }

  virtual d::DValue eval(Scheme*, d::DFrame);

  virtual bool equals(d::DValue rhs) const {
    return d::is_same(rhs,this) &&
           value == vcast<SSymbol>(rhs)->value;
  }

  virtual int compare(d::DValue rhs) const {
    if (!d::is_same(rhs,this))
      return pr_str().compare(rhs->pr_str());
    else
      return value.compare(vcast<SSymbol>(rhs)->value);
  }

  stdstr impl() const { return value; }
  void rename(cstdstr& n) { value=n; }

  virtual ~SSymbol() {}
  SSymbol() {}

  protected:

  SSymbol(d::DToken t) : SValue(t->addr()) { value=t->getStr(); }
  SSymbol(d::Addr a, cstdstr& s) : SValue(a) { value=s; }
  SSymbol(cstdstr& s) { value=s; }

  stdstr value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SNil : public SValue {

  virtual stdstr pr_str(bool=0) const {return "'()";}

  virtual d::DValue eval(Scheme*,d::DFrame) {
    return SNil::make();
  }

  virtual int compare(d::DValue rhs) const {
    return equals(rhs)
           ? 0 : pr_str(0).compare(rhs->pr_str(0));
  }

  virtual bool equals(d::DValue rhs) const {
    return d::is_same(rhs,this);
  }

  static d::DValue make() {
    return _singleton;
  }

  virtual ~SNil() {}
  SNil() {}

  private:
  static d::DValue _singleton;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SPair : public SValue {

  static d::DValue make(d::Addr r, d::DValue a, d::DValue b) {
    return WRAP_VAL(SPair,r,a,b);
  }

  static d::DValue make(d::DValue a, d::DValue b) {
    return WRAP_VAL(SPair,a,b);
  }

  static d::DValue make(d::Addr a, d::DValue v) {
    return make(a, v, SNil::make());
  }

  static d::DValue make(d::DValue v) {
    return make(v, SNil::make());
  }

  virtual d::DValue eval(Scheme*,d::DFrame);
  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  virtual stdstr pr_str(bool=0) const;

  bool hasProperTail() {
    return vcast<SPair>(f2) != P_NIL;
  }

  bool hasNilTail() {
    return vcast<SNil>(f2) != P_NIL;
  }

  bool isDotted() {
    return vcast<SNil>(f2) == P_NIL &&
      vcast<SPair>(f2)== P_NIL;
  }

  d::DValue head() const { return f1; }
  d::DValue tail() const { return f2; }
  void head(d::DValue v) { f1=v;}
  void tail(d::DValue v) { f2=v; }

  virtual ~SPair() {}
  SPair() {}

  private:

  SPair(d::Addr r, d::DValue a, d::DValue b) : SValue(r) {
    f1=a; f2=b;
  }

  SPair(d::DValue a, d::DValue b) {
    f1=a; f2=b;
  }

  d::DValue f1;
  d::DValue f2;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SVec : public SValue {

  static d::DValue make(d::Addr a, d::ValVec& v) {
    return WRAP_VAL(SVec,v,a);
  }

  static d::DValue make(d::ValVec& v) {
    return WRAP_VAL(SVec,v);
  }

  static d::DValue make(d::VSlice s) {
    return WRAP_VAL(SVec,s);
  }

  static d::DValue make() {
    return WRAP_VAL(SVec);
  }

  void evalEach(Scheme*, d::DFrame, d::ValVec&) const;
  //bool contains(d::DValue) const;
  d::DValue nth(int) const;
  bool isEmpty() const { return values.empty(); }
  int count() const { return values.size(); }

  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;
  virtual d::DValue eval(Scheme*, d::DFrame);
  virtual stdstr pr_str(bool p=0) const;
  //virtual d::DValue conj(d::VSlice) const;

  virtual ~SVec() {}
  SVec() {}

  protected:

  SVec(const SVec* rhs);
  SVec(d::VSlice v);
  SVec(d::ValVec& v, d::Addr a);
  SVec(d::ValVec& v);
  SVec(d::DValue,d::DValue,d::DValue);
  SVec(d::DValue);
  SVec(d::DValue,d::DValue);

  d::ValVec values;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SFunction : public SValue {

  virtual d::DValue invoke(Scheme*, d::VSlice) = 0;
  virtual d::DValue invoke(Scheme*) = 0;
  stdstr name() const { return _name; }
  virtual ~SFunction() {}

  protected:

  stdstr _name;
  SFunction(cstdstr& n) : _name(n) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SLambda : public SFunction {

  static d::DValue make(cstdstr& n, const StrVec& s, d::DValue v, d::DFrame f) {
    return WRAP_VAL(SLambda,n, s, v, f);
  }

  static d::DValue make(const StrVec& s, d::DValue v, d::DFrame f) {
    return WRAP_VAL(SLambda,s, v, f);
  }

  static d::DValue make() {
    return WRAP_VAL(SLambda);
  }

  virtual d::DValue invoke(Scheme*, d::VSlice);
  virtual d::DValue invoke(Scheme*);

  virtual stdstr pr_str(bool p=0) const {
    return "#<lambda>@" + name();
  }

  virtual d::DValue eval(Scheme*, d::DFrame) {
    return WRAP_VAL(SLambda, this);
  }

  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  d::DFrame bindContext(d::VSlice);
  SLambda() : SFunction("") {}
  virtual ~SLambda() {}

  d::DValue body;
  StrVec params;
  d::DFrame env;

  protected:

  SLambda(cstdstr&, const StrVec&, d::DValue, d::DFrame);
  SLambda(const StrVec&, d::DValue, d::DFrame);
  SLambda(const SLambda*);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SMacro : public SLambda {

  static d::DValue make(cstdstr& n, const StrVec& s, d::DValue v, d::DFrame f) {
    return WRAP_VAL(SMacro,n, s, v, f);
  }

  static d::DValue make(const StrVec& s, d::DValue v, d::DFrame f) {
    return WRAP_VAL(SMacro,s, v, f);
  }

  static d::DValue make() {
    return WRAP_VAL(SMacro);
  }

  virtual d::DValue eval(Scheme*, d::DFrame) {
    return WRAP_VAL(SMacro,this);
  }

  virtual bool isMacro() const { return 1; }

  virtual stdstr pr_str(bool p=0) const {
    return "#<macro>@" + name();
  }

  virtual ~SMacro() {}
  SMacro() {}

  protected:

  SMacro(const StrVec& s, d::DValue v, d::DFrame e) : SLambda(s,v,e) {}
  SMacro(cstdstr& n, const StrVec& s, d::DValue v, d::DFrame e)
    : SLambda(n,s,v,e)
  {}
  SMacro(const SMacro* rhs) : SLambda(rhs) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SNative : public SFunction {

  static d::DValue make(cstdstr& name, Invoker f) {
    return WRAP_VAL(SNative,name, f);
  }

  static d::DValue make() {
    return WRAP_VAL(SNative);
  }

  virtual stdstr pr_str(bool p=0) const {
    return "#<native>@" + name();
  }

  virtual d::DValue invoke(Scheme*, d::VSlice);
  virtual d::DValue invoke(Scheme*);

  virtual d::DValue eval(Scheme*, d::DFrame) {
    return WRAP_VAL(SNative, this);
  }

  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  SNative() : SFunction("") { S_NIL(fn); }
  virtual ~SNative() {}

  protected:

  SNative(cstdstr& name, Invoker p) : SFunction(name), fn(p) { }
  SNative(const SNative* n) : SFunction(n->name()) {
    fn=n->fn;
  }
  Invoker fn;
};





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(d::VSlice, int from, int to, d::ValVec&);
void appendAll(d::VSlice, int from, d::ValVec&);
void appendAll(SPair*, d::ValVec&);
void appendAll(SPair*, int from, int to, d::ValVec&);
void appendAll(SPair*, int from, d::ValVec&);
void appendAll(d::DValue, d::ValVec&);
void appendAll(d::DValue, int from, int to, d::ValVec&);
void appendAll(d::DValue, int from, d::ValVec&);

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue car(d::DValue);
d::DValue cdr(d::DValue);
d::DValue rest(d::DValue);
d::DValue rest(SPair*);
bool truthy(d::DValue);
bool listQ(d::DValue);
bool listQ(SPair*);
bool atomQ(d::DValue);
bool isNil(d::DValue);
d::DValue nth(d::DValue, int);
d::DValue nth(SPair*, int);
d::DValue setCar(d::DValue, d::DValue);
d::DValue setCdr(d::DValue, d::DValue);
d::DValue second(d::DValue);
d::DValue third(d::DValue);
d::DValue list(d::DValue, d::DValue);
d::DValue list(d::DValue);
d::DValue listStar(d::DValue);
d::DValue cons(d::DValue, d::DValue);
d::DValue reverse(d::DValue);
bool equals(d::DValue, d::DValue);
int length(d::DValue);
int count(SPair*);

d::DValue listToVector(d::DValue);
d::DValue vectorToList(d::DValue);

d::DValue makePair(d::Addr, d::ValVec&);
d::DValue makeList(d::Addr, d::ValVec&);
d::DValue makeList(d::ValVec&);
d::DValue makeList(d::VSlice);


d::DValue evalEach(Scheme*, d::DFrame,d::DValue);
d::DValue evalEach(Scheme*, d::DFrame,SPair*);






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

