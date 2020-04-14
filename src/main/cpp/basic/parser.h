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
  virtual stdstr pr_str() const=0;

  virtual ~Ast() {}

  d::DslToken token() const { return _token; }
  llong line() const { return _line; }
  void line(llong n) { _line=n;}

  llong offset() const { return _offset; }
  void offset(llong n) { _offset=n;}

  protected:

  Ast(d::DslToken t) : _token(t) { _line =0; _offset=0; }
  Ast();

  d::DslToken _token;
  llong _offset;
  llong _line;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FuncCall : public Ast {

  static d::DslAst make(d::DslAst a, const d::AstVec& v) {
    return d::DslAst(new FuncCall(a,v));
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
    return d::DslAst(new BoolTerm(v));
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
    return d::DslAst(new BoolExpr(v,t));
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
    return d::DslAst(new RelationOp(left,op,right));
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
    return d::DslAst(new NotFactor(a));
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
    return d::DslAst(new Assignment(left,op,right));
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
    return d::DslAst(new BinOp(left,op,right));
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
    return d::DslAst(new Num(t));
  }

  virtual stdstr pr_str() const;
  virtual ~Num() {}

  protected:

  Num(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new String(t));
  }

  virtual stdstr pr_str() const;
  virtual ~String() {}

  protected:

  String(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {

  stdstr name() const { return token()->getLiteralAsStr(); }
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new Var(t));
  }

  virtual stdstr pr_str() const;
  virtual ~Var() {}

  protected:

  Var(d::DslToken t);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst a) {
    return d::DslAst(new UnaryOp(t,a));
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
    return d::DslAst(new Run(t));
  }

  virtual stdstr pr_str() const;
  virtual ~Run() {}

  protected:

  Run(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct End : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new End(t));
  }

  virtual stdstr pr_str() const;
  virtual ~End() {}

  protected:

  End(d::DslToken);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSubReturn : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new GoSubReturn(t));
  }

  virtual stdstr pr_str() const;
  virtual ~GoSubReturn() {}

  protected:

  GoSubReturn(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSub : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst a) {
    return d::DslAst(new GoSub(t,a));
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
    return d::DslAst(new Goto(t,a));
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
struct ForNext : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslAst var) {
    return d::DslAst(new ForNext(var));
  }

  virtual stdstr pr_str() const;
  virtual ~ForNext() {}

  private:

  ForNext(d::DslAst var);
  d::DslAst var;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {

  static d::DslAst make(d::DslAst var, d::DslAst init, d::DslAst term, d::DslAst step) {
    return d::DslAst(new ForLoop(var,init,term,step));
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
    return d::DslAst(new PrintSep(t));
  }

  virtual stdstr pr_str() const;
  virtual ~PrintSep() {}

  protected:

  PrintSep(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Print : public Ast {

  static d::DslAst make(const d::AstVec& v) {
    return d::DslAst(new Print(v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  virtual stdstr pr_str() const;
  virtual ~Print() {}

  private:

  Print(const d::AstVec&);
  d::AstVec exprs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IfThen : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst c, d::DslAst n, d::DslAst z) {
    return d::DslAst(new IfThen(t,c,n,z));
  }

  static d::DslAst make(d::DslToken t, d::DslAst c, d::DslAst n) {
    return d::DslAst(new IfThen(t,c,n));
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

  static d::DslAst make(const std::map<llong,d::DslAst>& v) {
    return d::DslAst(new Program(v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Program() {}

  private:

  Program(const std::map<llong,d::DslAst>&);
  d::AstVec vlines;
  std::map<llong,llong> mlines;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {

  static d::DslAst make(llong line, const d::AstVec& v) {
    return d::DslAst(new Compound(line,v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Compound() {}

  private:

  Compound(llong line, const d::AstVec&);
  d::AstVec stmts;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Input : public Ast {

  static d::DslAst make(d::DslAst var, d::DslAst prompt) {
    return d::DslAst(new Input(var, prompt));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Input() {}

  private:

  Input(d::DslAst var, d::DslAst prompt);
  d::DslAst var;
  d::DslAst prompt;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Comment : public Ast {

  static d::DslAst make(const d::TokenVec& v) {
    return d::DslAst(new Comment(v));
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

  static d::DslAst make(d::DslToken n, d::DslAst t, const std::vector<llong>& v) {
    return d::DslAst(new ArrayDecl(n,t,v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~ArrayDecl() {}

  private:

  ArrayDecl(d::DslToken, d::DslAst, const std::vector<llong>&);
  bool stringType;
  d::DslAst var;
  std::vector<llong> ranges;
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

  llong line() const { return curLine; }
  void setLine(llong n) { curLine=n;}
  d::DslToken getEthereal();

  private:

  Lexer* lex;
  llong curLine;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

