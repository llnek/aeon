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

  virtual stdstr toString(bool p) const {
    if (type==d::T_STRING) return str;
    if (type==d::T_REAL) return std::to_string(u.r);
    if (type==d::T_INTEGER) return std::to_string(u.n);
    return "BIG TROUBLE!";
  }

  virtual bool equals(const Data*) const {
    return false;
  }

  virtual int compare(const Data*) const {
    return 0;
  }

  explicit EVal(double d) {
    u.r=d;
    type= d::T_REAL;
  }
  EVal(const stdstr& s) {
    str=s;
    type=d::T_STRING;
  }
  EVal(const char* s) {
    str=s;
    type=d::T_STRING;
  }
  EVal(llong n) {
    u.n=n;
    type= d::T_INTEGER;
  }
  stdstr str;
  int type;
  union {
    double r;
    llong n;
  } u;
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

  protected:
  stdstr _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  BinOp(d::DslAst left, d::DslToken op, d::DslAst right);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~BinOp() {}

  d::DslAst lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual stdstr name() const;
  Num(d::DslToken);
  virtual ~Num() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  String(d::DslToken);
  virtual ~String() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual stdstr name() const;
  UnaryOp(d::DslToken, d::DslAst);
  virtual void visit(d::IAnalyzer*);
  virtual ~UnaryOp() {}
  d::DslAst expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Compound() {}
  Compound();

  std::vector<d::DslAst> statements;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  Var(d::DslToken);
  virtual ~Var() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  Type(d::DslToken);
  virtual ~Type() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {
  Assignment(d::DslAst left, d::DslToken op, d::DslAst right);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Assignment() {}

  d::DslAst lhs; //Var* lhs;
  d::DslAst rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {
  virtual d::DslValue eval(d::IEvaluator*) {
    return d::DslValue();
  }
  virtual void visit(d::IAnalyzer*) {}
  virtual ~NoOp() {}
  NoOp() { _name= "709394"; }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Param : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  Param(d::DslAst var, d::DslAst type);
  virtual ~Param() {}
  d::DslAst var_node;
  d::DslAst type_node;
};

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  VarDecl(d::DslAst var, d::DslAst type);
  virtual ~VarDecl() {}

  d::DslAst var_node;
  d::DslAst type_node;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {
  Block(std::vector<d::DslAst>& decls, d::DslAst compound);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Block() {}

  d::DslAst compound;
  std::vector<d::DslAst> declarations;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {
  ProcedureDecl(const stdstr&, std::vector<d::DslAst>&, d::DslAst block);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureDecl() {}
  virtual void visit(d::IAnalyzer*);

  d::DslAst block;
  std::vector<d::DslAst> params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {
  ProcedureCall(const stdstr&, std::vector<d::DslAst>&, d::DslToken);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureCall() {}
  virtual void visit(d::IAnalyzer*);

  std::vector<d::DslAst> args;
  d::DslSymbol proc_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {
  Program(const stdstr&, d::DslAst block);
  virtual d::DslValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Program() {}

  d::DslAst block;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BuiltinTypeSymbol : public d::TypeSymbol {
  BuiltinTypeSymbol(const stdstr& n) : d::TypeSymbol(n) {}
  ~BuiltinTypeSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SymTable : public d::Table {
  SymTable(const stdstr&, d::DslTable);
  SymTable(const stdstr&);
  ~SymTable() {}
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

  Lexer* rdr() { return lex; }

  private:

  Lexer* lex;
};




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

