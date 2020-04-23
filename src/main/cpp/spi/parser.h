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
struct EVal : public d::Data {

  static d::DValue make(double d) {
    return WRAP_VAL( EVal,d);
  }
  static d::DValue make(cstdstr& s) {
    return WRAP_VAL( EVal,s);
  }
  static d::DValue make(const Tchar* s) {
    return WRAP_VAL( EVal,s);
  }
  static d::DValue make(llong n) {
    return WRAP_VAL( EVal,n);
  }

  virtual stdstr pr_str(bool p=0) const {
    if (type==d::T_STRING) return str;
    if (type==d::T_REAL) return N_STR(u.r);
    if (type==d::T_INTEGER) return N_STR(u.n);
    return "BIG TROUBLE!";
  }

  virtual bool equals(d::DValue) const {
    return false;
  }

  virtual int compare(d::DValue) const {
    return 0;
  }

  stdstr str;
  int type;
  union {
    double r; llong n; } u;

  private:
  explicit EVal(double d) { u.r=d; type= d::T_REAL; }
  EVal(cstdstr& s) { str=s; type=d::T_STRING; }
  EVal(const Tchar* s) { str=s; type=d::T_STRING; }
  EVal(llong n) { u.n=n; type= d::T_INTEGER; }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Ast : public d::Node {

  virtual d::DValue eval(d::IEvaluator*) = 0;
  virtual void visit(d::IAnalyzer*) = 0;

  virtual stdstr name() const { return _name; }
  virtual ~Ast() {}
  Ast(d::DToken);
  Ast();

  d::DToken token;
  stdstr _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST( BinOp,left,op,right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~BinOp() {}

  d::DAst lhs;
  d::DAst rhs;

  private:
  BinOp(d::DAst, d::DToken, d::DAst);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  static d::DAst make(d::DToken t) {
    return WRAP_AST( Num,t);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
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

  protected:
  String(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {

  static d::DAst make(d::DToken t, d::DAst a) {
    return WRAP_AST( UnaryOp,t,a);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  virtual ~UnaryOp() {}
  d::DAst expr;

  private:
  UnaryOp(d::DToken, d::DAst);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Compound() {}

  static d::DAst make() {
    return WRAP_AST( Compound);
  }

  d::AstVec  statements;

  private:
  Compound();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Var() {}

  static d::DAst make(d::DToken t) {
    return WRAP_AST( Var,t);
  }

  private:
  Var(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Type() {}

  static d::DAst make(d::DToken t) {
    return WRAP_AST( Type,t);
  }

  private:
  Type(d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {

  static d::DAst make(d::DAst left, d::DToken op, d::DAst right) {
    return WRAP_AST( Assignment,left,op,right);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Assignment() {}

  d::DAst lhs;
  d::DAst rhs;

  private:
  Assignment(d::DAst left, d::DToken op, d::DAst right);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {

  virtual d::DValue eval(d::IEvaluator*) {
    return DVAL_NIL;
  }

  static d::DAst make() {
    return WRAP_AST( NoOp);
  }

  virtual void visit(d::IAnalyzer*) {}
  virtual ~NoOp() {}

  private:
  NoOp() { _name= "709394"; }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Param : public Ast {

  static d::DAst make(d::DAst var, d::DAst type) {
    return WRAP_AST( Param,var,type);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Param() {}

  d::DAst var_node;
  d::DAst type_node;

  private:
  Param(d::DAst var, d::DAst type);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {

  static d::DAst make(d::DAst var, d::DAst type) {
    return WRAP_AST( VarDecl,var,type);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~VarDecl() {}

  d::DAst var_node;
  d::DAst type_node;

  private:
  VarDecl(d::DAst var, d::DAst type);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {

  static d::DAst make(d::AstVec& decls, d::DAst c) {
    return WRAP_AST( Block,decls, c);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Block() {}

  d::DAst compound;
  d::AstVec declarations;

  private:
  Block(d::AstVec& decls, d::DAst compound);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {

  static d::DAst make(const stdstr& s, d::AstVec& v, d::DAst b) {
    return WRAP_AST( ProcedureDecl,s,v,b);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual ~ProcedureDecl() {}
  virtual void visit(d::IAnalyzer*);

  d::DAst block;
  std::vector<d::DAst> params;

  private:
  ProcedureDecl(const stdstr&, d::AstVec&, d::DAst block);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {

  static d::DAst make(const stdstr& s, d::AstVec& v, d::DToken t) {
    return WRAP_AST( ProcedureCall,s,v,t);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual ~ProcedureCall() {}
  virtual void visit(d::IAnalyzer*);

  d::AstVec args;
  d::DSymbol proc_symbol;

  private:
  ProcedureCall(const stdstr&, d::AstVec&, d::DToken);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {

  static d::DAst make(const stdstr& s, d::DAst b) {
    return WRAP_AST( Program,s,b);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Program() {}

  d::DAst block;

  private:
  Program(const stdstr&, d::DAst block);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BuiltinTypeSymbol : public d::TypeSymbol {

  static d::DSymbol make(const stdstr& n) {
    return WRAP_SYM( BuiltinTypeSymbol,n);
  }

  ~BuiltinTypeSymbol() {}

  private:
  BuiltinTypeSymbol(cstdstr& n) : d::TypeSymbol(n) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SymTable : public d::Table {

  static d::DTable make(cstdstr& s, d::DTable t) {
    return d::DTable(new SymTable(s,t));
  }

  static d::DTable make(cstdstr& s) {
    return d::DTable(new SymTable(s));
  }

  ~SymTable() {}

  private:
  SymTable(cstdstr&, d::DTable);
  SymTable(cstdstr&);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SimplePascalParser : public d::IParser {
  SimplePascalParser(const Tchar* src);
  virtual ~SimplePascalParser();

  virtual d::DToken eat(int wantedToken);
  virtual d::DToken eat();
  virtual bool isEof() const;

  d::DAst parse();
  int cur();
  char peek();
  bool isCur(int);
  d::DToken token();

  private:

  Lexer* lex;
};




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

