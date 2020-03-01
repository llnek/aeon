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

#include "parser.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::spi {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound* compound_statement(SimplePascalParser*);
Block* block(SimplePascalParser*);
Ast* expr(SimplePascalParser*);
static char BUF[1024];


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast(Token* t) : Ast() {
  token=t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast() : token(nullptr) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BinOp::BinOp(Ast* left, Token* op, Ast* right)
  : Ast(op), lhs(left), rhs(right) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(d::IAnalyzer* a) {
  lhs->visit(a);
  rhs->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string BinOp::name() {
  return "BinOp";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue BinOp::eval(d::IEvaluator* e) {

  auto lf = lhs->eval(e);
  auto rt = rhs->eval(e);
  double x, y, z;

  if (lf.type == d::EXPR_INT)  {
    x= (double) lf.value.u.n;
  } else { x= lf.value.u.r; }

  if (rt.type == d::EXPR_INT)  {
    y= (double) rt.value.u.n;
  } else { y= rt.value.u.r; }

  switch (token->type()) {
    case d::T_MINUS: z = x - y; break;
    case d::T_PLUS: z = x + y; break;
    case d::T_MULT: z = x * y; break;
    case T_INT_DIV:
    case d::T_DIV: z = x / y; break;
    default:
      ::sprintf(BUF, "Bad binary-op near line %d, col %d.\n", token->impl.line, token->impl.col);
      throw d::SyntaxError(BUF);
  }

  switch (token->type()) {
    case T_INT_DIV:
      return d::ExprValue(d::EXPR_INT, (long) z);
    case d::T_DIV:
      return d::ExprValue(d::EXPR_REAL, z);
    default:
      return (lf.type == d::EXPR_REAL ||
              rt.type == d::EXPR_REAL)
        ? d::ExprValue(d::EXPR_REAL, z)
        : d::ExprValue(d::EXPR_INT, (long) z);
  }

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String::String(Token* t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void String::visit(d::IAnalyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string String::name() {
  return "string";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue String::eval(d::IEvaluator* e) {
  return d::ExprValue(d::EXPR_STR, token->getLiteralAsStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(Token* t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(d::IAnalyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Num::name() {
  switch (token->type()) {
    case d::T_INTEGER: return "integer";
    case d::T_REAL: return "real";
    default:
      ::sprintf(BUF,"Bad number near line %d, col %d.\n", token->impl.line, token->impl.col);
      throw d::SyntaxError(BUF);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Num::eval(d::IEvaluator* e) {
  switch (token->type()) {
    case d::T_INTEGER:
      return d::ExprValue(d::EXPR_INT, token->getLiteralAsInt());
    case d::T_REAL:
      return d::ExprValue(d::EXPR_REAL, token->getLiteralAsReal());
    default:
      ::sprintf(BUF,"Bad number near line %d, col %d.\n", token->impl.line, token->impl.col);
      throw d::SyntaxError(BUF);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(Token* t, Ast* expr) : Ast(t), expr(expr) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string UnaryOp::name() {
  switch (token->type()) {
    case d::T_PLUS: return "+";
    case d::T_MINUS: return "-";
    default:
      ::sprintf(BUF,"Bad unary-op near line %d, col %d.\n", token->impl.line, token->impl.col);
      throw d::SyntaxError(BUF);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(d::IAnalyzer* a) {
  expr->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue UnaryOp::eval(d::IEvaluator* e) {
  auto r = expr->eval(e);
  if (! (r.type == d::EXPR_REAL || r.type == d::EXPR_INT)) {
    ::sprintf(BUF,"Expected numeric type near line %d, col %d.\n", token->impl.line, token->impl.col);
    throw d::SyntaxError(BUF);
  }
  if (token->type() == d::T_MINUS) {
    if (r.type == d::EXPR_INT)
      return d::ExprValue(r.type, - r.value.u.n);
    if (r.type == d::EXPR_REAL)
      return d::ExprValue(r.type, - r.value.u.r);
  }
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound() : Ast() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Compound::name() {
  return "Compound";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Compound::visit(d::IAnalyzer* a) {
  for (auto& it : statements) {
    it->visit(a);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Compound::eval(d::IEvaluator* e) {
  d::ExprValue ret;
  for (auto& it : statements) {
    ret=it->eval(e);
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment::Assignment(Var* left, Token* op, Ast* right)
  : Ast(op), lhs(left), rhs(right) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Assignment::name() {
  return ":=";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a) {
  auto t = lhs->token;
  auto v = t->getLiteralAsStr();
  if (! a->lookup(v)) {
    ::sprintf(BUF,
              "Unknown var %s near line %d col %d.\n",
              v.c_str(), t->impl.line, t->impl.col);
    throw d::SyntaxError(BUF);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Assignment::eval(d::IEvaluator* e) {
  auto v = lhs->token->getLiteralAsStr();
  auto r= rhs->eval(e);
  //::printf("Assigning value to %s\n", v.c_str());
  e->setValue(v, r, false);
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(Token* t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Var::name() {
  return token->getLiteralAsStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(d::IAnalyzer* a) {
  auto n = token->getLiteralAsStr();
  if (! a->lookup(n)) {
    ::sprintf(BUF,
              "Unknown var %s near line %d col %d.\n",
              n.c_str(), token->impl.line, token->impl.col);
    throw d::SyntaxError(BUF);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Var::eval(d::IEvaluator* e) {
  auto n = token->getLiteralAsStr();
  return e->getValue(n);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type::Type(Token* token) : Ast(token) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Type::name() {
  return token->getLiteralAsStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Type::visit(d::IAnalyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Type::eval(d::IEvaluator* e) {
  return d::ExprValue();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymTable::SymTable(const std::string& n, SymTable* outer) : SymTable(n) {
  enclosing=outer;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymTable::SymTable(const std::string& n) : d::SymbolTable(n) {
  insert(new BuiltinTypeSymbol("INTEGER"));
  insert(new BuiltinTypeSymbol("REAL"));
  insert(new BuiltinTypeSymbol("STRING"));
  ::printf("Added built-in types.\n");
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
void Param::visit(d::IAnalyzer* a) {
  auto n= type_node->name();
  auto t= a->lookup(n);
  if (!t) {
    ::sprintf(BUF,
              "Unknown type %s near line %d col %d.\n",
              n.c_str(), token->impl.line, token->impl.col);
    throw d::SyntaxError("Unknown param type.");
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Param::eval(d::IEvaluator* e) {
  return d::ExprValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VarDecl::VarDecl(Var* var, Type* type)
  : Ast(var->token), var_node(var), type_node(type) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string VarDecl::name() {
  return var_node->name();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarDecl::visit(d::IAnalyzer* a) {
  auto type_name = type_node->name();
  auto var_name = var_node->name();
  auto type_symbol = a->lookup(type_name);
  if (!type_symbol) {
    ::sprintf(BUF,
              "Unknown type %s near line %d col %d.\n",
              type_name.c_str(), token->impl.line, token->impl.col);
    throw d::SyntaxError(BUF);
  }
  if (a->lookup(var_name.c_str(), false)) {
    ::sprintf(BUF,
              "Duplicate var %s near line %d col %d.\n",
              var_name.c_str(), token->impl.line, token->impl.col);
    throw d::SyntaxError(BUF);
  } else {
    a->define(new d::VarSymbol(var_name, type_symbol));
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue VarDecl::eval(d::IEvaluator* e) {
  d::ExprValue v;
  e->setValue(this->name(), v);
  return v;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block::Block(std::vector<Ast*>& decls, Compound* compound)
  : Ast(), compound(compound) {
  s__ccat(declarations, decls);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Block::name() {
  return "Block";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Block::visit(d::IAnalyzer* a) {
  for (auto& x : declarations) {
    x->visit(a);
  }
  compound->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Block::eval(d::IEvaluator* e) {
  for (auto& x : declarations) {
    x->eval(e);
  }
  return compound->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl::ProcedureDecl(const std::string& proc_name,
    std::vector<Param*>& pms, Block* block_node)
  : Ast()  {
  _name=proc_name;
  block=block_node;
  s__ccat(params, pms);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureDecl::visit(d::IAnalyzer* a) {
  auto fs= new d::FunctionSymbol( name());
  a->define(fs);
  a->pushScope(fs->name);

  for (auto& p : params) {
    auto pt = a->lookup(p->type_node->name());
    auto pn = p->var_node->name();
    auto v = new d::ParamSymbol(pn, pt);
    a->define(v);
    s__conj(fs->params,v);
  }
  block->visit(a);
  a->popScope();
  fs->block = this->block;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string ProcedureDecl::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue ProcedureDecl::eval(d::IEvaluator* e) {
  return d::ExprValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall::ProcedureCall(const std::string& proc_name,
                             std::vector<Ast*>& p, Token* token)
  : Ast(token), _name(proc_name) {
  S_NIL(proc_symbol);
  s__ccat(args, p);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string ProcedureCall::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureCall::visit(d::IAnalyzer* a) {
  for (auto& p : args) {
    p->visit(a);
  }
  //get the corresponding symbol
  auto x = a->lookup(_name);
  if (x) {
    proc_symbol = (d::FunctionSymbol*) x;
  } else {
    ::sprintf(BUF,
              "Unknown proc %s near line %d col %d.\n",
              _name.c_str(), token->impl.line, token->impl.col);
    throw d::SyntaxError(BUF);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue ProcedureCall::eval(d::IEvaluator* e) {

  assert(proc_symbol->params.size() == args.size());
  e->push(_name);

  for (auto i=0; i < args.size(); ++i) {
    auto& p= proc_symbol->params[i];
    auto v= args[i]->eval(e);
    e->setValue(p->name, v);
  }

  auto r= proc_symbol->block->eval(e);
  e->pop();
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(const std::string& name, Block* block) : Ast(), _name(name) {
  this->block=block;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Program::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(d::IAnalyzer* a) {
  block->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Program::eval(d::IEvaluator* e) {
  return block->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var* variable(SimplePascalParser* ps) {
  auto node = new Var((Token*) ps->token());
  ps->eat(d::T_IDENT);
  return node;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* factor(SimplePascalParser* ps) {
  auto t= (Token*) ps->token();
  Ast* res=nullptr;
  switch (t->type()) {
    case d::T_PLUS:
      res= (ps->eat(), new UnaryOp(t, factor(ps)));
      break;
    case d::T_MINUS:
      res= (ps->eat(), new UnaryOp(t, factor(ps)));
      break;
    case d::T_INTEGER:
      res= (ps->eat(), new Num(t));
      break;
    case d::T_REAL:
      res= (ps->eat(), new Num(t));
      break;
    case d::T_STRING:
      res= (ps->eat(), new String(t));
      break;
    case d::T_LPAREN:
      res= (ps->eat(), expr(ps));
      ps->eat(d::T_RPAREN);
      break;
    default:
      res= variable(ps);
      break;
  }
  return res;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* term(SimplePascalParser* ps) {
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV};
  auto res= factor(ps);
  while (contains(ops,ps->cur())) {
    res = new BinOp(res, (Token*)ps->eat(), factor(ps));
  }
  return res;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* expr(SimplePascalParser* ps) {
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  Ast* res= term(ps);
  while (contains(ops,ps->cur())) {
    res= new BinOp(res, (Token*)ps->eat(), term(ps));
  }
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type* type_spec(SimplePascalParser* ps) {
  auto t = (Token*)ps->token();
  switch (t->type()) {
    case T_STR:
    case T_INT:
    case T_REAL: ps->eat(); break;
    default:
      ::sprintf(BUF,"Unknown token %d near line %d, col %d.\n",
          t->type(), t->impl.line, t->impl.col);
      throw d::SyntaxError(BUF);
  }
  return new Type(t);
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<VarDecl*> variable_declaration(SimplePascalParser* ps) {
  std::vector<Var*> vars { new Var((Token*) ps->eat(d::T_IDENT)) };
  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(vars, new Var((Token*) ps->eat(d::T_IDENT)));
  }
  ps->eat(d::T_COLON);
  auto type = type_spec(ps);
  std::vector<VarDecl*> out;
  for (auto &x : vars) {
    s__conj(out, new VarDecl(x, type));
  }

  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment* assignment_statement(SimplePascalParser* ps) {
  auto left = variable(ps);
  auto t= ps->eat(T_ASSIGN);
  auto right = expr(ps);
  return new Assignment(left, (Token*)t, right);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NoOp* empty(SimplePascalParser* ps) {
  return new NoOp();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall* proccall_statement(SimplePascalParser* ps) {
  auto token = ps->token();
  auto proc_name = token->getLiteralAsStr();
  std::vector<Ast*> pms;

  ps->eat(d::T_IDENT);
  ps->eat(d::T_LPAREN);

  if (!ps->isCur(d::T_RPAREN)) {
    s__conj(pms, expr(ps));
  }

  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(pms,expr(ps));
  }

  return (ps->eat(d::T_RPAREN), new ProcedureCall(proc_name, pms, (Token*)token));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* statement(SimplePascalParser* ps) {
  Ast* node;
  switch (ps->cur()) {
    case T_BEGIN:
      node = compound_statement(ps);
      break;
    case d::T_IDENT:
      if (ps->peek() == '(') {
        node = proccall_statement(ps);
      } else {
        node = assignment_statement(ps);
      }
      break;
    default:
      /*
      auto z=(Token*) ps->token();
      ::sprintf(BUF,"Unknown token %s near line %d, col %d.\n",
          z->toString().c_str(), z->impl.line, z->impl.col);
      throw d::SyntaxError(BUF);
      */
      node = empty(ps);
      break;
  }
  return node;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Ast*> statement_list(SimplePascalParser* ps) {

  std::vector<Ast*> results;
  auto s= statement(ps);

  if (!(s->name() == "709394")) {
    s__conj(results,s);
  }

  while (ps->isCur(d::T_SEMI)) {
    ps->eat();
    s= statement(ps);
    if (!(s->name() == "709394")) {
      s__conj(results,s);
    }
  }

  if (ps->isCur(d::T_IDENT)) {
    ::sprintf(BUF, "Unexpected token `%s`", ps->token()->getLiteralAsStr().c_str());
    throw d::SyntaxError(BUF);
  }

  return results;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound* compound_statement(SimplePascalParser* ps) {
  auto nodes = (ps->eat(T_BEGIN),statement_list(ps));
  auto root= (ps->eat(T_END), new Compound());
  for (auto& node : nodes) {
    s__conj(root->statements,node);
  }
  return root;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Param*> formal_parameters(SimplePascalParser* ps) {

  std::vector<Token*> param_tokens { (Token*)ps->eat(d::T_IDENT)};
  std::vector<Param*> pnodes;

  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(param_tokens, (Token*)ps->eat(d::T_IDENT));
  }

  auto type_node = (ps->eat(d::T_COLON),type_spec(ps));

  for (auto& t : param_tokens) {
    //::printf("param toke= %s\n", t->getLiteralAsStr());
    s__conj(pnodes,new Param(new Var(t), type_node));
  }

  return pnodes;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Param*> formal_parameter_list(SimplePascalParser* ps) {
  std::vector<Param*> out;

  if (!ps->isCur(d::T_IDENT)) {
    return out;
  }

  auto pnodes = formal_parameters(ps);
  while (ps->isCur(d::T_SEMI)) {
    auto pms = (ps->eat(), formal_parameters(ps));
    s__ccat(pnodes, pms);
  }

  return pnodes;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl* procedure_declaration(SimplePascalParser* ps) {

  auto proc_name = (ps->eat(T_PROCEDURE), ps->eat(d::T_IDENT));
  std::vector<Param*> params;

  if (ps->isCur(d::T_LPAREN)) {
    params = (ps->eat(), formal_parameter_list(ps));
    ps->eat(d::T_RPAREN);
  }

  auto decl = new ProcedureDecl(proc_name->getLiteralAsStr(),
                                params,
                                (ps->eat(d::T_SEMI), block(ps)));
  //::printf("proc name=%s\n", decl->name().c_str());
  return (ps->eat(d::T_SEMI), decl);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Ast*> declarations(SimplePascalParser* ps) {
  std::vector<Ast*> ds;

  if (ps->isCur(T_VAR)) {
    ps->eat();
    while (ps->isCur(d::T_IDENT)) {
      auto vs = variable_declaration(ps);
      s__ccat(ds,vs);
      ps->eat(d::T_SEMI);
    }
  }

  while (ps->isCur(T_PROCEDURE)) {
    s__conj(ds,procedure_declaration(ps));
  }

  return ds;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block* block(SimplePascalParser* ps) {
  auto decls=declarations(ps);
  return new Block(decls, compound_statement(ps));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program* program(SimplePascalParser* ps) {
  auto var_node = (ps->eat(T_PROGRAM),variable(ps));
  auto prog_name = var_node->name();
  auto prog = new Program(prog_name.c_str(),
                          (ps->eat(d::T_SEMI),block(ps)));
  //::printf("program = %s\n", prog->name().c_str());
  return (ps->eat(d::T_DOT), prog);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SimplePascalParser::~SimplePascalParser() {
  delete lex;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SimplePascalParser::SimplePascalParser(const char* src) {
  lex = new Lexer(src);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IAst* SimplePascalParser::parse() {
  return program(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SimplePascalParser::cur() {
  return lex->ctx.cur->type();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char SimplePascalParser::peek() {
  return d::peek(lex->ctx);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SimplePascalParser::isCur(int type) {
  return lex->ctx.cur->type() == type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* SimplePascalParser::token() {
  return lex->ctx.cur;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* SimplePascalParser::eat() {
  auto t= lex->ctx.cur;
  lex->ctx.cur=lex->getNextToken();
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* SimplePascalParser::eat(int wanted) {
  auto t= (Token*) lex->ctx.cur;
  if (t->type() != wanted) {
    ::sprintf(BUF, "Expected token %s, found token %s near line %d, col %d.\n",
        Token::typeToString(wanted).c_str(),
        t->toString().c_str(), t->impl.line, t->impl.col);
    throw d::SyntaxError(BUF);
  }
  lex->ctx.cur=lex->getNextToken();
  return t;
}

/* GRAMMER
        program : PROGRAM variable SEMI block DOT

        block : declarations compound_statement

        declarations : (VAR (variable_declaration SEMI)+)? procedure_declaration*

        variable_declaration : ID (COMMA ID)* COLON type_spec

        procedure_declaration :
             PROCEDURE ID (LPAREN formal_parameter_list RPAREN)? SEMI block SEMI

        formal_params_list : formal_parameters
                           | formal_parameters SEMI formal_parameter_list

        formal_parameters : ID (COMMA ID)* COLON type_spec

        type_spec : INTEGER | REAL

        compound_statement : BEGIN statement_list END

        statement_list : statement
                       | statement SEMI statement_list

        statement : compound_statement
                  | proccall_statement
                  | assignment_statement
                  | empty

        proccall_statement : ID LPAREN (expr (COMMA expr)*)? RPAREN

        assignment_statement : variable ASSIGN expr

        empty :

        expr : term ((PLUS | MINUS) term)*

        term : factor ((MUL | INTEGER_DIV | FLOAT_DIV) factor)*

        factor : PLUS factor
               | MINUS factor
               | INTEGER_CONST
               | REAL_CONST
               | LPAREN expr RPAREN
               | variable

        variable: ID
*/
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF
