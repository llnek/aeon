#include <set>
#include "parser.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon::parser {
namespace lx = czlab::aeon::lexer;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound* compound_statement(lx::Context&);
Block* block(lx::Context&);
Ast* expr(lx::Context&);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast(lx::Token* t) : Ast() {
  token=t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast() : token(nullptr) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Ast::name() {
  return "Ast";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BinOp::BinOp(Ast* left, lx::Token* op, Ast* right)
  : Ast(op), left(left), right(right) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(Analyzer* a) {
  left->visit(a);
  right->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string BinOp::name() {
  return "BinOp";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue BinOp::eval(Evaluator* e) {
  DataValue lf, rt;
  double x, y, z;

  switch (this->token->type) {
    case lx::T_MINUS:
      lf= left->eval(e);
      rt= right->eval(e);
      if (lf.type == lx::T_INT)  {
        x= (double) lf.value.num ;
      } else { x= lf.value.real; }
      if (rt.type == lx::T_INT)  {
        y= (double) rt.value.num ;
      } else { y= rt.value.real; }
      z = x - y;
      if (lf.type == lx::T_REAL || rt.type== lx::T_REAL) {
        return DataValue(lx::T_REAL, z);
      } else {
        return DataValue(lx::T_INT,(long) z);
      }
      break;

    case lx::T_PLUS:
      lf= left->eval(e);
      rt= right->eval(e);
      if (lf.type == lx::T_INT)  {
        x= (double) lf.value.num ;
      } else { x= lf.value.real; }
      if (rt.type == lx::T_INT)  {
        y= (double) rt.value.num ;
      } else { y= rt.value.real; }
      z = x + y;
      if (lf.type == lx::T_REAL || rt.type== lx::T_REAL) {
        return DataValue(lx::T_REAL, z);
      } else {
        return DataValue(lx::T_INT,(long) z);
      }
      break;

    case lx::T_MULT:
      lf= left->eval(e);
      rt= right->eval(e);
      if (lf.type == lx::T_INT)  {
        x= (double) lf.value.num ;
      } else { x= lf.value.real; }
      if (rt.type == lx::T_INT)  {
        y= (double) rt.value.num ;
      } else { y= rt.value.real; }
      z = x * y;
      if (lf.type == lx::T_REAL || rt.type== lx::T_REAL) {
        return DataValue(lx::T_REAL, z);
      } else {
        return DataValue(lx::T_INT,(long) z);
      }
      break;

    case lx::T_DIV:
      lf= left->eval(e);
      rt= right->eval(e);
      if (lf.type == lx::T_INT)  {
        x= (double) lf.value.num ;
      } else { x= lf.value.real; }
      if (rt.type == lx::T_INT)  {
        y= (double) rt.value.num ;
      } else { y= rt.value.real; }
      z = x / y;
      return DataValue(lx::T_REAL, z);
      break;
    case lx::T_INT_DIV:
      lf= left->eval(e);
      rt= right->eval(e);
      if (lf.type == lx::T_INT)  {
        x= (double) lf.value.num ;
      } else { x= lf.value.real; }
      if (rt.type == lx::T_INT)  {
        y= (double) rt.value.num ;
      } else { y= rt.value.real; }
      z = x / y;
      return DataValue(lx::T_INT,(long) z);
      break;
  }
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(lx::Token* t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(Analyzer* a) {
}
std::string Num::name() {
  switch (token->type) {
    case lx::T_INTEGER:
      return "integer";
    case lx::T_FLOAT:
    return "float";
  }
  return "number?";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue Num::eval(Evaluator* e) {
  switch (token->type) {
    case lx::T_INTEGER:
      return DataValue(lx::T_INT, token->value.num);
    case lx::T_FLOAT:
    return DataValue(lx::T_REAL, token->value.real);
  }
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(lx::Token* t, Ast* expr) : Ast(t), expr(expr) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string UnaryOp::name() {
  switch (token->type) {
    case lx::T_PLUS: return "+";
    case lx::T_MINUS: return "-";
  }
  return "unary?";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(Analyzer* a) {
  expr->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue UnaryOp::eval(Evaluator* e) {
  switch (this->token->type) {
    case lx::T_PLUS:
      return expr->eval(e);
    case lx::T_MINUS:
      auto x= expr->eval(e);
      if (x.type == lx::T_INT)
        return DataValue(x.type, - x.value.num);
      if (x.type == lx::T_REAL)
        return DataValue(x.type, - x.value.real);
  }
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound() : Ast() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Compound::name() {
  return "Compound";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Compound::visit(Analyzer* a) {
  for (auto& it : children) {
    it->visit(a);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue Compound::eval(Evaluator* e) {
  for (auto& it : children) {
    it->eval(e);
  }
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment::Assignment(Var* left, lx::Token* op, Ast* right)
  : Ast(op), left(left), right(right) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Assignment::name() {
  return "=";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(Analyzer* a) {
  auto var_name = left->token->value.name;
  auto var_symbol = a->lookup(var_name);
  if (! var_symbol) {
    throw new std::runtime_error("bad symbol");
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue Assignment::eval(Evaluator* e) {
  auto var_name = left->token->value.name;
  ::printf("Assigning value to %s\n", var_name);
  e->setValue(var_name, right->eval(e));
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(lx::Token* t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Var::name() {
  return token->value.name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(Analyzer* a) {
  std::string name(token->value.name);
  auto s = a->lookup(name);
  if (! s)
    throw new std::runtime_error("Name error ");
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue Var::eval(Evaluator* e) {
  std::string name(token->value.name);
  return e->getValue(name);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type::Type(lx::Token* token) : Ast(token) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Type::name() {
  return token->value.name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Type::visit(Analyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue Type::eval(Evaluator* e) {
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::SymbolTable(const std::string& name,
                         int level, SymbolTable* enclosing_scope)
  : scope_name(name), scope_level(level), enclosing(enclosing_scope) {
  if (!enclosing) {
    init_builtins();
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void SymbolTable::init_builtins() {
  insert(new BuiltinTypeSymbol("INTEGER"));
  insert(new BuiltinTypeSymbol("REAL"));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void SymbolTable::insert(Symbol* s) {
  //log("Insert: {symbol.name}')
  if (s) symbols[s->name] = s;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Symbol* SymbolTable::lookup(
    const std::string& name, bool current_scope_only) {
  //log(f'Lookup: {name}. (Scope name: {self.scope_name})')
  auto s = symbols.find(name);
  if (s != symbols.end()) {
    return s->second;
  }
  if (current_scope_only)
    return nullptr;
  else
    return enclosing ? enclosing->lookup(name) : nullptr;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Param::Param(Var* var, Type* type)
  : Ast(), var_node(var), type_node(type) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Param::name() {
  return var_node->name();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Param::visit(Analyzer* a) {
  auto t= a->lookup(type_node->name());
  if (!t)
    throw new std::runtime_error("Param type is bad");
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue Param::eval(Evaluator* e) {
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VarDecl::VarDecl(Var* var, Type* type)
  : Ast(), var_node(var), type_node(type) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string VarDecl::name() {
  return var_node->name();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarDecl::visit(Analyzer* a) {
  auto type_name = type_node->name();
  auto var_name = var_node->name();
  auto type_symbol = a->lookup(type_name);
  if (!type_symbol) {
    throw new std::runtime_error("What Type?");
  }
  if (a->lookup(var_name,false)) {
    throw new std::runtime_error("Duplicate Var");
  } else {
    a->define(new VarSymbol(var_name, type_symbol));
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue VarDecl::eval(Evaluator* e) {
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block::Block(std::vector<Ast*> &decls, Compound* compound)
  : Ast(), compound_statement(compound) {
  declarations.insert(declarations.end(), decls.begin(), decls.end());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Block::name() {
  return "Block";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Block::visit(Analyzer* a) {
  for (auto& x : declarations) {
    x->visit(a);
  }
  compound_statement->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue Block::eval(Evaluator* e) {
  for (auto& x : declarations) {
    x->eval(e);
  }
  return compound_statement->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl::ProcedureDecl(const std::string& proc_name,
    std::vector<Param*>& pms, Block* block_node)
  : Ast()  {
  _name=proc_name;
  block=block_node;
  params.insert(params.end(), pms.begin(), pms.end());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureDecl::visit(Analyzer* a) {
  auto ps= new ProcedureSymbol(_name);
  a->define(ps);
  a->pushScope(_name);
  for (auto& p : params) {
    auto pt = a->lookup(p->type_node->name());
    auto pn = p->var_node->name();
    auto v = new VarSymbol(pn, pt);
    a->define(v);
    ps->params.push_back(v);
  }
  block->visit(a);
  a->popScope();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string ProcedureDecl::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue ProcedureDecl::eval(Evaluator* e) {
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall::ProcedureCall(const std::string& proc_name,
                             std::vector<Ast*>& p, lx::Token* token)
  : Ast(token), _name(proc_name) {
  params.insert(params.end(), p.begin(), p.end());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string ProcedureCall::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureCall::visit(Analyzer* a) {
  for (auto& p : params) {
    p->visit(a);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue ProcedureCall::eval(Evaluator* e) {
  return DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(const std::string& name, Block* block)
  : Ast() {
  _name=name;
  this->block=block;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Program::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(Analyzer* a) {
  block->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue Program::eval(Evaluator* e) {
  return block->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
lx::Token* error(lx::Context& ctx) {
  return nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
lx::Token* eat(int want, lx::Context& ctx) {
  auto t= ctx.cur;
  if (t->type == want) {
    next_token(ctx);
    return t;
  } else {
    return error(ctx);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var* variable(lx::Context& ctx) {
  auto node = new Var(ctx.cur);
  eat(lx::T_ID,ctx);
  return node;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* factor(lx::Context& ctx) {
  auto t= ctx.cur;
  Ast* res=nullptr;
  switch (t->type) {
    case lx::T_PLUS:
      eat(lx::T_PLUS,ctx);
      res = new UnaryOp(t, factor(ctx));
      break;
    case lx::T_MINUS:
      eat(lx::T_MINUS,ctx);
      res = new UnaryOp(t, factor(ctx));
      break;
    case lx::T_INTEGER:
      eat(lx::T_INTEGER,ctx);
      res= new Num(t);
      break;
    case lx::T_FLOAT:
      eat(lx::T_FLOAT,ctx);
      res= new Num(t);
      break;
    case lx::T_LPAREN:
      eat(lx::T_LPAREN,ctx);
      res= expr(ctx);
      eat(lx::T_RPAREN,ctx);
      break;
    default:
      res= variable(ctx);
      break;
  }
  return res;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* term(lx::Context& ctx) {
  static std::set<int> ops {lx::T_MULT,lx::T_DIV, lx::T_INT_DIV};
  auto res= factor(ctx);
  lx::Token* op;
  while (ops.find(ctx.cur->type) != ops.end()) {
    op= ctx.cur;
    switch (op->type) {
      case lx::T_MULT:
        eat(lx::T_MULT,ctx);
      break;
      case lx::T_DIV:
        eat(lx::T_DIV,ctx);
      break;
      case lx::T_INT_DIV:
        eat(lx::T_INT_DIV,ctx);
      break;
    }
    res = new BinOp(res, op, factor(ctx));
  }
  return res;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* expr(lx::Context& ctx) {
  static std::set<int> ops {lx::T_PLUS, lx::T_MINUS};
  Ast* res= term(ctx);
  lx::Token* op;
  while (ops.find(ctx.cur->type) != ops.end()) {
    op= ctx.cur;
    switch (op->type) {
      case lx::T_PLUS:
        eat(lx::T_PLUS,ctx);
      break;
      case lx::T_MINUS:
        eat(lx::T_MINUS,ctx);
      break;
    }
    res= new BinOp(res, op, term(ctx));
  }

  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type* type_spec(lx::Context& ctx) {
  auto token = ctx.cur;
  if (token->type == lx::T_INT)
    eat(lx::T_INT,ctx);
  else
    eat(lx::T_REAL,ctx);
  return new Type(token);
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<VarDecl*> variable_declaration(lx::Context& ctx) {
  std::vector<Var*> vars { new Var(ctx.cur) };
  eat(lx::T_ID,ctx);
  while (ctx.cur->type == lx::T_COMMA) {
    eat(lx::T_COMMA,ctx);
    vars.push_back(new Var(ctx.cur));
    eat(lx::T_ID,ctx);
  }
  eat(lx::T_COLON,ctx);

  auto type = type_spec(ctx);
  std::vector<VarDecl*> out;
  for (auto &x : vars) {
    out.push_back(new VarDecl(x, type));
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment* assignment_statement(lx::Context& ctx) {
  auto left = variable(ctx);
  auto t = ctx.cur;
  eat(lx::T_ASSIGN,ctx);
  auto right = expr(ctx);
  return new Assignment(left, t, right);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NoOp* empty(lx::Context& ctx) {
  return new NoOp();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall* proccall_statement(lx::Context& ctx) {
  auto token = ctx.cur;
  std::string proc_name = token->value.name;

  eat(lx::T_ID,ctx);
  eat(lx::T_LPAREN,ctx);

  std::vector<Ast*> pms;

  if (ctx.cur->type != lx::T_RPAREN) {
    pms.push_back( expr(ctx));
  }

  while (ctx.cur->type == lx::T_COMMA) {
    eat(lx::T_COMMA,ctx);
    pms.push_back(expr(ctx));
  }

  eat(lx::T_RPAREN,ctx);

  return new ProcedureCall(proc_name, pms, token);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* statement(lx::Context& ctx) {
  Ast* node;
  switch (ctx.cur->type) {
    case lx::T_BEGIN:
      node = compound_statement(ctx);
      break;
    case lx::T_ID:
      if (lx::get_ch(ctx) == '(') {
        node = proccall_statement(ctx);
      } else {
        node = assignment_statement(ctx);
      }
      break;
    default:
      node = empty(ctx);
      break;
  }
  return node;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Ast*> statement_list(lx::Context& ctx) {

  std::vector<Ast*> results {
    statement(ctx)
  };

  while (ctx.cur->type == lx::T_SEMI) {
    eat(lx::T_SEMI, ctx);
    results.push_back(statement(ctx));
  }

  if (ctx.cur->type == lx::T_ID) {
    error(ctx);
  }

  return results;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound* compound_statement(lx::Context& ctx) {
  eat(lx::T_BEGIN,ctx);
  auto nodes = statement_list(ctx);
  eat(lx::T_END, ctx);
  auto root= new Compound();
  for (auto& node : nodes) {
    root->children.push_back(node);
  }
  return root;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Param*> formal_parameters(lx::Context& ctx) {
  std::vector<Param*> pnodes;
  std::vector<lx::Token*> param_tokens {ctx.cur};
  eat(lx::T_ID,ctx);
  while (ctx.cur->type == lx::T_COMMA) {
    eat(lx::T_COMMA, ctx);
    param_tokens.push_back(ctx.cur);
    eat(lx::T_ID, ctx);
  }
  eat(lx::T_COLON, ctx);
  auto type_node = type_spec(ctx);
  for (auto& t : param_tokens) {
    pnodes.push_back(new Param(new Var(t), type_node));
  }
  return pnodes;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Param*> formal_parameter_list(lx::Context& ctx) {
  std::vector<Param*> out;

  if (ctx.cur->type != lx::T_ID) {
    return out;
  }

  auto pnodes = formal_parameters(ctx);
  while (ctx.cur->type == lx::T_SEMI) {
    eat(lx::T_SEMI, ctx);
    auto pms = formal_parameters(ctx);
    pnodes.insert(pnodes.end(), pms.begin(), pms.end());
  }

  return pnodes;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl* procedure_declaration(lx::Context& ctx) {
  eat(lx::T_PROCEDURE,ctx);
  auto proc_name = ctx.cur->value.name;
  std::vector<Param*> params;

  eat(lx::T_ID,ctx);
  if (ctx.cur->type == lx::T_LPAREN) {
    eat(lx::T_LPAREN,ctx);
    params = formal_parameter_list(ctx);
    eat(lx::T_RPAREN,ctx);
  }

  eat(lx::T_SEMI,ctx);
  auto block_node = block(ctx);
  auto proc_decl = new ProcedureDecl(proc_name, params, block_node);
  eat(lx::T_SEMI,ctx);
  return proc_decl;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Ast*> declarations(lx::Context& ctx) {
  std::vector<Ast*> ds;
  if (ctx.cur->type == lx::T_VAR) {
    eat(lx::T_VAR, ctx);
    while (ctx.cur->type == lx::T_ID) {
      auto vs = variable_declaration(ctx);
      ds.insert(ds.end(),vs.begin(),vs.end());
      eat(lx::T_SEMI,ctx);
    }
  }

  while (ctx.cur->type == lx::T_PROCEDURE) {
    ds.push_back(procedure_declaration(ctx));
  }

  return ds;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block* block(lx::Context& ctx) {
  auto decls = declarations(ctx);
  auto cs = compound_statement(ctx);
  return new Block(decls, cs);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program* program(lx::Context& ctx) {
  eat(lx::T_PROGRAM,ctx);
  auto var_node = variable(ctx);
  auto prog_name = var_node->name();
  eat(lx::T_SEMI,ctx);
  auto block_node = block(ctx);
  auto program_node = new Program(prog_name, block_node);
  eat(lx::T_DOT,ctx);
  return program_node;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const char* name, int type, int nesting_level)
  : name(name), type(type), level(nesting_level) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DataValue Frame::get(const std::string& key) {
  auto x= slots.find(key);
  return x != slots.end() ? x->second : DataValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Frame::set(const std::string& key, const DataValue& v) {
  slots[key]=v;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CallStack::CallStack() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void CallStack::push(Frame* ar) {
  frames.push(ar);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::pop() {
  auto x= peek();
  if (x) frames.pop();
  return x;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::peek() {
  return frames.empty() ? nullptr : frames.top();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* parser(lx::Context& ctx) {
  return program(ctx);
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


