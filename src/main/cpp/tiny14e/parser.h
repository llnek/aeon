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
namespace czlab::tiny14e {

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace d = czlab::dsl;
namespace a = czlab::aeon;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Reader {
  virtual stdstr readString()=0;
  virtual double readFloat()=0;
  virtual llong readInt()=0;
  virtual ~Reader() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Writer {
  virtual void writeString(const stdstr&)=0;
  virtual void writeFloat(double)=0;
  virtual void writeInt(llong)=0;
  virtual void writeln()=0;
  virtual ~Writer() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct EvaluatorAPI : public d::IEvaluator,
                      public Reader, public Writer {
  virtual ~EvaluatorAPI() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct AnalyzerAPI : public d::IAnalyzer {
  virtual ~AnalyzerAPI() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Ast : public d::Node {

  virtual d::DslValue eval(d::IEvaluator* e)=0;
  virtual void visit(d::IAnalyzer*) = 0;

  d::DslToken token() const { return _token; }

  virtual stdstr name() const;

  virtual ~Ast() {}

  Ast(d::DslToken t) { _token=t; }

  protected:

  d::DslToken _token;

  Ast();

  private:

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolExpr : public Ast {

  static d::DslAst make(const d::AstVec& a, const d::TokenVec& v) {
    return d::DslAst(new BoolExpr(a,v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~BoolExpr() {}

  private:

  BoolExpr(const d::AstVec&, const d::TokenVec&);
  d::TokenVec ops;
  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolTerm : public Ast {

  static d::DslAst make(const d::AstVec& v) {
    return d::DslAst(new BoolTerm(v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~BoolTerm() {}

  private:

  BoolTerm(const d::AstVec&);
  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RelationOp : public Ast {

  static d::DslAst make(d::DslAst left, d::DslToken op, d::DslAst right) {
    return d::DslAst(new RelationOp(left, op, right));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~RelationOp() {}

  private:

  RelationOp(d::DslAst, d::DslToken, d::DslAst);
  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  static d::DslAst make(d::DslAst left, d::DslToken op, d::DslAst right) {
    return d::DslAst(new BinOp(left, op, right));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~BinOp() {}

  private:

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

  virtual stdstr name() const;
  virtual ~Num() {}

  private:

  Num(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new String(t));
  }

  virtual ~String() {}
  virtual stdstr name() const;

  private:

  String(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NotFactor : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst expr) {
    return d::DslAst(new NotFactor(t, expr));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~NotFactor() {}

  private:

  NotFactor(d::DslToken, d::DslAst);
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst expr) {
    return d::DslAst(new UnaryOp(t, expr));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~UnaryOp() {}

  private:

  UnaryOp(d::DslToken, d::DslAst);
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make() {
    return d::DslAst(new Compound());
  }

  virtual stdstr name() const;
  virtual ~Compound() {}

  d::AstVec statements;

  private:

  Compound();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new Var(t));
  }

  virtual stdstr name() const;
  virtual ~Var() {}

  d::DslSymbol type_symbol;

  protected:

  Var(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarInput : public Var {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new VarInput(t));
  }

  virtual ~VarInput() {}

  private:

  VarInput(d::DslToken t) : Var(t) {}
  d::DslSymbol type_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new Type(t));
  }

  virtual stdstr name() const;
  virtual ~Type() {}

  private:
  Type(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Write : public Ast {

  static d::DslAst make(d::DslToken t, const d::AstVec& v) {
    return d::DslAst(new Write(t, v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Write() {}

  private:

  Write(d::DslToken, const d::AstVec&);
  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Read : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst a) {
    return d::DslAst(new Read(t, a));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Read() {}

  private:

  Read(d::DslToken, d::DslAst);
  d::DslAst var_node;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct WhileLoop : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst c, d::DslAst a) {
    return d::DslAst(new WhileLoop(t, c, a));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~WhileLoop() {}

  private:

  WhileLoop(d::DslToken, d::DslAst cond, d::DslAst);
  d::DslAst cond;
  d::DslAst code;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst v, d::DslAst i, d::DslAst e, d::DslAst s) {
    return d::DslAst(new ForLoop(t,v,i,e,s));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~ForLoop() {}

  private:

  ForLoop(d::DslToken, d::DslAst, d::DslAst, d::DslAst, d::DslAst);
  d::DslAst init;
  d::DslAst term;
  d::DslAst code;
  d::DslAst var_node;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IfThenElse : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst c, d::DslAst n, d::DslAst z) {
    return d::DslAst(new IfThenElse(t,c,n,z));
  }

  static d::DslAst make(d::DslToken t, d::DslAst c, d::DslAst n) {
    return d::DslAst(new IfThenElse(t,c,n));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~IfThenElse() {}

  private:

  IfThenElse(d::DslToken, d::DslAst, d::DslAst, d::DslAst);
  IfThenElse(d::DslToken, d::DslAst, d::DslAst);

  d::DslAst cond;
  d::DslAst then;
  d::DslAst elze;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RepeatUntil : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst c, d::DslAst b) {
    return d::DslAst(new RepeatUntil(t,c,b));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~RepeatUntil() {}

  protected:

  RepeatUntil(d::DslToken, d::DslAst, d::DslAst);

  d::DslAst cond;
  d::DslAst code;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {

  static d::DslAst make(d::DslAst left, d::DslToken op, d::DslAst right) {
    return d::DslAst(new Assignment(left, op, right));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Assignment() {}
  virtual stdstr name() const;

  private:

  Assignment(d::DslAst, d::DslToken, d::DslAst);
  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {

  virtual d::DslValue eval(d::IEvaluator*) { return P_NIL; }
  virtual void visit(d::IAnalyzer*) {}
  virtual stdstr name() const { return "709394"; }
  virtual ~NoOp() {}

  static d::DslAst make() { return d::DslAst(new NoOp()); }

  private:

  NoOp() : Ast(P_NIL) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {

  static d::DslAst make(d::DslAst v, d::DslAst t) {
    return d::DslAst(new VarDecl(v,t));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~VarDecl() {}

  d::DslAst var_node;
  d::DslAst type_node;

  private:

  VarDecl(d::DslAst v, d::DslAst t);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {

  static d::DslAst make(const d::AstVec& v, d::DslAst b) {
    return d::DslAst(new Block(v,b));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Block() {}
  virtual stdstr name() const;

  private:

  Block(const d::AstVec& decls, d::DslAst);
  d::DslAst compound;
  d::AstVec declarations;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {

  static d::DslAst make(d::DslToken t, const d::AstVec& v, d::DslAst b) {
    return d::DslAst(new ProcedureDecl(t,v,b));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureDecl() {}
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;

  private:

  ProcedureDecl(d::DslToken, const d::AstVec&, d::DslAst);
  d::DslAst block;
  d::AstVec params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {

  static d::DslAst make(d::DslToken t, const d::AstVec& v) {
    return d::DslAst(new ProcedureCall(t, v));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureCall() {}
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;

  private:

  ProcedureCall(d::DslToken, const d::AstVec&);
  d::AstVec args;
  d::DslSymbol proc_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst b) {
    return d::DslAst(new Program(t,b));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Program() {}
  virtual stdstr name() const;

  private:

  Program(d::DslToken, d::DslAst);
  d::DslAst block;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct CrenshawParser : public d::IParser {

  CrenshawParser(const char* src);
  virtual ~CrenshawParser();

  Lexer* rdr() { return lex; }

  d::DslAst parse();
  int cur();
  Tchar peek();
  bool isCur(int);
  virtual bool isEof() const;

  d::DslToken token();

  virtual d::DslToken eat();
  virtual d::DslToken eat(int wanted);

  private:

  Lexer* lex;
};




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

