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
#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace d = czlab::dsl;
namespace a=czlab::aeon;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Ast : public d::Node {

  virtual stdstr pr_str() const { return tok()->getStr(); }
  d::DToken tok() const { return _token; }
  int line() const { return _line; }
  int offset() const { return _offset; }

  void offset(int n) { _offset=n;}
  void line(int n) { _line=n;}

  virtual ~Ast() {}

  protected:

  Ast(d::DToken t) : _token(t) {}

  int _offset=0;
  int _line=0;
  d::DToken _token;

  private:
  Ast() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FuncCall : public Ast {

  static d::DAst make(d::DToken t, d::DAst a, const d::AstVec& v) {
    return WRAP_AST(FuncCall,t,a,v);
  }

  d::AstVec& funcArgs()  { return args; }
  d::DAst funcName() const { return fn; }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    fn->visit(a);
    for (auto& x:args) x->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~FuncCall() {}

  void setArray(d::Addr m) {
    _token=d::Token::make(T_ARRAYINDEX, "[]",m);
  }

  private:

  FuncCall(d::DToken t, d::DAst a, const d::AstVec& v) : Ast(t) {
    s__ccat(args,v);
    fn=a;
  }
  d::DAst fn;
  d::AstVec args;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolTerm : public Ast {

  static d::DAst make(d::DToken t, const d::AstVec& v) {
    return WRAP_AST(BoolTerm,t,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    for(auto& x:terms)x->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~BoolTerm() {}

  private:

  BoolTerm(d::DToken t, const d::AstVec& v) : Ast(t) { s__ccat(terms,v); }
  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolExpr : public Ast {

  static d::DAst make(d::DToken k, const d::AstVec& v, const d::TokenVec& t) {
    return WRAP_AST(BoolExpr,k,v,t);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    for (auto& x:terms) x->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~BoolExpr() {}

  private:

  BoolExpr(d::DToken k, const d::AstVec& v, const d::TokenVec& t) : Ast(k) {
    s__ccat(ops, t);
    s__ccat(terms, v);
  }
  d::TokenVec ops;
  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RelationOp : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST(RelationOp,left,op,right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    lhs->visit(a),rhs->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~RelationOp() {}

  private:

  RelationOp(d::DAst l, d::DToken t, d::DAst r) : Ast(t) {
    lhs=l;
    rhs=r;
  }
  d::DAst lhs;
  d::DAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NotFactor : public Ast {

  static d::DAst make(d::DToken t, d::DAst a) {
    return WRAP_AST(NotFactor,t,a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    expr->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~NotFactor() {}

  private:

  NotFactor(d::DToken t, d::DAst a) : Ast(t) { expr=a; }
  d::DAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST(Assignment,left,op,right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Assignment() {}

  private:

  Assignment(d::DAst l, d::DToken t, d::DAst r) : Ast(t) { lhs=l; rhs=r; }
  d::DAst lhs;
  d::DAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST(BinOp,left,op,right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    lhs->visit(a),rhs->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~BinOp() {}

  protected:

  BinOp(d::DAst l, d::DToken t, d::DAst r) : Ast(t) { lhs=l; rhs=r; }
  d::DAst lhs;
  d::DAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*) {}
  static d::DAst make(d::DToken t) {
    return WRAP_AST(Num,t);
  }

  virtual ~Num() {}

  protected:

  Num(d::DToken t) : Ast(t) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*) {}

  static d::DAst make(d::DToken t) {
    return WRAP_AST(String,t);
  }

  virtual stdstr pr_str() const;
  virtual ~String() {}

  protected:

  String(d::DToken t) : Ast(t) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {

  stdstr name() const { return tok()->getStr(); }
  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*) {}

  static d::DAst make(d::DToken t) {
    return WRAP_AST(Var,t);
  }

  virtual ~Var() {}

  protected:

  Var(d::DToken t) : Ast(t) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {

  static d::DAst make(d::DToken t, d::DAst a) {
    return WRAP_AST(UnaryOp,t,a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    expr->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~UnaryOp() {}

  private:

  UnaryOp(d::DToken t, d::DAst a) : Ast(t) { expr=a; }
  d::DAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Run : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*) {}

  static d::DAst make(d::DToken t) {
    return WRAP_AST(Run,t);
  }

  virtual ~Run() {}

  protected:

  Run(d::DToken t) : Ast(t) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Restore : public Ast {
  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*) {}
  static d::DAst make(d::DToken t) {
    return WRAP_AST(Restore,t);
  }

  virtual ~Restore() {}

  protected:

  Restore(d::DToken t) : Ast(t) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct End : public Ast {
  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*) {}
  static d::DAst make(d::DToken t) {
    return WRAP_AST(End,t);
  }

  virtual ~End() {}

  protected:

  End(d::DToken t) : Ast(t) {}

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Read : public Ast {

  static d::DAst make(d::DToken t, const d::AstVec& v) {
    return WRAP_AST(Read,t,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual stdstr pr_str() const;
  virtual void visit(d::IAnalyzer* a) {
    for (auto& x:vars) x->visit(a);
  }

  virtual ~Read() {}

  protected:

  Read(d::DToken t, const d::AstVec& v) : Ast(t) {
    s__ccat(vars,v);
  }
  d::AstVec vars;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSubReturn : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*) {}
  static d::DAst make(d::DToken t) {
    return WRAP_AST(GoSubReturn,t);
  }

  virtual ~GoSubReturn() {}

  protected:

  GoSubReturn(d::DToken t) : Ast(t) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSub : public Ast {

  static d::DAst make(d::DToken t, d::DAst a) {
    return WRAP_AST(GoSub,t,a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    expr->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~GoSub() {}

  private:

  GoSub(d::DToken t, d::DAst a) : Ast(t) {
    expr=a;
  }
  d::DAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Goto : public Ast {

  static d::DAst make(d::DToken t, d::DAst a) {
    return WRAP_AST(Goto,t,a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    expr->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~Goto() {}

  private:

  Goto(d::DToken t, d::DAst a) : Ast(t) {
    expr=a;
  }
  d::DAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct OnXXX : public Ast {

  static d::DAst make(d::DToken t, d::DToken n, d::TokenVec& v) {
    return WRAP_AST(OnXXX,t, Var::make(n), v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    var->visit(a);
  }
  virtual ~OnXXX() {}

  private:

  OnXXX(d::DToken, d::DAst, d::TokenVec&);
  d::DAst var;
  IntVec targets;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Defun : public Ast {

  static d::DAst make(d::DToken t, d::DAst var, d::AstVec& pms, d::DAst body) {
    return WRAP_AST(Defun,t, var, pms, body);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Defun() {}

  private:

  Defun(d::DToken t, d::DAst v, d::AstVec& p, d::DAst b) : Ast(t) {
    var=v; body=b; s__ccat(params,p);
  }
  d::DAst var, body;
  d::AstVec params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForNext : public Ast {

  static d::DAst make(d::DToken t, d::DAst var) {
    return WRAP_AST(ForNext,t, var);
  }

  static d::DAst make(d::DToken t) {
    return WRAP_AST(ForNext,t);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~ForNext() {}

  private:

  ForNext(d::DToken t, d::DAst v) : Ast(t) { var=v; }
  ForNext(d::DToken t) : Ast(t) {}
  d::DAst var;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {

  static d::DAst make(d::DToken k, d::DAst var, d::DAst init, d::DAst term, d::DAst step) {
    return WRAP_AST(ForLoop,k,var,init,term,step);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~ForLoop() {}

  private:

  ForLoop(d::DToken k, d::DAst v, d::DAst i, d::DAst t, d::DAst s) : Ast(k) {
    var = v; init = i; term = t; step = s;
  }
  d::DAst var,init,term,step;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct PrintSep : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*) {}
  static d::DAst make(d::DToken t) {
    return WRAP_AST(PrintSep,t);
  }

  virtual ~PrintSep() {}

  protected:

  PrintSep(d::DToken t) : Ast(t) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Print : public Ast {

  static d::DAst make(d::DToken t,const d::AstVec& v) {
    return WRAP_AST(Print,t, v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    for (auto& x:exprs) x->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~Print() {}

  private:

  Print(d::DToken t, const d::AstVec& v) : Ast(t) {
    s__ccat(exprs,v);
  }
  d::AstVec exprs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IfThen : public Ast {

  static d::DAst make(d::DToken t, d::DAst c, d::DAst n, d::DAst z) {
    return WRAP_AST(IfThen,t,c,n,z);
  }

  static d::DAst make(d::DToken t, d::DAst c, d::DAst n) {
    return WRAP_AST(IfThen,t,c,n);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    cond->visit(a);
    then->visit(a);
    if (elze) elze->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~IfThen() {}

  private:

  IfThen(d::DToken k, d::DAst c, d::DAst t, d::DAst z) : Ast(k) { cond=c; then=t; elze=z; }
  IfThen(d::DToken k, d::DAst c, d::DAst t) : Ast(k) { cond=c; then=t; }
  d::DAst cond,then,elze;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {

  static d::DAst make(d::DToken t,const std::map<int,d::DAst>& v) {
    return WRAP_AST(Program,t,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Program() {}

  private:

  Program(d::DToken,const std::map<int,d::DAst>&);
  d::AstVec vlines;
  std::map<int,int> mlines;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {

  static d::DAst make(d::DToken t,int line, const d::AstVec& v) {
    return WRAP_AST(Compound,t,line,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    for (auto& x:stmts) x->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~Compound() {}

  private:

  Compound(d::DToken,int line, const d::AstVec&);
  d::AstVec stmts;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Data : public Ast {

  static d::DAst make(d::DToken t, const d::AstVec& vs) {
    return WRAP_AST(Data,t,vs);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Data() {}

  private:

  Data(d::DToken t, const d::AstVec& v) : Ast(t) {
    s__ccat(data,v);
  }
  d::AstVec data;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Input : public Ast {

  static d::DAst make(d::DToken t, d::DAst var, d::DAst prompt) {
    return WRAP_AST(Input,t, var, prompt);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer* a) {
    var->visit(a);
    if (prompt) prompt->visit(a);
  }
  virtual stdstr pr_str() const;
  virtual ~Input() {}

  private:

  Input(d::DToken t, d::DAst var, d::DAst prompt) : Ast(t) {
    this->var=var;
    this->prompt=prompt;
  }
  d::DAst var;
  d::DAst prompt;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Comment : public Ast {

  static d::DAst make(d::DToken k, const d::TokenVec& v) {
    return WRAP_AST(Comment,k,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*) {}
  virtual stdstr pr_str() const;
  virtual ~Comment() {}

  private:

  Comment(d::DToken k, const d::TokenVec& v) : Ast(k) {
    s__ccat(tkns,v);
  }
  d::TokenVec tkns;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ArrayDecl : public Ast {

  static d::DAst make(d::DToken n, d::DAst t, const IntVec& v) {
    return WRAP_AST(ArrayDecl,n,t,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~ArrayDecl() {}

  private:

  ArrayDecl(d::DToken, d::DAst, const IntVec&);
  bool stringType;
  d::DAst var;
  IntVec ranges;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BasicParser : public d::IParser {

  virtual d::DToken eat(int wantedToken);
  virtual d::DToken eat();
  virtual bool isEof() const;

  int line() const { return curLine; }
  void setLine(int n) { curLine=n;}

  BasicParser(const Tchar* src);
  virtual ~BasicParser();

  d::DAst parse();
  int cur();
  char peek();
  bool isCur(int);
  d::DToken tok();

  private:

  Lexer* lex;
  int curLine;
};









//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

