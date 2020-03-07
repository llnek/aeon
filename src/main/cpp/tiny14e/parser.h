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
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SInt : public d::Data {
  virtual std::string toString() const { return std::to_string(value); }
  virtual bool equals(const Data* rhs) const { return false; }
  SInt(long n) {
    value=n;
  }
  long value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SReal : public d::Data {
  virtual std::string toString() const { return std::to_string(value); }
  virtual bool equals(const Data* rhs) const { return false; }
  SReal(double d)  {
    value=d;
  }
  double value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SStr : public d::Data {
  virtual std::string toString() const { return value; }
  virtual bool equals(const Data* rhs) const { return false; }
  SStr(const std::string& s) : value(s) {}
  SStr(const char* s) : value(s) {}
  std::string value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Reader {
  virtual std::string readString()=0;
  virtual double readFloat()=0;
  virtual long readInt()=0;
  virtual ~Reader() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Writer {
  virtual void writeString(const std::string&)=0;
  virtual void writeFloat(double)=0;
  virtual void writeInt(long)=0;
  virtual void writeln()=0;
  virtual ~Writer() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct EvaluatorAPI : public d::IEvaluator, public Reader, public Writer {
  virtual ~EvaluatorAPI() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct AnalyzerAPI : public d::IAnalyzer {
  virtual ~AnalyzerAPI() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Ast : public d::IAst {
  virtual d::DslValue eval(d::IEvaluator* e)=0;
  virtual void visit(d::IAnalyzer*) = 0;
  virtual std::string name() = 0;
  virtual ~Ast() {}
  Ast(Token*);
  Ast();
  Token* token;
  protected:
  std::string _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolExpr : public Ast {

  BoolExpr(std::vector<Ast*>&, std::vector<Token*>&);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  virtual ~BoolExpr() {}

  std::vector<Ast*> terms;
  std::vector<Token*> ops;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BoolTerm : public Ast {

  d::DslValue eval(d::IEvaluator*);
  BoolTerm(std::vector<Ast*>&);
  void visit(d::IAnalyzer*);
  std::string name();
  virtual ~BoolTerm() {}

  std::vector<Ast*> terms;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RelationOp : public Ast {

  RelationOp(Ast* left, Token* op, Ast* right);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  virtual ~RelationOp() {}

  Ast* lhs;
  Ast* rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {

  BinOp(Ast* left, Token* op, Ast* right);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  virtual ~BinOp() {}

  Ast* lhs;
  Ast* rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Num(Token* t);
  std::string name();
  virtual ~Num() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  String(Token* t);
  virtual ~String() {}
  std::string name();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NotFactor : public Ast {
  d::DslValue eval(d::IEvaluator*);
  NotFactor(Ast* expr);
  void visit(d::IAnalyzer*);
  virtual ~NotFactor() {}
  std::string name();
  Ast* expr;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {
  d::DslValue eval(d::IEvaluator*);
  UnaryOp(Token* t, Ast* expr);
  void visit(d::IAnalyzer*);
  virtual ~UnaryOp() {}
  std::string name();
  Ast* expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Compound() {}
  Compound();
  std::string name();

  std::vector<Ast*> statements;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Var(Token* t);
  virtual ~Var() {}
  std::string name();
  d::TypeSymbol* type_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarInput : public Var {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  VarInput(Token* t);
  virtual ~VarInput() {}
  d::TypeSymbol* type_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Type(Token* token);
  virtual ~Type() {}
  std::string name();
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Write : public Ast {
  Write(Token*, std::vector<Ast*>&);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Write() {}
  std::string name();
  std::vector<Ast*> terms;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Read : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Read(Token*, VarInput*);
  virtual ~Read() {}
  std::string name();
  VarInput* var_node;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct WhileLoop : public Ast {
  WhileLoop(Token*, Ast* cond, Compound* code);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~WhileLoop() {}
  std::string name();
  Ast* cond;
  Compound* code;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ForLoop : public Ast {
  ForLoop(Token*, Var* v, Ast* i, Ast* e, Compound*);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~ForLoop() {}
  std::string name();
  Var* var_node;
  Ast* init;
  Ast* term;
  Compound* code;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IfThenElse : public Ast {
  IfThenElse(Token*, Ast* cond, Compound* then, Compound* elze);
  IfThenElse(Token*, Ast* cond, Compound* then);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~IfThenElse() {}
  std::string name();
  Ast* cond;
  Compound* then;
  Compound* elze;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct RepeatUntil : public Ast {
  RepeatUntil(Token*, Ast* cond, Compound* code);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~RepeatUntil() {}
  std::string name();
  Ast* cond;
  Compound* code;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {
  Assignment(Var* left, Token* op, Ast* right);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Assignment() {}
  std::string name();
  Var* lhs;
  Ast* rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {
  d::DslValue eval(d::IEvaluator*) {
    return d::DslValue();
  }
  void visit(d::IAnalyzer*) {}
  std::string name() { return "709394"; }
  virtual ~NoOp() {}
  NoOp() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Param : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  Param(Var* v, Type* t);
  virtual ~Param() {}
  std::string name();
  Var* var_node;
  Type* type_node;
};

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  VarDecl(Var* v, Type* t);
  virtual ~VarDecl() {}
  std::string name();
  Var* var_node;
  Type* type_node;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {
  Block(std::vector<Ast*>& decls, Compound*);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Block() {}
  std::string name();
  Compound* compound;
  std::vector<Ast*> declarations;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {
  ProcedureDecl(const std::string&, std::vector<Param*>&, Block* block);
  d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureDecl() {}
  void visit(d::IAnalyzer*);
  std::string name();
  Block* block;
  std::string _name;
  std::vector<Param*> params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {
  ProcedureCall(const std::string&, std::vector<Ast*>&, Token*);
  d::DslValue eval(d::IEvaluator*);
  virtual ~ProcedureCall() {}
  void visit(d::IAnalyzer*);
  std::string name();
  std::string _name;
  std::vector<Ast*> args;
  d::FunctionSymbol* proc_symbol;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {
  Program(const std::string&, Block* block);
  d::DslValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  virtual ~Program() {}
  std::string name();
  Block* block;
  std::string _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BuiltinTypeSymbol : public d::TypeSymbol {
  BuiltinTypeSymbol(const std::string& n) : d::TypeSymbol(n) {}
  ~BuiltinTypeSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SymTable : public d::SymbolTable {
  SymTable(const std::string&, SymTable*);
  SymTable(const std::string&);
  ~SymTable() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct CrenshawParser : public d::IParser {
  CrenshawParser(const char* src);
  virtual ~CrenshawParser();
  d::IAst* parse();
  int cur();
  char peek();
  bool isCur(int);
  d::IToken* token();
  d::IToken* eat();
  d::IToken* eat(int wanted);
  Lexer* lex;
};




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF
