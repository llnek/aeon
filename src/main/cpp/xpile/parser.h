#pragma once
#include <string>
#include <vector>
#include <map>
#include <stack>
#include "lexer.h"
namespace czlab::aeon::parser {
namespace lx= czlab::aeon::lexer;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum DataValueType {
  INT_VALUE, FLOAT_VALUE, NULL_VALUE
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct DataValue {
  DataValue(int t, long v) : type(t) {
    value.num=v;
  }
  DataValue(int t, double v) : type(t) {
    value.real=v;
  }
  DataValue() {
    type=NULL_VALUE;
  }
  bool isNull() {
    return type == NULL_VALUE;
  }
  int type;
  union {
    long num;
    double real;
  } value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Frame {
  Frame(const char* name, int type, int nesting_level);
  DataValue get(const std::string&);
  void set(const std::string&, const DataValue&);
  std::string name;
  int type;
  int level;
  std::map<std::string,DataValue> slots;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct CallStack {
  CallStack();
  void push(Frame*);
  Frame* pop();
  Frame* peek();
  std::stack<Frame*> frames;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Symbol {
  Symbol(const std::string& n, Symbol* t) : Symbol(n) { type=t; }
  Symbol(const std::string& n) : name(n) { type=nullptr; }
  ~Symbol() {}
  Symbol* type;
  std::string name;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct TypeSymbol : public Symbol {
  TypeSymbol(const std::string& n) : Symbol(n) {}
  ~TypeSymbol() {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SymbolTable {
  SymbolTable(const std::string& scope_name, int scope_level,
      SymbolTable* enclosing_scope= nullptr);
  void insert(Symbol*);
  Symbol* lookup(const std::string& name, bool current_scope_only=false);
  std::string scope_name;
  int scope_level;
  SymbolTable* enclosing;
  std::map<std::string, Symbol*> symbols;
private:
  void init_builtins();
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Evaluator {
  Evaluator() {}
  virtual ~Evaluator() {}
  virtual void setValue(const std::string&, const DataValue&) = 0;
  virtual DataValue getValue(const std::string&) = 0;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Analyzer {
  Analyzer() {}
  virtual ~Analyzer() {}
  virtual Symbol* lookup(const std::string& , bool traverse=true) = 0;
  virtual void pushScope(const std::string&) =0;
  virtual SymbolTable* popScope()=0;
  virtual void define(Symbol*)=0;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Ast {
  Ast(lx::Token* t);
  Ast();
  virtual ~Ast() {}
  virtual DataValue eval(Evaluator*) = 0;
  virtual void visit(Analyzer*) = 0;
  lx::Token* token;
  std::string _name;
  virtual std::string name();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BinOp : public Ast {
  BinOp(Ast* left, lx::Token* op, Ast* right);
  ~BinOp() {}
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
  Ast* left;
  Ast* right;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Num : public Ast {
  Num(lx::Token* t);
  ~Num() {}
  void visit(Analyzer*);
  std::string name();
  DataValue eval(Evaluator*);
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct UnaryOp : public Ast {
  UnaryOp(lx::Token* t, Ast* expr);
  ~UnaryOp() {}
  void visit(Analyzer*);
  std::string name();
  DataValue eval(Evaluator*);
  Ast* expr;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Compound : public Ast {
  Compound();
  ~Compound() {}
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
  std::vector<Ast*> children;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Var : public Ast {
  Var(lx::Token* t);
  ~Var() {}
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Type : public Ast {
  Type(lx::Token* token);
  ~Type() {}
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Assignment : public Ast {
  Assignment(Var* left, lx::Token* op, Ast* right);
  ~Assignment() {}
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
  Var* left;
  Ast* right;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoOp : public Ast {
  NoOp() {}
  ~NoOp() {}
  DataValue eval(Evaluator*) {
    return DataValue();
  }
  std::string name() { return ""; }
  void visit(Analyzer*) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Param : public Ast {
  Param(Var* var_node, Type* type_node);
  ~Param() {}
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
  Var* var_node;
  Type* type_node;
};
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarDecl : public Ast {
  VarDecl(Var* var_node, Type* type_node);
  ~VarDecl() {}
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
  Var* var_node;
  Type* type_node;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Block : public Ast {
  Block(std::vector<Ast*>& decls, Compound* compound_statement);
  ~Block() {}
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
  Compound* compound_statement;
  std::vector<Ast*> declarations;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureDecl : public Ast {
  ProcedureDecl(const std::string& proc_name,
                std::vector<Param*>& p, Block* block_node);
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
  Block* block;
  std::string _name;
  std::vector<Param*> params;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureCall : public Ast {
  ProcedureCall(const std::string& proc_name,
                std::vector<Ast*>& p, lx::Token* token);
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
  std::string _name;
  std::vector<Ast*> params;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Program : public Ast {
  Program(const std::string& name, Block* block);
  ~Program() {}
  DataValue eval(Evaluator*);
  void visit(Analyzer*);
  std::string name();
  Block* block;
  std::string _name;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BuiltinTypeSymbol : public TypeSymbol {
  BuiltinTypeSymbol(const std::string& n) : TypeSymbol(n) {}
  ~BuiltinTypeSymbol() {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarSymbol : public Symbol {
  VarSymbol(const std::string& n, Symbol* t) : Symbol(n,t) {}
  ~VarSymbol() {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ProcedureSymbol : public Symbol {
  ProcedureSymbol(const std::string& name) : Symbol(name) {
  }
  std::vector<VarSymbol*> params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* parser(lx::Context& ctx);


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

