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

  virtual d::DslValue eval(d::IEvaluator*) = 0;
  virtual void visit(d::IAnalyzer*) = 0;

  virtual stdstr pr_str() const {
    return token()->getStr();
  }

  virtual ~Ast() {}

  d::DslToken token() const { return _token; }
  int line() const { return _line; }
  void line(int n) { _line=n;}

  int offset() const { return _offset; }
  void offset(int n) { _offset=n;}

  protected:

  Ast(d::DslToken t) : _token(t) {
    _line =0; _offset=0;
  }

  Ast();

  int _offset;
  int _line;
  d::DslToken _token;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FuncCall : public Ast {

  static d::DslAst make(d::DslAst a, const d::AstVec& v) {
    return WRAP_AST(new FuncCall(a,v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~FuncCall() {}

  d::DslAst funcName() const { return fn; }
  d::AstVec& funcArgs()  { return args; }

  private:

  FuncCall(d::DslAst, const d::AstVec&);

  d::DslAst fn;
  d::AstVec args;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolTerm : public Ast {

  static d::DslAst make(const d::AstVec& v) {
    return WRAP_AST(new BoolTerm(v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~BoolTerm() {}

  private:

  BoolTerm(const d::AstVec&);
  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolExpr : public Ast {

  static d::DslAst make(const d::AstVec& v, const d::TokenVec& t) {
    return WRAP_AST(new BoolExpr(v,t));
  }

  virtual d::DslValue eval(d::IEvaluator*);
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

  static d::DslAst make(d::DslAst left, d::DslToken op, d::DslAst right) {
    return WRAP_AST(new RelationOp(left,op,right));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~RelationOp() {}

  private:

  RelationOp(d::DslAst, d::DslToken, d::DslAst);
  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NotFactor : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslAst a) {
    return WRAP_AST(new NotFactor(a));
  }

  virtual stdstr pr_str() const;
  virtual ~NotFactor() {}

  private:

  NotFactor(d::DslAst);

  d::DslAst expr;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {

  static d::DslAst make(d::DslAst left, d::DslToken op, d::DslAst right) {
    return WRAP_AST(new Assignment(left,op,right));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Assignment() {}

  private:

  Assignment(d::DslAst, d::DslToken, d::DslAst);

  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  static d::DslAst make(d::DslAst left, d::DslToken op, d::DslAst right) {
    return WRAP_AST(new BinOp(left,op,right));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~BinOp() {}

  protected:

  BinOp(d::DslAst, d::DslToken, d::DslAst);

  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return WRAP_AST(new Num(t));
  }

  virtual ~Num() {}

  protected:

  Num(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return WRAP_AST(new String(t));
  }

  virtual stdstr pr_str() const;
  virtual ~String() {}

  protected:

  String(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {

  stdstr name() const { return token()->getStr(); }
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return WRAP_AST(new Var(t));
  }

  virtual ~Var() {}

  protected:

  Var(d::DslToken t);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst a) {
    return WRAP_AST(new UnaryOp(t,a));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~UnaryOp() {}

  private:

  UnaryOp(d::DslToken, d::DslAst);
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Run : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return WRAP_AST(new Run(t));
  }

  virtual ~Run() {}

  protected:

  Run(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Restore : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return WRAP_AST(new Restore(t));
  }

  virtual ~Restore() {}

  protected:

  Restore(d::DslToken);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct End : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return WRAP_AST(new End(t));
  }

  virtual ~End() {}

  protected:

  End(d::DslToken);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Read : public Ast {

  static d::DslAst make(d::DslToken t, const d::AstVec& v) {
    return WRAP_AST(new Read(t,v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  virtual ~Read() {}

  protected:

  d::AstVec vars;
  Read(d::DslToken, const d::AstVec&);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSubReturn : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return WRAP_AST(new GoSubReturn(t));
  }

  virtual ~GoSubReturn() {}

  protected:

  GoSubReturn(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSub : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst a) {
    return WRAP_AST(new GoSub(t,a));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~GoSub() {}

  private:

  GoSub(d::DslToken, d::DslAst);
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Goto : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst a) {
    return WRAP_AST(new Goto(t,a));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Goto() {}

  private:

  Goto(d::DslToken, d::DslAst);
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct OnXXX : public Ast {

  static d::DslAst make(d::DslToken t, d::DslToken n, d::TokenVec& v) {
    return WRAP_AST(new OnXXX(t, Var::make(n), v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  virtual ~OnXXX() {}

  private:

  OnXXX(d::DslToken, d::DslAst, d::TokenVec&);
  d::DslAst var;
  IntVec targets;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForNext : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t, d::DslAst var) {
    return WRAP_AST(new ForNext(t, var));
  }

  static d::DslAst make(d::DslToken t) {
    return WRAP_AST(new ForNext(t));
  }

  virtual ~ForNext() {}

  private:

  ForNext(d::DslToken, d::DslAst);
  ForNext(d::DslToken);
  d::DslAst var;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {

  static d::DslAst make(d::DslAst var, d::DslAst init, d::DslAst term, d::DslAst step) {
    return WRAP_AST(new ForLoop(var,init,term,step));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~ForLoop() {}

  private:

  ForLoop(d::DslAst, d::DslAst, d::DslAst, d::DslAst);
  d::DslAst init;
  d::DslAst var;
  d::DslAst term;
  d::DslAst step;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct PrintSep : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return WRAP_AST(new PrintSep(t));
  }

  virtual ~PrintSep() {}

  protected:

  PrintSep(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Print : public Ast {

  static d::DslAst make(d::DslToken t,const d::AstVec& v) {
    return WRAP_AST(new Print(t, v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  virtual stdstr pr_str() const;
  virtual ~Print() {}

  private:

  Print(d::DslToken, const d::AstVec&);
  d::AstVec exprs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IfThen : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst c, d::DslAst n, d::DslAst z) {
    return WRAP_AST(new IfThen(t,c,n,z));
  }

  static d::DslAst make(d::DslToken t, d::DslAst c, d::DslAst n) {
    return WRAP_AST(new IfThen(t,c,n));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~IfThen() {}

  private:

  IfThen(d::DslToken, d::DslAst c, d::DslAst t, d::DslAst z);
  IfThen(d::DslToken, d::DslAst c, d::DslAst t);
  d::DslAst cond;
  d::DslAst then;
  d::DslAst elze;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {

  static d::DslAst make(const std::map<int,d::DslAst>& v) {
    return WRAP_AST(new Program(v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Program() {}

  private:

  Program(const std::map<int,d::DslAst>&);
  d::AstVec vlines;
  std::map<int,int> mlines;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {

  static d::DslAst make(int line, const d::AstVec& v) {
    return WRAP_AST(new Compound(line,v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Compound() {}

  private:

  Compound(int line, const d::AstVec&);
  d::AstVec stmts;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Data : public Ast {

  static d::DslAst make(d::DslToken t, const d::AstVec& vs) {
    return WRAP_AST(new Data(t,vs));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Data() {}

  private:

  Data(d::DslToken, const d::AstVec&);
  d::AstVec data;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Input : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst var, d::DslAst prompt) {
    return WRAP_AST(new Input(t, var, prompt));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Input() {}

  private:

  Input(d::DslToken, d::DslAst var, d::DslAst prompt);
  d::DslAst var;
  d::DslAst prompt;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Comment : public Ast {

  static d::DslAst make(const d::TokenVec& v) {
    return WRAP_AST(new Comment(v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Comment() {}

  private:

  Comment(const d::TokenVec&);
  d::TokenVec tkns;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ArrayDecl : public Ast {

  static d::DslAst make(d::DslToken n, d::DslAst t, const IntVec& v) {
    return WRAP_AST(new ArrayDecl(n,t,v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~ArrayDecl() {}

  private:

  ArrayDecl(d::DslToken, d::DslAst, const IntVec&);
  bool stringType;
  d::DslAst var;
  IntVec ranges;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BasicParser : public d::IParser {

  virtual d::DslToken eat(int wantedToken);
  virtual d::DslToken eat();
  virtual bool isEof() const;

  BasicParser(const Tchar* src);
  virtual ~BasicParser();

  d::DslAst parse();
  int cur();
  char peek();
  bool isCur(int);
  d::DslToken token();

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

