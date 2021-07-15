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

#include "lexer.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define PRV(a,x) DCAST(czlab::dsl::Data,a)->pr_str(x).c_str()
#define PRN(a) DCAST(Ast,a)->pr_str().c_str()
#define PSTR(a) (a)->pr_str().c_str()
#define PNAME(T,a) DCAST(T,a)->name()
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define NUMBER_VAL(n) czlab::dsl::Number::make(n)
#define STRING_VAL(s) czlab::dsl::String::make(s)
#define FALSE_VAL() czlab::dsl::Number::make(0)
#define TRUE_VAL() czlab::dsl::Number::make(1)
//#define CHAR_VAL(s) BChar::make(s)
#define FN_VAL(n, f) LibFunc::make(n, f)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic{
namespace a= czlab::aeon;
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef d::DValue (*Invoker) (d::IEvaluator*, d::VSlice);
typedef std::pair<int,int> CheckPt;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Function : public d::Data{

  virtual d::DValue invoke(d::IEvaluator*, d::VSlice)=0;
  virtual d::DValue invoke(d::IEvaluator*)=0;
  stdstr name() const{ return _name; }

  protected:

  virtual ~Function(){}

  stdstr _name;
  Function(){}
  Function(cstdstr& n) : _name(n){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct LibFunc : public Function{

  virtual d::DValue invoke(d::IEvaluator*, d::VSlice);
  virtual d::DValue invoke(d::IEvaluator*);

  virtual stdstr rtti() const{ return "LibFunc"; }

  virtual stdstr pr_str(bool p=0) const;
  virtual int compare(d::DValue) const;
  virtual bool equals(d::DValue) const;

  static d::DValue make(cstdstr& name, Invoker f){
    return WRAP_VAL(LibFunc,name,f);
  }

  static d::DValue make(){
    return WRAP_VAL(LibFunc);
  }

  //internal use only
  LibFunc(){fn=P_NIL;}
  virtual ~LibFunc(){}

  protected:

  LibFunc(cstdstr& name, Invoker);
  Invoker fn;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lambda : public Function{

  virtual d::DValue invoke(d::IEvaluator*, d::VSlice);
  virtual d::DValue invoke(d::IEvaluator*);

  virtual stdstr rtti() const{ return "UserFunc"; }

  static d::DValue make(cstdstr& name,
                        StrVec& pms, d::DAst body){
    return WRAP_VAL(Lambda, name,pms,body);
  }

  virtual stdstr pr_str(bool p=0) const;
  virtual int compare(d::DValue) const;
  virtual bool equals(d::DValue) const;

  virtual ~Lambda(){}
  //internal use only
  Lambda(){ }

  protected:

  StrVec params;
  d::DAst body;
  Lambda(cstdstr&, StrVec&, d::DAst);
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BArray : public d::Data{

  virtual stdstr rtti() const{ return "Array"; }

  static d::DValue make(const IntVec& v){
    return WRAP_VAL(BArray,v);
  }

  static d::DValue make(){
    return WRAP_VAL(BArray);
  }

  d::DValue set(d::VSlice, d::DValue);
  d::DValue get(d::VSlice);

  virtual stdstr pr_str(bool p=0) const;
  virtual int compare(d::DValue) const;
  virtual bool equals(d::DValue) const;

  // internal use only
  BArray(){ value=P_NIL;}
  virtual ~BArray();

  protected:

  BArray(const IntVec&);
  int index(d::VSlice);

  d::ValVec* value;
  IntVec ranges;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BChar : public d::Data{

  virtual stdstr rtti() const{ return "Char"; }

  static d::DValue make(const Tchar c){
    return WRAP_VAL(BChar,c);
  }

  virtual stdstr pr_str(bool p=0) const{
    return stdstr { value};
  }

  virtual bool equals(d::DValue) const;
  virtual int compare(d::DValue) const;

  Tchar impl() const{ return value; }

  // internal use only
  BChar(){ value=0;}

  protected:

  BChar(const Tchar c) : value(c){}
  Tchar value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoopInfo;
typedef std::shared_ptr<ForLoopInfo> DslFLInfo;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoopInfo{

  static DslFLInfo make(cstdstr& v, int n, int p){
    return DslFLInfo(new ForLoopInfo(v,n,p));
  }

  int beginOffset, endOffset;
  int begin, end;
  stdstr var;
  d::DValue init;
  d::DValue step;
  DslFLInfo outer;

  private:

  ForLoopInfo(cstdstr& v, int n, int p){
    var=v; begin=n; end=0;
    beginOffset=p;
    endOffset=0;
  }

};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Basic : public d::IEvaluator, public d::IAnalyzer{

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

  void install(const std::map<int,int>&);
  void uninstall();

  void addLambda(d::DValue);

  void addData(d::DValue);
  d::DValue readData();
  void restore();

  void init_counters();
  void finz_counters();

  void halt(){ running =false; }
  bool isOn() const{ return running; }

  int jumpSub(int target, int from, int pos);
  int retSub();
  int jumpFor(DslFLInfo);
  int endFor(DslFLInfo);
  int jump(int line);

  int poffset(){ auto p= progOffset; progOffset=0; return p;}
  int pc() const{ return progCounter; };
  int incr_pc(){ return ++progCounter; }

  DslFLInfo getCurForLoop() const { return forLoop; }
  DslFLInfo getForLoop(int c, int offset) const;

  // used during analysis
  void xrefForNext(cstdstr&, int n, int pos);
  void xrefForNext(int n, int pos);
  void addForLoop(DslFLInfo);

  //void addr(d::Addr m) { curMark=m; }
  //d::Addr addr() { return curMark;}

  Basic(const Tchar* src) : source(src){}
  d::DValue interpret();
  virtual ~Basic(){}

  private:

  std::map<stdstr,DslFLInfo> forBegins;
  std::map<stdstr,DslFLInfo> forEnds;

  std::stack<CheckPt> gosubReturns;
  std::map<int,int> lines;

  std::map<stdstr,d::DValue> defs;

  d::ValVec dataSlots;
  int dataPtr=0;
  //d::Addr curMark;

  const Tchar* source;
  DslFLInfo forLoop;
  bool running=0;
  int progCounter=0;
  int progOffset=0;

  d::DFrame stack;
  d::DTable symbols;
  void init_lambdas();
  void check(d::DAst);
  d::DFrame root_env();
  d::DValue eval(d::DAst);
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr&, d::DValue, d::Addr);
d::DValue expected(cstdstr&, d::DValue);
d::DValue op_math(d::DValue, int op, d::DValue);
void ensure_data_type(cstdstr&, d::DValue);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
T* vcast(d::DValue v){
  T obj;
  if(auto p= v.get(); p &&
     typeid(obj)==typeid(*p))
    return s__cast(T,p); else return P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
T* vcast(d::DValue v, d::Addr mark){
  T obj;
  if(auto p= v.get(); p &&
     typeid(obj)==typeid(*p)) return s__cast(T,p);
  if(_1(mark) == 0 &&
     _2(mark) == 0)
    expected(obj.rtti(), v);
  else
    expected(obj.rtti(), v,mark);
  return P_NIL;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

