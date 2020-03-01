#pragma once

#include "lexer.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {
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
struct Type : public Ast {
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  Var(const char*);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Reader : public Ast {

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Writer : public Ast {

};

struct Factor : public Ast {

};

struct Term : public Ast {
  Term(Ast*, std::vector<Ast*>&);
  Term(Ast*);
  Factor* f;
  std::vector<Ast>* nodes;
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
};

struct Expr : public Ast {
  Expr(Ast*, std::vector<Ast*>&);
  Expr(Ast*);
  Expr(std::vector<Ast*>&);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::vector<Ast>* nodes;
};

struct NotFactor : public Ast {
  NotFactor(bool, Ast*);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

  bool _not ;
  Ast* factor;
};
struct BoolTerm : public Ast {
  BoolTerm(Ast*, std::vector<Ast*>&);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

  Ast* factor;
  std::vector<Ast*> nodes;
};
struct BoolExpr : public Ast {
  BoolExpr(BoolTerm*, std::vector<Ast*>&);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

  BoolTerm* term;
  std::vector<Ast*> nodes;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Relation : public Ast {
  Relation(Ast* lhs, d::IToken*, Ast*);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

  Ast* lhs;
  Ast* rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  Num(d::IToken*);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {
  Ast* expr;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Repeat : public Ast {
  Repeat(Ast*, BoolExpr* cond);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

  BoolExpr* cond;
  Ast* body;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct For : public Ast {
  For(Var*, Expr*, Expr*, Ast*);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

  Var* var;
  Ast* body;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct While : public Ast {
  While(BoolExpr* cond, Ast*);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

  BoolExpr* cond;
  Ast* body;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct If : public Ast {
  If(BoolExpr* cond, Ast* t, Ast* e);
  If(BoolExpr* cond, Ast* t);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

  BoolExpr* cond;
  Ast* _then;
  Ast* _else;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {
  Assignment(Var*, Ast*);
  std::string name();
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);

  Var* lhs;
  Ast* rhs;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {
  std::vector<Ast*> children;
};

struct Param : public Ast {

};

struct Block : public Ast {

};

struct ProcCall : public Ast {
  ProcCall(Var*, std::vector<Expr*>&);
  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcDecl : public Ast {
  Block* block;
  std::vector<Param*> params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {
  Var* var;
  Type* type;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {
  Program(const char* name,
          std::vector<VarDecl*>& gvars,
          std::vector<ProcDecl*>& procs, Block*);
  virtual ~Program() {}

  d::ExprValue eval(d::IEvaluator*);
  void visit(d::IAnalyzer*);
  std::string name();

  Block* block;
  std::vector<VarDecl*> gvars;
  std::vector<ProcDecl*> procs;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct CrenshawParser : public d::IParser {

  CrenshawParser(const char*);
  virtual ~CrenshawParser();

  d::IToken* eat(int wanted);
  d::IToken* eat();
  d::IAst* parse();

  bool isCur(int token_type);
  int cur();
  int peek();
  d::IToken* token();

  Lexer* lex;
};






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

