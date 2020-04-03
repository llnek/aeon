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

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CAST(t,x) s__cast(t,x.ptr())
#define AST(x) CAST(Ast,x)
#define TKN(x) CAST(Token,x)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Ast : public d::Node {

  virtual d::DslValue eval(d::IEvaluator*) = 0;
  virtual void visit(d::IAnalyzer*) = 0;
  virtual stdstr pr_str() const=0;

  virtual ~Ast() {}

  Ast(d::DslToken t) : token(t) {}
  Ast();

  d::DslToken token;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FuncCall : public Ast {
  FuncCall(d::DslAst, const d::AstVec&);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~FuncCall() {}

  private:

  d::DslAst fn;
  d::AstVec args;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolTerm : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  BoolTerm(const d::AstVec&);
  virtual ~BoolTerm() {}

  private:

  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolExpr : public Ast {

  BoolExpr(const d::AstVec&, const d::TokenVec&);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~BoolExpr() {}

  private:

  d::TokenVec ops;
  d::AstVec terms;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RelationOp : public Ast {

  RelationOp(d::DslAst left, d::DslToken op, d::DslAst right);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~RelationOp() {}

  private:

  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NotFactor : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  NotFactor(d::DslAst);
  virtual ~NotFactor() {}
  private:
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {
  Assignment(d::DslAst left, d::DslToken op, d::DslAst right);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~Assignment() {}

  private:

  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  BinOp(d::DslAst left, d::DslToken op, d::DslAst right);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~BinOp() {}

  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  Num(d::DslToken);
  virtual ~Num() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  String(d::DslToken);
  virtual ~String() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  Var(d::DslToken t);
  virtual ~Var() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  UnaryOp(d::DslToken, d::DslAst);
  virtual stdstr pr_str() const;
  virtual ~UnaryOp() {}

  private:
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Run : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  Run();
  virtual stdstr pr_str() const;
  virtual ~Run() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct End : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  End();
  virtual stdstr pr_str() const;
  virtual ~End() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSubReturn : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  GoSubReturn();
  virtual stdstr pr_str() const;
  virtual ~GoSubReturn() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct GoSub : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  GoSub(d::DslAst);
  virtual stdstr pr_str() const;
  virtual ~GoSub() {}
  private:
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Goto : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  Goto(d::DslAst);
  virtual stdstr pr_str() const;
  virtual ~Goto() {}
  private:
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {
  ForLoop(d::DslAst var, d::DslAst init, d::DslAst term);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~ForLoop() {}
  private:
  d::DslAst init;
  d::DslAst var;
  d::DslAst term;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Print : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  Print(const d::AstVec&);
  virtual stdstr pr_str() const;
  virtual ~Print() {}
  private:
  d::AstVec exprs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IfThen : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  IfThen(d::DslAst c, d::DslAst t);
  virtual stdstr pr_str() const;
  virtual ~IfThen() {}
  private:
  d::DslAst cond;
  d::DslAst then;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Input : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  Input(d::DslAst var, d::DslAst prompt);
  virtual stdstr pr_str() const;
  virtual ~Input() {}
  private:
  d::DslAst var;
  d::DslAst prompt;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Comment : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  Comment(const std::vector<d::DslToken>&);
  virtual stdstr pr_str() const;
  virtual ~Comment() {}

  private:
  std::vector<d::DslToken> tkns;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ArrayDecl : public Ast {
  ArrayDecl(d::DslAst t, const std::vector<llong>&);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr pr_str() const;
  virtual ~ArrayDecl() {}

  private:
  bool stringType;
  d::DslAst var;
  std::vector<llong> dims;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BasicParser : public d::IParser {
  BasicParser(const Tchar* src);
  virtual ~BasicParser();

  virtual d::DslToken eat(int wantedToken);
  virtual d::DslToken eat();
  virtual bool isEof() const;

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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

