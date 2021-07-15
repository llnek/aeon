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
 * Copyright © 2013-2021, Kenneth Leung. All rights reserved. */

#include "../dsl/dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define HASH_VAL(k,v) std::pair<d::DValue,d::DValue>(k,v)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define MTD_WITH_META(T) \
  d::DValue withMeta(d::DValue m) const{ return d::DValue(new T(this, m)); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::otto{
namespace a= czlab::aeon;
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef bool (*SetCompare) (d::DValue, d::DValue);
typedef std::pair<d::DValue, d::DValue> VPair;
struct Lisper;
typedef d::DValue (*Invoker) (Lisper*, d::VSlice);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define NUMBER_VAL(n) LNumber::make(n)
#define EMPTY_LIST() LList::make()
#define CHAR_VAL(c) LChar::make(c)
#define ATOM_VAL(a) LAtom::make(a)
#define FALSE_VAL() LFalse::make()
#define TRUE_VAL() LTrue::make()
#define NIL_VAL() LNil::make()
#define VEC_VAL(v) LVec::make(v)
#define VEC_VAL2(p1,p2) LVec::make(p1,p2)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define LIST_VAL(v) LList::make(v)
#define LIST_VAL2(p1,p2) LList::make(p1,p2)
#define LIST_VAL3(p1,p2,p3) LList::make(p1,p2,p3)
#define MAP_VAL(v) LHash::make(v)
#define SET_VAL(v) LSet::make(v)
#define KEYWORD_VAL(s) LKeyword::make(s)
#define STRING_VAL(s) LString::make(s)
#define SYMBOL_VAL(s) LSymbol::make(s)
#define FN_VAL(n, f) LNative::make(n, f)
#define MACRO_VAL(n,p,b,e) LMacro::make(n,p,b,e)
#define LAMBDA_VAL(n,p,b,e) LLambda::make(n,p,b,e)
#define BOOL_VAL(b) ((b) ? TRUE_VAL() : FALSE_VAL())
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr&, d::DValue, d::Addr);
d::DValue expected(cstdstr&, d::DValue);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
T* vcast(d::DValue v){
  T obj;
  if(auto p= v.get(); p &&
     typeid(obj)==typeid(*p))
    return s__cast(T,p); else return P_NIL; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
T* vcast(d::DValue v, d::Addr mark){
  T obj;
  if(auto p= v.get(); p &&
     typeid(obj)==typeid(*p))
    return s__cast(T,p);

  if(_1(mark) == 0 && _2(mark) == 0)
    expected(obj.rtti(), v);
  else
    expected(obj.rtti(), v,mark);

  return P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lisper{
  d::DValue macroExpand(d::DValue, d::DFrame);
  d::DValue syntaxQuote(d::DValue, d::DFrame);
  d::DValue evalAst(d::DValue, d::DFrame);
  d::DValue EVAL(d::DValue, d::DFrame);
  std::pair<int,d::DValue> READ(cstdstr&);
  stdstr PRINT(d::DValue);
  Lisper(){ seed=0; }
  ~Lisper(){}

  private:

  int seed;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//A LISP value
struct LValue : public d::Data{

  virtual d::DValue eval(Lisper*, d::DFrame) = 0;
  virtual d::DValue withMeta(d::DValue) const=0;
  virtual stdstr rtti() const{ return pr_str(0); }
  virtual bool truthy() const{ return 1; }
  d::DValue meta() const{ return metaObj; }
  d::Addr addr() const{ return loc; }
  virtual ~LValue(){}

  protected:

  LValue(d::DValue m) : metaObj(m){}
  LValue(d::Addr a){ loc=a; }
  LValue(){}

  d::DValue metaObj;
  d::Addr loc;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSeqable{

  virtual bool contains(d::DValue) const = 0;
  virtual d::DValue first() const = 0;
  virtual d::DValue rest() const = 0;
  virtual d::DValue seq() const = 0;
  virtual d::DValue nth(int) const = 0;
  virtual bool isEmpty() const = 0;
  virtual int count() const = 0 ;
  virtual ~LSeqable(){}

  protected:

  LSeqable(){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFalse : public LValue{

  virtual stdstr pr_str(bool p=0) const{ return "false"; }
  static d::DValue make(){ return WRAP_VAL(LFalse); }

  static d::DValue make(d::Addr a){
    return WRAP_VAL(LFalse,a);
  }

  virtual bool truthy() const{ return 0; }
  MTD_WITH_META(LFalse)

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LFalse);
  }

  virtual bool equals(d::DValue rhs) const{
    return d::is_same(rhs, this);
  }

  virtual int compare(d::DValue rhs) const{
    return d::is_same(rhs, this) ? 0
                                 : pr_str().compare(rhs->pr_str()); }

  virtual ~LFalse(){}
  LFalse(){}

  protected:

  LFalse(d::Addr t) : LValue(t){}
  LFalse(const LFalse*, d::DValue m) : LValue(m){}

};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LTrue : public LValue{

  virtual stdstr pr_str(bool p=0) const{ return "true"; }

  static d::DValue make(){ return WRAP_VAL(LTrue); }

  static d::DValue make(d::Addr t){
    return WRAP_VAL(LTrue,t);
  }

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LTrue);
  }

  virtual int compare(d::DValue rhs) const{
    return d::is_same(rhs, this) ? 0
                                 : pr_str().compare(rhs->pr_str()); }

  virtual bool equals(d::DValue rhs) const{
    return d::is_same(rhs,this);
  }

  MTD_WITH_META(LTrue)

  virtual ~LTrue(){}
  LTrue(){}

  protected:

  LTrue(d::Addr t) : LValue(t){}
  LTrue(const LTrue*, d::DValue m) : LValue(m){}

};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNil : public LValue{

  virtual stdstr pr_str(bool p=0) const{ return "nil"; }
  virtual bool truthy() const{ return 0; }

  static d::DValue make(){ return WRAP_VAL(LNil); }
  static d::DValue make(d::Addr t){
    return WRAP_VAL(LNil,t);
  }

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LNil);
  }

  MTD_WITH_META(LNil)

  virtual bool equals(d::DValue rhs) const{
    return d::is_same(rhs, this);
  }

  virtual int compare(d::DValue rhs) const{
    return d::is_same(rhs, this) ? 0
                                 : pr_str().compare(rhs->pr_str()); }

  virtual ~LNil(){}
  LNil(){}

  protected:

  LNil(d::Addr t) : LValue(t){}
  LNil(const LNil*, d::DValue m) : LValue(m){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LChar : public LValue{

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LChar,value);
  }

  virtual stdstr pr_str(bool p=0) const{
    return stdstr{value};
  }

  static d::DValue make(Tchar c){
    return WRAP_VAL(LChar,c);
  }

  virtual bool equals(d::DValue rhs) const{
    return d::is_same(rhs, this) &&
           value == vcast<LChar>(rhs)->value;
  }

  virtual int compare(d::DValue rhs) const{
    if(!d::is_same(rhs, this))
      return pr_str().compare(rhs->pr_str());
    else{
      auto c = vcast<LChar>(rhs)->value;
      return value==c ? 0 : value > c ? 1 : -1; }
  }

  MTD_WITH_META(LChar)

  Tchar impl(){ return value; }
  virtual ~LChar(){}

  LChar(){ value=0;}

  protected:

  LChar(const LChar* rhs, d::DValue m) : LValue(m){
    value=rhs->value;
  }
  LChar(Tchar c){ value = c; }
  Tchar value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LAtom : public LValue{

  d::DValue reset(d::DValue x){ return (value = x); }
  d::DValue deref() const{ return value; }

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LAtom,value);
  }

  static d::DValue make(d::DValue v){
    return WRAP_VAL(LAtom,v);
  }

  virtual stdstr pr_str(bool p=0) const{
    return "atom(" + value->pr_str(p) + ")";
  }

  virtual bool equals(d::DValue rhs) const{
    return d::is_same(rhs, this) && value->equals(rhs); }

  virtual int compare(d::DValue rhs) const{
    if(!d::is_same(rhs, this))
      return pr_str().compare(rhs->pr_str());
    else
      return value->compare(vcast<LAtom>(rhs)->value);
  }

  MTD_WITH_META(LAtom)

  LAtom(){ value= NIL_VAL();}
  virtual ~LAtom(){}

  protected:

  LAtom(const LAtom* rhs, d::DValue m) : LValue(m){
    value=rhs->value;
  }
  LAtom(d::DValue v){ value=v; }
  d::DValue value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNumber : public LValue{

  double getFloat() const{ return isInt() ? (double)num.n : num.r; }
  llong getInt() const{ return isInt() ? num.n : (llong)num.r; }
  bool isInt() const{ return _type == d::T_INT; }

  static d::DValue make(double d){ return WRAP_VAL(LNumber,d); }
  static d::DValue make(llong n){ return WRAP_VAL(LNumber,n); }
  static d::DValue make(int n){ return WRAP_VAL(LNumber,n); }

  static d::DValue make(d::DToken t){
    return WRAP_VAL(LNumber,t);
  }

  virtual bool equals(d::DValue rhs) const{
    if(!d::is_same(rhs, this))
      return false;
    else{
      auto p= vcast<LNumber>(rhs);
      return isInt() == p->isInt() &&
             a::fuzzy_equals(getFloat(), p->getFloat()); } }

  virtual int compare(d::DValue rhs) const{
    if(!d::is_same(rhs, this))
      return pr_str().compare(rhs->pr_str());
    else{
      auto f= vcast<LNumber>(rhs)->getFloat();
      auto f2= getFloat();
      return a::fuzzy_equals(f, f2) ? 0 : (f2 > f ? 1 : -1); } }

    MTD_WITH_META(LNumber)

    virtual d::DValue eval(Lisper*, d::DFrame){
      return WRAP_VAL(LNumber, this, DVAL_NIL);
  }

  virtual stdstr pr_str(bool p=0) const{
    return isInt() ? N_STR(num.n) : N_STR(num.r);
  }

  virtual ~LNumber(){}
  explicit LNumber(int nn){ _type=d::T_INT; num.n=nn; }
  LNumber(){ _type=d::T_INT; num.n=0; }

  protected:

  LNumber(const LNumber* rhs, d::DValue m) : LValue(m){
    _type=rhs->_type;
    num=rhs->num;
  }
  LNumber(d::DToken t) : LValue(t->addr()){
    if(t->type()== d::T_REAL){
      _type=d::T_REAL; num.r=t->getFloat();
    }else if(t->type()== d::T_INT){
      _type=d::T_INT; num.n=t->getInt();
    }
  }

  explicit LNumber(double d){ _type=d::T_REAL; num.r=d; }
  LNumber(llong nn){ _type=d::T_INT; num.n=nn; }

  int _type;
  union { double r; llong n; } num;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LString : public LValue, public LSeqable{

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LString,value);
  }

  virtual stdstr pr_str(bool p=0) const{
    return p ? encoded() : value;
  }

  static d::DValue make(d::DToken t){
    return WRAP_VAL(LString,t);
  }

  static d::DValue make(cstdstr& s){
    return WRAP_VAL(LString,s);
  }

  virtual bool equals(d::DValue rhs) const{
    return d::is_same(rhs, this) &&
           value == vcast<LString>(rhs)->value;
  }

  virtual int compare(d::DValue rhs) const{
    if (!d::is_same(rhs, this))
      return pr_str().compare(rhs->pr_str());
    else
      return value.compare(vcast<LString>(rhs)->value);
  }

  MTD_WITH_META(LString)

  virtual ~LString(){}

  stdstr encoded() const;
  stdstr impl() const{ return value; }

  virtual bool contains(d::DValue) const;

  virtual d::DValue nth(int) const;

  virtual d::DValue first() const{
    return value.size() == 0
           ? NIL_VAL() : CHAR_VAL(value[0]);
  }

  virtual d::DValue rest() const;

  virtual d::DValue seq() const;

  virtual bool isEmpty() const{ return value.empty(); }
  virtual int count() const{ return value.size(); }

  LString(){}

  protected:

  LString(const LString* rhs, d::DValue m) : LValue(m){
    value=rhs->value;
  }
  LString(d::DToken t) : LValue(t->addr()){ value=t->getStr(); }
  LString(cstdstr& s){ value=s; }
  stdstr value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LKeyword : public LValue{

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LKeyword,value);
  }

  static d::DValue make(d::DToken t){
    return WRAP_VAL(LKeyword,t);
  }

  static d::DValue make(cstdstr& s){
    return WRAP_VAL(LKeyword,s);
  }

  virtual stdstr pr_str(bool p=0) const;
  MTD_WITH_META(LKeyword)

  virtual bool equals(d::DValue rhs) const{
    return d::is_same(rhs, this) &&
           value == vcast<LKeyword>(rhs)->value;
  }

  virtual int compare(d::DValue rhs) const{
    if (!d::is_same(rhs,this))
      return pr_str().compare(rhs->pr_str());
    else
      return value.compare(vcast<LKeyword>(rhs)->value);
  }

  stdstr impl() const{ return value; }

  virtual ~LKeyword(){}
  LKeyword(){};

  protected:

  LKeyword(const LKeyword* rhs, d::DValue m) : LValue(m){
    value=rhs->value;
  }
  LKeyword(d::DToken);
  LKeyword(cstdstr&);
  stdstr value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSymbol : public LValue{

  virtual stdstr pr_str(bool p=0) const{
    return value;
  }

  static d::DValue make(d::DToken t){
    return WRAP_VAL(LSymbol,t);
  }

  static d::DValue make(cstdstr& s){
    return WRAP_VAL(LSymbol,s);
  }

  virtual d::DValue eval(Lisper*, d::DFrame);

  virtual bool equals(d::DValue rhs) const{
    return d::is_same(rhs,this) &&
           value == vcast<LSymbol>(rhs)->value;
  }

  virtual int compare(d::DValue rhs) const{
    if (!d::is_same(rhs,this))
      return pr_str().compare(rhs->pr_str());
    else
      return value.compare(vcast<LSymbol>(rhs)->value);
  }

  stdstr impl() const{ return value; }
  void rename(cstdstr& n){ value=n; }

  MTD_WITH_META(LSymbol)

  virtual ~LSymbol(){}
  LSymbol(){}

  protected:

  LSymbol(const LSymbol* rhs, d::DValue m) : LValue(m){
    value=rhs->value;
  }
  LSymbol(d::DToken t) : LValue(t->addr()){ value=t->getStr(); }
  LSymbol(cstdstr& s){ value=s; }
  stdstr value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSequential : public LValue, public LSeqable{

  void evalEach(Lisper*, d::DFrame, d::ValVec&) const;

  virtual d::DValue conj(d::VSlice) const = 0;
  virtual stdstr pr_str(bool p=0) const;

  virtual bool contains(d::DValue) const;
  virtual d::DValue nth(int) const;
  virtual d::DValue first() const;
  virtual d::DValue rest() const;
  virtual d::DValue seq() const;
  virtual bool isEmpty() const{ return values.empty(); }
  virtual int count() const{ return values.size(); }

  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  protected:

  virtual ~LSequential(){}

  LSequential(const LSequential*, d::DValue);
  LSequential(d::VSlice);
  LSequential(d::ValVec&);
  LSequential(){}
  LSequential(d::Addr a) : LValue(a){}
  d::ValVec values;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LList : public LSequential{

  MTD_WITH_META(LList)

  virtual stdstr pr_str(bool p=0) const;

  virtual d::DValue eval(Lisper*, d::DFrame);

  virtual d::DValue conj(d::VSlice) const;

  virtual ~LList(){}

  static d::DValue make(d::DValue v){
    return WRAP_VAL(LList,v);
  }

  static d::DValue make(d::Addr a,d::DValue v1, d::DValue v2){
    return WRAP_VAL(LList,a,v1, v2);
  }
  static d::DValue make(d::DValue v1, d::DValue v2){
    return WRAP_VAL(LList,v1, v2);
  }

  static d::DValue make(d::Addr a, d::DValue v1, d::DValue v2, d::DValue v3){
    return WRAP_VAL(LList,a,v1, v2, v3);
  }

  static d::DValue make(d::DValue v1, d::DValue v2, d::DValue v3){
    return WRAP_VAL(LList,v1, v2, v3);
  }

  static d::DValue make(d::Addr a, d::ValVec& v){
    return WRAP_VAL(LList,a,v);
  }

  static d::DValue make(d::ValVec& v){
    return WRAP_VAL(LList,v);
  }

  static d::DValue make(d::VSlice s){
    return WRAP_VAL(LList,s);
  }

  static d::DValue make(){
    return WRAP_VAL(LList);
  }

  LList(){}

  protected:

  LList(const LList* rhs, d::DValue m) : LSequential(rhs,m){}
  LList(d::Addr a, d::ValVec& v) : LSequential(v){ loc=a; }
  LList(d::ValVec& v) : LSequential(v){}
  LList(d::VSlice v) : LSequential(v){}

  LList(d::Addr,d::DValue,d::DValue,d::DValue);
  LList(d::DValue,d::DValue,d::DValue);

  LList(d::Addr,d::DValue,d::DValue);
  LList(d::DValue,d::DValue);
  LList(d::DValue);
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LVec : public LSequential{

  static d::DValue make(d::DValue v1, d::DValue v2, d::DValue v3){
    return WRAP_VAL(LVec,v1, v2, v3);
  }

  static d::DValue make(d::DValue v1, d::DValue v2){
    return WRAP_VAL(LVec,v1, v2);
  }

  static d::DValue make(d::DValue v1){
    return WRAP_VAL(LVec,v1);
  }

  static d::DValue make(d::Addr a, d::ValVec& v){
    return WRAP_VAL(LVec,v,a);
  }

  static d::DValue make(d::ValVec& v){
    return WRAP_VAL(LVec,v);
  }

  static d::DValue make(d::VSlice s){
    return WRAP_VAL(LVec,s);
  }

  static d::DValue make(){
    return WRAP_VAL(LVec);
  }

  virtual d::DValue eval(Lisper*, d::DFrame);
  virtual stdstr pr_str(bool p=0) const;
  virtual d::DValue conj(d::VSlice) const;

  MTD_WITH_META(LVec)
  virtual ~LVec(){}
  LVec(){}

  protected:

  LVec(const LVec* rhs, d::DValue m) : LSequential(rhs,m){}
  LVec(d::VSlice v) : LSequential(v){}
  LVec(d::ValVec& v, d::Addr a) : LSequential(v){
    loc=a;
  }
  LVec(d::ValVec& v) : LSequential(v){}

  LVec(d::DValue,d::DValue,d::DValue);
  LVec(d::DValue);
  LVec(d::DValue,d::DValue);
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LSet : public LValue, public LSeqable{

  static d::DValue make(const std::set<d::DValue,SetCompare>& s){
    return WRAP_VAL(LSet,s);
  }

  static d::DValue make(d::DValue v){
    return WRAP_VAL(LSet,v);
  }

  static d::DValue make(d::VSlice s){
    return WRAP_VAL(LSet,s);
  }

  static d::DValue make(d::Addr a, d::ValVec& v){
    return WRAP_VAL(LSet,a,v);
  }

  static d::DValue make(d::ValVec& v){
    return WRAP_VAL(LSet,v);
  }

  static d::DValue make(){
    return WRAP_VAL(LSet);
  }

  virtual d::DValue eval(Lisper*, d::DFrame);
  virtual stdstr pr_str(bool p=0) const;

  MTD_WITH_META(LSet)

  virtual d::DValue first() const{ return nth(0); }
  virtual bool contains(d::DValue) const;
  virtual d::DValue rest() const;
  virtual d::DValue seq() const;
  virtual d::DValue nth(int) const;

  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  virtual bool isEmpty() const{ return values->empty(); }
  virtual int count() const{ return values->size(); }

  d::DValue conj(d::VSlice) const;
  d::DValue get(d::DValue) const;
  d::DValue disj(d::VSlice) const;

  virtual ~LSet();
  LSet();

  protected:

  LSet(const std::set<d::DValue,SetCompare>&);
  LSet(const LSet*, d::DValue);
  LSet(d::DValue);
  LSet(d::VSlice);
  LSet(d::Addr, d::ValVec&);
  LSet(d::ValVec&);

  std::set<d::DValue,SetCompare>* values;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LHash : public LValue, public LSeqable{

  static d::DValue make(const std::map<stdstr, VPair>& s){
    return WRAP_VAL(LHash,s);
  }

  static d::DValue make(d::VSlice s){
    return WRAP_VAL(LHash,s);
  }

  static d::DValue make(d::Addr a, d::ValVec& v){
    return WRAP_VAL(LHash,a,v);
  }

  static d::DValue make(d::ValVec& v){
    return WRAP_VAL(LHash,v);
  }

  static d::DValue make(){
    return WRAP_VAL(LHash);
  }

  virtual d::DValue eval(Lisper*, d::DFrame);
  virtual stdstr pr_str(bool p=0) const;
  d::DValue get(d::DValue) const;

  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  virtual bool isEmpty() const{ return values.empty(); }
  virtual int count() const{ return values.size(); }
  virtual d::DValue first() const{ return nth(0); }

  virtual bool contains(d::DValue) const;
  virtual d::DValue rest() const;
  virtual d::DValue seq() const;
  virtual d::DValue nth(int) const;

  MTD_WITH_META(LHash)

  d::DValue dissoc(d::VSlice) const;
  d::DValue assoc(d::VSlice) const;

  d::DValue keys() const;
  d::DValue vals() const;

  virtual ~LHash(){}
  LHash(){}

  protected:

  LHash(const std::map<stdstr, VPair>&);
  LHash(const LHash* rhs, d::DValue);
  LHash(d::VSlice);

  LHash(d::Addr,d::ValVec&);
  LHash(d::ValVec&);

  std::map<stdstr,VPair> values;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LFunction : public LValue{

  virtual d::DValue invoke(Lisper*, d::VSlice) = 0;
  virtual d::DValue invoke(Lisper*) = 0;

  stdstr name() const{ return _name; }

  protected:

  virtual ~LFunction(){}

  stdstr _name;
  LFunction(cstdstr& n) : _name(n){}
  LFunction(d::DValue m) : LValue(m){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LLambda : public LFunction{

  static d::DValue make(cstdstr& n, const StrVec& s, d::DValue v, d::DFrame f){
    return WRAP_VAL(LLambda,n, s, v, f);
  }

  static d::DValue make(const StrVec& s, d::DValue v, d::DFrame f){
    return WRAP_VAL(LLambda,s, v, f);
  }

  static d::DValue make(){
    return WRAP_VAL(LLambda);
  }

  virtual d::DValue invoke(Lisper*, d::VSlice);
  virtual d::DValue invoke(Lisper*);
  virtual stdstr pr_str(bool p=0) const;

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LLambda, this, DVAL_NIL);
  }

  MTD_WITH_META(LLambda)

  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  d::DFrame bindContext(d::VSlice);
  LLambda() : LFunction(""){}
  virtual ~LLambda(){}

  d::DValue body;
  StrVec params;
  d::DFrame env;

  protected:

  LLambda(cstdstr&, const StrVec&, d::DValue, d::DFrame);
  LLambda(const StrVec&, d::DValue, d::DFrame);
  LLambda(const LLambda*, d::DValue);
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LMacro : public LLambda{

  static d::DValue make(cstdstr& n, const StrVec& s, d::DValue v, d::DFrame f){
    return WRAP_VAL(LMacro,n, s, v, f);
  }

  static d::DValue make(const StrVec& s, d::DValue v, d::DFrame f){
    return WRAP_VAL(LMacro,s, v, f);
  }

  static d::DValue make(){
    return WRAP_VAL(LMacro);
  }

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LMacro,this, DVAL_NIL);
  }

  virtual ~LMacro(){}

  MTD_WITH_META(LMacro)

  virtual bool isMacro() const{ return 1; }
  virtual stdstr pr_str(bool p=0) const;

  LMacro(){}

  protected:

  LMacro(cstdstr&, const StrVec&, d::DValue, d::DFrame);
  LMacro(const StrVec&, d::DValue, d::DFrame);
  LMacro(const LMacro* rhs, d::DValue m) : LLambda(rhs,m){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LNative : public LFunction{

  static d::DValue make(cstdstr& name, Invoker f){
    return WRAP_VAL(LNative,name, f);
  }

  static d::DValue make(){
    return WRAP_VAL(LNative);
  }

  MTD_WITH_META(LNative)

  virtual stdstr pr_str(bool p=0) const;

  virtual d::DValue invoke(Lisper*, d::VSlice);
  virtual d::DValue invoke(Lisper*);

  virtual d::DValue eval(Lisper*, d::DFrame){
    return WRAP_VAL(LNative, this, DVAL_NIL);
  }

  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  LNative() : LFunction(""){ S_NIL(fn); }
  virtual ~LNative(){}

  protected:

  LNative(cstdstr& name, Invoker p) : LFunction(name), fn(p){ }
  LNative(const LNative*, d::DValue);

  Invoker fn;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void appendAll(d::VSlice, int, d::ValVec&);
void appendAll(d::VSlice, int, int, d::ValVec&);
void appendAll(LSeqable*, d::ValVec&);
void appendAll(LSeqable*, int, d::ValVec&);
void appendAll(LSeqable*, int, int, d::ValVec&);
bool truthy(d::DValue);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSequential* cast_sequential(d::DValue, d::Addr);
LSeqable* cast_seqable(d::DValue, d::Addr);
LFunction* cast_function(d::DValue, d::Addr);
LSequential* cast_sequential(d::DValue);
LSeqable* cast_seqable(d::DValue);
LFunction* cast_function(d::DValue);
bool scan_numbers(d::VSlice);







//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

