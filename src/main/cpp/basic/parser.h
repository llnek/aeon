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

  virtual d::DValue eval(d::IEvaluator*) = 0;
  virtual void visit(d::IAnalyzer*) = 0;

  virtual stdstr pr_str() const {
    return token()->getStr();
  }

  virtual ~Ast() {}

  d::DToken token() const { return _token; }
  int line() const { return _line; }
  void line(int n) { _line=n;}

  int offset() const { return _offset; }
  void offset(int n) { _offset=n;}

  protected:

  Ast(d::DToken t) : _token(t) {
    _line =0; _offset=0;
  }

  Ast();

  int _offset;
  int _line;
  d::DToken _token;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FuncCall : public Ast {

  static d::DAst make(d::DAst a, const d::AstVec& v) {
    return WRAP_AST(FuncCall,a,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~FuncCall() {}

  d::DAst funcName() const { return fn; }
  d::AstVec& funcArgs()  { return args; }

  private:

  FuncCall(d::DAst, const d::AstVec&);

  d::DAst fn;
  d::AstVec args;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolTerm : public Ast {

  static d::DAst make(const d::AstVec& v) {
    return WRAP_AST(BoolTerm,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~BoolTerm() {}

  private:

  BoolTerm(const d::AstVec&);
  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolExpr : public Ast {

  static d::DAst make(const d::AstVec& v, const d::TokenVec& t) {
    return WRAP_AST(BoolExpr,v,t);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~BoolExpr() {}

  private:

  BoolExpr(const d::AstVec&, const d::TokenVec&);

  d::TokenVec ops;
  d::AstVec terms;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RelationOp : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST(RelationOp,left,op,right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~RelationOp() {}

  private:

  RelationOp(d::DAst, d::DToken, d::DAst);
  d::DAst lhs;
  d::DAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NotFactor : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DAst a) {
    return WRAP_AST(NotFactor,a);
  }

  virtual stdstr pr_str() const;
  virtual ~NotFactor() {}

  private:

  NotFactor(d::DAst);

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

  Assignment(d::DAst, d::DToken, d::DAst);

  d::DAst lhs;
  d::DAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST(BinOp,left,op,right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~BinOp() {}

  protected:

  BinOp(d::DAst, d::DToken, d::DAst);

  d::DAst lhs;
  d::DAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST(Num,t);
  }

  virtual ~Num() {}

  protected:

  Num(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST(String,t);
  }

  virtual stdstr pr_str() const;
  virtual ~String() {}

  protected:

  String(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {

  stdstr name() const { return token()->getStr(); }
  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST(Var,t);
  }

  virtual ~Var() {}

  protected:

  Var(d::DToken t);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {

  static d::DAst make(d::DToken t, d::DAst a) {
    return WRAP_AST(UnaryOp,t,a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~UnaryOp() {}

  private:

  UnaryOp(d::DToken, d::DAst);
  d::DAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Run : public Ast {
  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST(Run,t);
  }

  virtual ~Run() {}

  protected:

  Run(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Restore : public Ast {
  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST(Restore,t);
  }

  virtual ~Restore() {}

  protected:

  Restore(d::DToken);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct End : public Ast {
  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST(End,t);
  }

  virtual ~End() {}

  protected:

  End(d::DToken);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Read : public Ast {

  static d::DAst make(d::DToken t, const d::AstVec& v) {
    return WRAP_AST(Read,t,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  virtual ~Read() {}

  protected:

  d::AstVec vars;
  Read(d::DToken, const d::AstVec&);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSubReturn : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST(GoSubReturn,t);
  }

  virtual ~GoSubReturn() {}

  protected:

  GoSubReturn(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSub : public Ast {

  static d::DAst make(d::DToken t, d::DAst a) {
    return WRAP_AST(GoSub,t,a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~GoSub() {}

  private:

  GoSub(d::DToken, d::DAst);
  d::DAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Goto : public Ast {

  static d::DAst make(d::DToken t, d::DAst a) {
    return WRAP_AST(Goto,t,a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Goto() {}

  private:

  Goto(d::DToken, d::DAst);
  d::DAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct OnXXX : public Ast {

  static d::DAst make(d::DToken t, d::DToken n, d::TokenVec& v) {
    return WRAP_AST(OnXXX,t, Var::make(n), v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  virtual ~OnXXX() {}

  private:

  OnXXX(d::DToken, d::DAst, d::TokenVec&);
  d::DAst var;
  IntVec targets;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Defun : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t, d::DAst var, d::AstVec& pms, d::DAst body) {
    return WRAP_AST(Defun,t, var, pms, body);
  }

  virtual ~Defun() {}

  private:

  Defun(d::DToken, d::DAst, d::AstVec&, d::DAst);
  d::DAst var;
  d::DAst body;
  d::AstVec params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForNext : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t, d::DAst var) {
    return WRAP_AST(ForNext,t, var);
  }

  static d::DAst make(d::DToken t) {
    return WRAP_AST(ForNext,t);
  }

  virtual ~ForNext() {}

  private:

  ForNext(d::DToken, d::DAst);
  ForNext(d::DToken);
  d::DAst var;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {

  static d::DAst make(d::DAst var, d::DAst init, d::DAst term, d::DAst step) {
    return WRAP_AST(ForLoop,var,init,term,step);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~ForLoop() {}

  private:

  ForLoop(d::DAst, d::DAst, d::DAst, d::DAst);
  d::DAst init;
  d::DAst var;
  d::DAst term;
  d::DAst step;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct PrintSep : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST(PrintSep,t);
  }

  virtual ~PrintSep() {}

  protected:

  PrintSep(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Print : public Ast {

  static d::DAst make(d::DToken t,const d::AstVec& v) {
    return WRAP_AST(Print,t, v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  virtual stdstr pr_str() const;
  virtual ~Print() {}

  private:

  Print(d::DToken, const d::AstVec&);
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
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~IfThen() {}

  private:

  IfThen(d::DToken, d::DAst c, d::DAst t, d::DAst z);
  IfThen(d::DToken, d::DAst c, d::DAst t);
  d::DAst cond;
  d::DAst then;
  d::DAst elze;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {

  static d::DAst make(const std::map<int,d::DAst>& v) {
    return WRAP_AST(Program,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Program() {}

  private:

  Program(const std::map<int,d::DAst>&);
  d::AstVec vlines;
  std::map<int,int> mlines;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {

  static d::DAst make(int line, const d::AstVec& v) {
    return WRAP_AST(Compound,line,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Compound() {}

  private:

  Compound(int line, const d::AstVec&);
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

  Data(d::DToken, const d::AstVec&);
  d::AstVec data;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Input : public Ast {

  static d::DAst make(d::DToken t, d::DAst var, d::DAst prompt) {
    return WRAP_AST(Input,t, var, prompt);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Input() {}

  private:

  Input(d::DToken, d::DAst var, d::DAst prompt);
  d::DAst var;
  d::DAst prompt;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Comment : public Ast {

  static d::DAst make(const d::TokenVec& v) {
    return WRAP_AST(Comment,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Comment() {}

  private:

  Comment(const d::TokenVec&);
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

  BasicParser(const Tchar* src);
  virtual ~BasicParser();

  d::DAst parse();
  int cur();
  char peek();
  bool isCur(int);
  d::DToken token();

  int line() const { return curLine; }
  void setLine(int n) { curLine=n;}

  private:

  Lexer* lex;
  int curLine;
};









//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

