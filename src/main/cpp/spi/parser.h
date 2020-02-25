#pragma once
#include "lexer.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::spi {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Ast : public d::IAst {
  virtual ~Ast() {}
  Ast(d::IToken* t);
  Ast();
  d::IToken* token;
  std::string _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {
  BinOp(Ast* left, d::IToken* op, Ast* right);
  virtual ~BinOp() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  Ast* lhs;
  Ast* rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  Num(d::IToken* t);
  virtual ~Num() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Ast {
  String(d::IToken* t);
  virtual ~String() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {
  UnaryOp(d::IToken* t, Ast* expr);
  virtual ~UnaryOp() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  Ast* expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {
  Compound();
  virtual ~Compound() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  std::vector<Ast*> children;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  Var(d::IToken* t);
  virtual ~Var() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {
  Type(d::IToken* token);
  virtual ~Type() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {
  Assignment(Var* left, d::IToken* op, Ast* right);
  virtual ~Assignment() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  Var* lhs;
  Ast* rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {
  NoOp() {}
  virtual ~NoOp() {}
  d::ExprValue eval(d::IEvaluator*) {
    return d::ExprValue();
  }
  void visit(d::IAnalyzer*) {}
  std::string name() { return ""; }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Param : public Ast {
  Param(Var* v, Type* t);
  virtual ~Param() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  Var* var_node;
  Type* type_node;
};

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {
  VarDecl(Var* v, Type* t);
  virtual ~VarDecl() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  Var* var_node;
  Type* type_node;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {
  Block(std::vector<Ast*>& decls, Compound*);
  virtual ~Block() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  Compound* compound_statement;
  std::vector<Ast*> declarations;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {
  ProcedureDecl(const char* proc, std::vector<Param*>& p, Block* block);
  virtual ~ProcedureDecl() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  Block* block;
  std::string _name;
  std::vector<Param*> params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {
  ProcedureCall(const char* proc,
                std::vector<Ast*>& p, d::IToken* token);
  virtual ~ProcedureCall() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  std::string _name;
  std::vector<Ast*> params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {
  Program(const char* name, Block* block);
  virtual ~Program() {}
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
  Block* block;
  std::string _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BuiltinTypeSymbol : public d::TypeSymbol {
  BuiltinTypeSymbol(const char* n) : d::TypeSymbol(n) {}
  ~BuiltinTypeSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SymTable : public d::SymbolTable {
  SymTable(SymTable*);
  SymTable();
  ~SymTable() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SimplePascalParser : public d::IParser {
  virtual ~SimplePascalParser();
  SimplePascalParser(const char* src);
  d::IAst* parse();
  d::IToken* eat(int wanted);
  Lexer* lex;
}


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

