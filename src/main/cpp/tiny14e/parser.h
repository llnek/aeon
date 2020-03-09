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
using namespace czlab::dsl;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SInt : public d::Data {
  virtual stdstr toString() const { return std::to_string(value); }
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
  virtual stdstr toString() const { return std::to_string(value); }
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
  virtual stdstr toString() const { return value; }
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
struct EvaluatorAPI : public IEvaluator,
                      public Reader, public Writer {
  virtual ~EvaluatorAPI() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct AnalyzerAPI : public IAnalyzer {
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
  Ast(DslToken t) { _token=t; }
  protected:
  DslToken _token;
  private:
  Ast() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolExpr : public Ast {

  BoolExpr(DslToken, const AstVec&, const TokenVec&);
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
  BoolTerm(DslToken,const AstVec&);
  virtual ~BoolTerm() {}

  private:
  AstVec terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RelationOp : public Ast {

  RelationOp(DslAst left, DslToken op, DslAst right);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  stdstr name();
  virtual ~RelationOp() {}

  private:
  DslAst lhs;
  DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  BinOp(DslAst left, DslToken op, DslAst right);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  stdstr name();
  virtual ~BinOp() {}

  private:
  DslAst lhs;
  DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Num(DslToken t);
  stdstr name();
  virtual ~Num() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  String(DslToken t);
  virtual ~String() {}
  stdstr name();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NotFactor : public Ast {
  d::DslValue eval(d::IEvaluator*);
  NotFactor(DslToken, DslAst expr);
  void visit(d::IAnalyzer*);
  virtual ~NotFactor() {}
  stdstr name();
  private:
  DslAst expr;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {
  d::DslValue eval(d::IEvaluator*);
  UnaryOp(DslToken t, DslAst expr);
  void visit(d::IAnalyzer*);
  virtual ~UnaryOp() {}
  stdstr name();
  private:
  DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Compound() {}
  Compound(DslToken);
  stdstr name();

  //private:
  AstVec statements;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Var(DslToken t);
  virtual ~Var() {}
  virtual stdstr name();

  //private:
  DslSymbol type_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarInput : public Var {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  VarInput(DslToken t);
  virtual ~VarInput() {}

  private:
  DslSymbol type_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Type(DslToken token);
  virtual ~Type() {}
  virtual stdstr name();
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Write : public Ast {
  d::DslValue eval(d::IEvaluator*);
  Write(DslToken, const AstVec&);
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
  Read(DslToken, DslAst);//VarInput*);
  virtual ~Read() {}
  virtual stdstr name();

  private:
  DslAst var_node;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct WhileLoop : public Ast {
  WhileLoop(DslToken, DslAst cond, DslAst);//Compound* code);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~WhileLoop() {}
  virtual stdstr name();

  private:
  DslAst cond;
  DslAst code;//Compound* code;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {
  ForLoop(DslToken, DslAst v, DslAst i, DslAst e, DslAst);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~ForLoop() {}
  virtual stdstr name();

  private:
  DslAst init;
  DslAst term;
  DslAst code;
  DslAst var_node;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IfThenElse : public Ast {
  IfThenElse(DslToken, DslAst cond, DslAst then, DslAst elze);
  IfThenElse(DslToken, DslAst cond, DslAst then);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~IfThenElse() {}
  virtual stdstr name();

  private:
  DslAst cond;
  DslAst then;
  DslAst elze;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RepeatUntil : public Ast {
  RepeatUntil(DslToken, DslAst cond, DslAst code);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~RepeatUntil() {}
  virtual stdstr name();

  private:
  DslAst cond;
  DslAst code;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {
  Assignment(DslAst left, DslToken op, DslAst right);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Assignment() {}
  virtual stdstr name();

  private:
  DslAst lhs;
  DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {
  d::DslValue eval(d::IEvaluator*) {
    return d::DslValue();
  }
  void visit(d::IAnalyzer*) {}
  virtual stdstr name() { return "709394"; }
  virtual ~NoOp() {}
  NoOp() : Ast(DslToken()){}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/*
struct Param : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Param(DslAst v, DslAst t);
  virtual ~Param() {}
  virtual stdstr name();

  private:
  DslAst var_node;
  DslAst type_node;
};
*/
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  VarDecl(DslAst v, DslAst t);
  virtual ~VarDecl() {}
  virtual stdstr name();

  //private:
  DslAst var_node;
  DslAst type_node;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {
  Block(DslToken, const AstVec& decls, DslAst);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Block() {}
  virtual stdstr name();

  private:
  DslAst compound;
  AstVec declarations;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {
  ProcedureDecl(DslToken, const AstVec&, DslAst block);
  d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureDecl() {}
  void visit(d::IAnalyzer*);
  virtual stdstr name();

  private:
  DslAst block;
  //stdstr _name;
  AstVec params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {
  ProcedureCall(DslToken, const AstVec&);
  d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureCall() {}
  void visit(d::IAnalyzer*);
  virtual stdstr name();

  private:
  //stdstr _name;
  AstVec args;
  DslSymbol proc_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {
  Program(DslToken, DslAst block);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Program() {}
  virtual stdstr name();

  private:
  DslAst block;
  //stdstr _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/*
struct BuiltinTypeSymbol : public d::TypeSymbol {
  BuiltinTypeSymbol(const stdstr& n) : d::TypeSymbol(n) {}
  ~BuiltinTypeSymbol() {}
};*/
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct CrenshawParser : public d::IParser {
  CrenshawParser(const char* src);
  virtual ~CrenshawParser();
  DslAst parse();
  int cur();
  char peek();
  bool isCur(int);
  DslToken token();
  DslToken eat();
  DslToken eat(int wanted);
  DslToken getEthereal();

  private:
  Lexer* lex;
};




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

