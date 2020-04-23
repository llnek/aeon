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
  virtual void writeString(cstdstr&)=0;
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

  virtual d::DValue eval(d::IEvaluator* e)=0;
  virtual void visit(d::IAnalyzer*) = 0;

  d::DToken token() const { return _token; }

  virtual stdstr name() const;

  virtual ~Ast() {}

  Ast(d::DToken t) { _token=t; }

  protected:

  d::DToken _token;

  Ast();

  private:

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolExpr : public Ast {

  static d::DAst make(const d::AstVec& a, const d::TokenVec& v) {
    return WRAP_AST( BoolExpr,a,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
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

  static d::DAst make(const d::AstVec& v) {
    return WRAP_AST( BoolTerm,v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~BoolTerm() {}

  private:

  BoolTerm(const d::AstVec&);
  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RelationOp : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST( RelationOp,left, op, right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~RelationOp() {}

  private:

  RelationOp(d::DAst, d::DToken, d::DAst);
  d::DAst lhs;
  d::DAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST( BinOp,left, op, right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~BinOp() {}

  private:

  BinOp(d::DAst, d::DToken, d::DAst);
  d::DAst lhs;
  d::DAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST( Num,t);
  }

  virtual stdstr name() const;
  virtual ~Num() {}

  private:

  Num(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST( String,t);
  }

  virtual ~String() {}
  virtual stdstr name() const;

  private:

  String(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NotFactor : public Ast {

  static d::DAst make(d::DToken t, d::DAst expr) {
    return WRAP_AST( NotFactor,t, expr);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~NotFactor() {}

  private:

  NotFactor(d::DToken, d::DAst);
  d::DAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {

  static d::DAst make(d::DToken t, d::DAst expr) {
    return WRAP_AST( UnaryOp,t, expr);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~UnaryOp() {}

  private:

  UnaryOp(d::DToken, d::DAst);
  d::DAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make() {
    return WRAP_AST( Compound);
  }

  virtual stdstr name() const;
  virtual ~Compound() {}

  d::AstVec statements;

  private:

  Compound();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST( Var,t);
  }

  virtual stdstr name() const;
  virtual ~Var() {}

  d::DSymbol type_symbol;

  protected:

  Var(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarInput : public Var {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST( VarInput,t);
  }

  virtual ~VarInput() {}

  private:

  VarInput(d::DToken t) : Var(t) {}
  d::DSymbol type_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);

  static d::DAst make(d::DToken t) {
    return WRAP_AST( Type,t);
  }

  virtual stdstr name() const;
  virtual ~Type() {}

  private:
  Type(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Write : public Ast {

  static d::DAst make(d::DToken t, const d::AstVec& v) {
    return WRAP_AST( Write,t, v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Write() {}

  private:

  Write(d::DToken, const d::AstVec&);
  d::AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Read : public Ast {

  static d::DAst make(d::DToken t, d::DAst a) {
    return WRAP_AST( Read,t, a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Read() {}

  private:

  Read(d::DToken, d::DAst);
  d::DAst var_node;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct WhileLoop : public Ast {

  static d::DAst make(d::DToken t, d::DAst c, d::DAst a) {
    return WRAP_AST( WhileLoop,t, c, a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~WhileLoop() {}

  private:

  WhileLoop(d::DToken, d::DAst cond, d::DAst);
  d::DAst cond;
  d::DAst code;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {

  static d::DAst make(d::DToken t, d::DAst v, d::DAst i, d::DAst e, d::DAst s) {
    return WRAP_AST( ForLoop,t,v,i,e,s);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~ForLoop() {}

  private:

  ForLoop(d::DToken, d::DAst, d::DAst, d::DAst, d::DAst);
  d::DAst init;
  d::DAst term;
  d::DAst code;
  d::DAst var_node;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IfThenElse : public Ast {

  static d::DAst make(d::DToken t, d::DAst c, d::DAst n, d::DAst z) {
    return WRAP_AST( IfThenElse,t,c,n,z);
  }

  static d::DAst make(d::DToken t, d::DAst c, d::DAst n) {
    return WRAP_AST( IfThenElse,t,c,n);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~IfThenElse() {}

  private:

  IfThenElse(d::DToken, d::DAst, d::DAst, d::DAst);
  IfThenElse(d::DToken, d::DAst, d::DAst);

  d::DAst cond;
  d::DAst then;
  d::DAst elze;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RepeatUntil : public Ast {

  static d::DAst make(d::DToken t, d::DAst c, d::DAst b) {
    return WRAP_AST( RepeatUntil,t,c,b);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~RepeatUntil() {}

  protected:

  RepeatUntil(d::DToken, d::DAst, d::DAst);

  d::DAst cond;
  d::DAst code;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST( Assignment,left, op, right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Assignment() {}
  virtual stdstr name() const;

  private:

  Assignment(d::DAst, d::DToken, d::DAst);
  d::DAst lhs;
  d::DAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {

  virtual d::DValue eval(d::IEvaluator*) { return P_NIL; }
  virtual void visit(d::IAnalyzer*) {}
  virtual stdstr name() const { return "709394"; }
  virtual ~NoOp() {}

  static d::DAst make() { return WRAP_AST( NoOp); }

  private:

  NoOp() : Ast(P_NIL) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {

  static d::DAst make(d::DAst v, d::DAst t) {
    return WRAP_AST( VarDecl,v,t);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~VarDecl() {}

  d::DAst var_node;
  d::DAst type_node;

  private:

  VarDecl(d::DAst v, d::DAst t);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {

  static d::DAst make(const d::AstVec& v, d::DAst b) {
    return WRAP_AST( Block,v,b);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Block() {}
  virtual stdstr name() const;

  private:

  Block(const d::AstVec& decls, d::DAst);
  d::DAst compound;
  d::AstVec declarations;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {

  static d::DAst make(d::DToken t, const d::AstVec& v, d::DAst b) {
    return WRAP_AST( ProcedureDecl,t,v,b);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual ~ProcedureDecl() {}
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;

  private:

  ProcedureDecl(d::DToken, const d::AstVec&, d::DAst);
  d::DAst block;
  d::AstVec params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {

  static d::DAst make(d::DToken t, const d::AstVec& v) {
    return WRAP_AST( ProcedureCall,t, v);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual ~ProcedureCall() {}
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;

  private:

  ProcedureCall(d::DToken, const d::AstVec&);
  d::AstVec args;
  d::DSymbol proc_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {

  static d::DAst make(d::DToken t, d::DAst b) {
    return WRAP_AST( Program,t,b);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Program() {}
  virtual stdstr name() const;

  private:

  Program(d::DToken, d::DAst);
  d::DAst block;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct CrenshawParser : public d::IParser {

  CrenshawParser(const Tchar* src);
  virtual ~CrenshawParser();

  Lexer* rdr() { return lex; }

  d::DAst parse();
  int cur();
  Tchar peek();
  bool isCur(int);
  virtual bool isEof() const;

  d::DToken token();

  virtual d::DToken eat();
  virtual d::DToken eat(int wanted);

  private:

  Lexer* lex;
};




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

