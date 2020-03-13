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
namespace a= czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SInt : public d::Data {
  stdstr toString(bool pretty) const { return std::to_string(value); }
  virtual bool equals(const Data* rhs) const {
    return X_NIL(rhs) &&
           typeid(*this) == typeid(*rhs) &&
           value == s__cast(const SInt,rhs)->value;
  }
  SInt(llong n) { value=n; }
  llong value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SReal : public d::Data {
  stdstr toString(bool pretty) const { return std::to_string(value); }
  virtual bool equals(const Data* rhs) const {
    return X_NIL(rhs) &&
           typeid(*this) == typeid(*rhs) &&
           a::dbl_equals(value, s__cast(const SReal,rhs)->value);
  }
  SReal(double d)  { value=d; }
  double value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SStr : public d::Data {
  virtual bool equals(const Data* rhs) const {
    return X_NIL(rhs) &&
           typeid(*this) == typeid(*rhs) &&
           value == s__cast(const SStr,rhs)->value;
  }
  stdstr toString(bool pretty) const { return value; }
  SStr(const stdstr& s) : value(s) {}
  SStr(const char* s) : value(s) {}
  stdstr value;
};

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
  Token* token() { return s__cast(Token, _token.ptr()); }
  virtual d::DslValue eval(d::IEvaluator* e)=0;
  virtual void visit(d::IAnalyzer*) = 0;
  virtual stdstr name() = 0;
  virtual ~Ast() {}
  //DslToken token() { return _token; }
  Ast(d::DslToken t) { _token=t; }
  protected:
  d::DslToken _token;
  private:
  Ast() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolExpr : public Ast {

  BoolExpr(d::DslToken, const AstVec&, const TokenVec&);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  stdstr name();
  virtual ~BoolExpr() {}

  private:
  TokenVec ops;
  AstVec terms;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolTerm : public Ast {

  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  stdstr name();
  BoolTerm(d::DslToken,const AstVec&);
  virtual ~BoolTerm() {}

  private:
  AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RelationOp : public Ast {

  RelationOp(d::DslAst left, d::DslToken op, d::DslAst right);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  stdstr name();
  virtual ~RelationOp() {}

  private:
  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  BinOp(d::DslAst left, d::DslToken op, d::DslAst right);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  stdstr name();
  virtual ~BinOp() {}

  private:
  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Num(d::DslToken t);
  stdstr name();
  virtual ~Num() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  String(d::DslToken t);
  virtual ~String() {}
  stdstr name();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NotFactor : public Ast {
  d::DslValue eval(d::IEvaluator*);
  NotFactor(d::DslToken, d::DslAst expr);
  void visit(d::IAnalyzer*);
  virtual ~NotFactor() {}
  stdstr name();
  private:
  d::DslAst expr;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {
  d::DslValue eval(d::IEvaluator*);
  UnaryOp(d::DslToken t, d::DslAst expr);
  void visit(d::IAnalyzer*);
  virtual ~UnaryOp() {}
  stdstr name();
  private:
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Compound() {}
  Compound(d::DslToken);
  stdstr name();

  //private:
  AstVec statements;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Var(d::DslToken t);
  virtual ~Var() {}
  virtual stdstr name();

  //private:
  d::DslSymbol type_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarInput : public Var {
  VarInput(d::DslToken t) : Var(t) {}
  virtual ~VarInput() {}
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  private:
  d::DslSymbol type_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Type(d::DslToken token);
  virtual ~Type() {}
  virtual stdstr name();
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Write : public Ast {
  d::DslValue eval(d::IEvaluator*);
  Write(d::DslToken, const AstVec&);
  void visit(d::IAnalyzer*);
  virtual ~Write() {}
  virtual stdstr name();

  private:
  AstVec terms;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Read : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Read(d::DslToken, d::DslAst);//VarInput*);
  virtual ~Read() {}
  virtual stdstr name();

  private:
  d::DslAst var_node;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct WhileLoop : public Ast {
  WhileLoop(d::DslToken, d::DslAst cond, d::DslAst);//Compound* code);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~WhileLoop() {}
  virtual stdstr name();

  private:
  d::DslAst cond;
  d::DslAst code;//Compound* code;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {
  ForLoop(d::DslToken, d::DslAst v, d::DslAst i, d::DslAst e, d::DslAst);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~ForLoop() {}
  virtual stdstr name();

  private:
  d::DslAst init;
  d::DslAst term;
  d::DslAst code;
  d::DslAst var_node;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IfThenElse : public Ast {
  IfThenElse(d::DslToken, d::DslAst cond, d::DslAst then, d::DslAst elze);
  IfThenElse(d::DslToken, d::DslAst cond, d::DslAst then);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~IfThenElse() {}
  virtual stdstr name();

  private:
  d::DslAst cond;
  d::DslAst then;
  d::DslAst elze;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RepeatUntil : public Ast {
  RepeatUntil(d::DslToken, d::DslAst cond, d::DslAst code);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~RepeatUntil() {}
  virtual stdstr name();

  private:
  d::DslAst cond;
  d::DslAst code;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {
  Assignment(d::DslAst left, d::DslToken op, d::DslAst right);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Assignment() {}
  virtual stdstr name();

  private:
  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {
  d::DslValue eval(d::IEvaluator*) {
    return d::DslValue();
  }
  void visit(d::IAnalyzer*) {}
  virtual stdstr name() { return "709394"; }
  virtual ~NoOp() {}
  NoOp() : Ast(d::DslToken()){}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  VarDecl(d::DslAst v, d::DslAst t);
  virtual ~VarDecl() {}
  virtual stdstr name();

  //private:
  d::DslAst var_node;
  d::DslAst type_node;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {
  Block(d::DslToken, const AstVec& decls, d::DslAst);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Block() {}
  virtual stdstr name();

  private:
  d::DslAst compound;
  AstVec declarations;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {
  ProcedureDecl(d::DslToken, const AstVec&, d::DslAst block);
  d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureDecl() {}
  void visit(d::IAnalyzer*);
  virtual stdstr name();

  private:
  d::DslAst block;
  AstVec params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {
  ProcedureCall(d::DslToken, const AstVec&);
  d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureCall() {}
  void visit(d::IAnalyzer*);
  virtual stdstr name();

  private:
  AstVec args;
  d::DslSymbol proc_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {
  Program(d::DslToken, d::DslAst block);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Program() {}
  virtual stdstr name();

  private:
  d::DslAst block;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct CrenshawParser : public d::IParser {
  CrenshawParser(const char* src);
  virtual ~CrenshawParser();
  d::DslAst parse();
  int cur();
  char peek();
  bool isCur(int);
  bool isEof();
  d::DslToken token();
  d::DslToken eat();
  d::DslToken eat(int wanted);
  d::DslToken getEthereal();

  private:
  Lexer* lex;
};




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

