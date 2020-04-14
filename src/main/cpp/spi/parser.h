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
namespace czlab::spi {
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CAST(t,x) s__cast(t, x.get())
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct EVal : public d::Data {

  static d::DslValue make(double d) {
    return d::DslValue(new EVal(d));
  }
  static d::DslValue make(const stdstr& s) {
    return d::DslValue(new EVal(s));
  }
  static d::DslValue make(const char* s) {
    return d::DslValue(new EVal(s));
  }
  static d::DslValue make(llong n) {
    return d::DslValue(new EVal(n));
  }

  virtual stdstr pr_str(bool p=0) const {
    if (type==d::T_STRING) return str;
    if (type==d::T_REAL) return N_STR(u.r);
    if (type==d::T_INTEGER) return N_STR(u.n);
    return "BIG TROUBLE!";
  }

  virtual bool equals(d::DslValue) const {
    return false;
  }

  virtual int compare(d::DslValue) const {
    return 0;
  }

  stdstr str;
  int type;
  union {
    double r; llong n; } u;

  private:
  explicit EVal(double d) { u.r=d; type= d::T_REAL; }
  EVal(const stdstr& s) { str=s; type=d::T_STRING; }
  EVal(const char* s) { str=s; type=d::T_STRING; }
  EVal(llong n) { u.n=n; type= d::T_INTEGER; }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Ast : public d::Node {

  virtual d::DslValue eval(d::IEvaluator*) = 0;
  virtual void visit(d::IAnalyzer*) = 0;

  virtual stdstr name() const { return _name; }
  virtual ~Ast() {}
  Ast(d::DslToken);
  Ast();

  d::DslToken token;
  stdstr _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  static d::DslAst make(d::DslAst left, d::DslToken op, d::DslAst right) {
    return d::DslAst(new BinOp(left,op,right));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~BinOp() {}

  d::DslAst lhs;
  d::DslAst rhs;

  private:
  BinOp(d::DslAst, d::DslToken, d::DslAst);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new Num(t));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
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

  protected:
  String(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {

  static d::DslAst make(d::DslToken t, d::DslAst a) {
    return d::DslAst(new UnaryOp(t,a));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~UnaryOp() {}
  d::DslAst expr;

  private:
  UnaryOp(d::DslToken, d::DslAst);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Compound() {}

  static d::DslAst make() {
    return d::DslAst(new Compound());
  }

  d::AstVec  statements;

  private:
  Compound();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Var() {}

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new Var(t));
  }

  private:
  Var(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Type() {}

  static d::DslAst make(d::DslToken t) {
    return d::DslAst(new Type(t));
  }

  private:
  Type(d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {

  static d::DslAst make(d::DslAst left, d::DslToken op, d::DslAst right) {
    return d::DslAst(new Assignment(left,op,right));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Assignment() {}

  d::DslAst lhs;
  d::DslAst rhs;

  private:
  Assignment(d::DslAst left, d::DslToken op, d::DslAst right);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {

  virtual d::DslValue eval(d::IEvaluator*) {
    return d::DslValue();
  }

  static d::DslAst make() {
    return d::DslAst(new NoOp());
  }

  virtual void visit(d::IAnalyzer*) {}
  virtual ~NoOp() {}

  private:
  NoOp() { _name= "709394"; }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Param : public Ast {

  static d::DslAst make(d::DslAst var, d::DslAst type) {
    return d::DslAst(new Param(var,type));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Param() {}

  d::DslAst var_node;
  d::DslAst type_node;

  private:
  Param(d::DslAst var, d::DslAst type);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {

  static d::DslAst make(d::DslAst var, d::DslAst type) {
    return d::DslAst(new VarDecl(var,type));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~VarDecl() {}

  d::DslAst var_node;
  d::DslAst type_node;

  private:
  VarDecl(d::DslAst var, d::DslAst type);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {

  static d::DslAst make(d::AstVec& decls, d::DslAst c) {
    return d::DslAst(new Block(decls, c));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Block() {}

  d::DslAst compound;
  d::AstVec declarations;

  private:
  Block(d::AstVec& decls, d::DslAst compound);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {

  static d::DslAst make(const stdstr& s, d::AstVec& v, d::DslAst b) {
    return d::DslAst(new ProcedureDecl(s,v,b));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureDecl() {}
  virtual void visit(d::IAnalyzer*);

  d::DslAst block;
  std::vector<d::DslAst> params;

  private:
  ProcedureDecl(const stdstr&, d::AstVec&, d::DslAst block);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {

  static d::DslAst make(const stdstr& s, d::AstVec& v, d::DslToken t) {
    return d::DslAst(new ProcedureCall(s,v,t));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureCall() {}
  virtual void visit(d::IAnalyzer*);

  d::AstVec args;
  d::DslSymbol proc_symbol;

  private:
  ProcedureCall(const stdstr&, d::AstVec&, d::DslToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {

  static d::DslAst make(const stdstr& s, d::DslAst b) {
    return d::DslAst(new Program(s,b));
  }

  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Program() {}

  d::DslAst block;

  private:
  Program(const stdstr&, d::DslAst block);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BuiltinTypeSymbol : public d::TypeSymbol {

  static d::DslSymbol make(const stdstr& n) {
    return d::DslSymbol(new BuiltinTypeSymbol(n));
  }

  ~BuiltinTypeSymbol() {}

  private:
  BuiltinTypeSymbol(const stdstr& n) : d::TypeSymbol(n) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SymTable : public d::Table {

  static d::DslTable make(const stdstr& s, d::DslTable t) {
    return d::DslTable(new SymTable(s,t));
  }

  static d::DslTable make(const stdstr& s) {
    return d::DslTable(new SymTable(s));
  }

  ~SymTable() {}

  private:
  SymTable(const stdstr&, d::DslTable);
  SymTable(const stdstr&);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SimplePascalParser : public d::IParser {
  SimplePascalParser(const char* src);
  virtual ~SimplePascalParser();

  virtual d::DslToken eat(int wantedToken);
  virtual d::DslToken eat();
  virtual bool isEof() const;

  d::DslAst parse();
  int cur();
  char peek();
  bool isCur(int);
  d::DslToken token();

  private:

  Lexer* lex;
};




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

