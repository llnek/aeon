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
struct Ast : public d::Node {

  virtual d::DValue eval(d::IEvaluator*) = 0;
  virtual void visit(d::IAnalyzer*) = 0;

  virtual stdstr name() const { return _name; }
  virtual ~Ast() {}
  Ast(d::DToken);

  d::DToken token;
  stdstr _name;

  private:
  Ast() {}
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

  static d::DAst make(d::DToken k) {
    return WRAP_AST( Compound,k);
  }

  d::AstVec  statements;

  private:
  Compound(d::DToken);
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

  static d::DAst make(d::DToken t) {
    return WRAP_AST( NoOp,t);
  }

  virtual void visit(d::IAnalyzer*) {}
  virtual ~NoOp() {}

  private:
  NoOp(d::DToken t) : Ast(t) { _name= "709394"; }
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

  static d::DAst make(d::DToken k,d::AstVec& decls, d::DAst c) {
    return WRAP_AST( Block,k,decls, c);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Block() {}

  d::DAst compound;
  d::AstVec declarations;

  private:
  Block(d::DToken,d::AstVec& decls, d::DAst compound);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {

  static d::DAst make(d::DToken k,const stdstr& s, d::AstVec& v, d::DAst b) {
    return WRAP_AST( ProcedureDecl,k,s,v,b);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual ~ProcedureDecl() {}
  virtual void visit(d::IAnalyzer*);

  d::DAst block;
  std::vector<d::DAst> params;

  private:
  ProcedureDecl(d::DToken,const stdstr&, d::AstVec&, d::DAst block);
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

  static d::DAst make(d::DToken k,const stdstr& s, d::DAst b) {
    return WRAP_AST( Program,k,s,b);
  }

  virtual d::DValue eval(d::IEvaluator*);
  virtual void visit(d::IAnalyzer*);
  virtual ~Program() {}

  d::DAst block;

  private:
  Program(d::DToken,const stdstr&, d::DAst block);
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

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr&, d::DValue, d::Addr);
d::DValue expected(cstdstr&, d::DValue);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
T* vcast(d::DValue v) {
  T obj;
  if (auto p= v.get(); p &&
      typeid(obj)==typeid(*p))
    return s__cast(T,p); else return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
T* vcast(d::DValue v, d::Addr mark) {
  T obj;
  if (auto p= v.get(); p &&
      typeid(obj)==typeid(*p)) return s__cast(T,p);
  if (_1(mark) == 0 &&
      _2(mark) == 0)
    expected(obj.rtti(), v);
  else expected(obj.rtti(), v,mark);
  return P_NIL;
}


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

