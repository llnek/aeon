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
d::DslAst compound_statement(SimplePascalParser*);
d::DslAst block(SimplePascalParser*);
d::DslAst expr(SimplePascalParser*);
static char BUF[1024];

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast(d::DslToken t) {
  token=t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast() {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BinOp::BinOp(d::DslAst left, d::DslToken op, d::DslAst right)
  : Ast(op), lhs(left), rhs(right) {
  _name= "BinOp";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(d::IAnalyzer* a) {
  lhs->visit(a);
  rhs->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BinOp::eval(d::IEvaluator* e) {

  auto lf = lhs->eval(e);
  auto rt = rhs->eval(e);
  auto lf_= s__cast(EVal,lf.ptr());
  auto rt_= s__cast(EVal,rt.ptr());
  double x, y, z;

  if (lf_->type == d::T_INTEGER)  {
    x= (double) lf_->u.n;
  } else { x= lf_->u.r; }

  if (rt_->type == d::T_INTEGER)  {
    y= (double) rt_->u.n;
  } else { y= rt_->u.r; }

  switch (token->type()) {
    case d::T_MINUS: z = x - y; break;
    case d::T_PLUS: z = x + y; break;
    case d::T_MULT: z = x * y; break;
    case T_INT_DIV:
    case d::T_DIV: z = x / y; break;
    default:
      auto t= s__cast(Token,token.ptr());
      ::sprintf(BUF,
          "Bad binary-op near line %d, col %d.\n",
          t->impl().line, t->impl().col);
      throw d::SyntaxError(BUF);
  }

  switch (token->type()) {
    case T_INT_DIV:
      return d::DslValue(new EVal((llong) z));
    case d::T_DIV:
      return d::DslValue(new EVal(z));
    default:
      return (lf_->type == d::T_REAL ||
              rt_->type == d::T_REAL)
        ? d::DslValue(new EVal(z))
        : d::DslValue(new EVal((llong) z));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String::String(d::DslToken t) : Ast(t) {
  _name="string";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void String::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue String::eval(d::IEvaluator* e) {
  return d::DslValue(new EVal(token->getLiteralAsStr()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Num::name() const {
  switch (token->type()) {
    case d::T_INTEGER: return "integer";
    case d::T_REAL: return "real";
    default:
      auto t= s__cast(Token,token.ptr());
      ::sprintf(BUF,
          "Bad number near line %d, col %d.\n",
          t->impl().line, t->impl().col);
      throw d::SyntaxError(BUF);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Num::eval(d::IEvaluator* e) {
  switch (token->type()) {
    case d::T_INTEGER:
      return d::DslValue(new EVal(token->getLiteralAsInt()));
    case d::T_REAL:
      return d::DslValue(new EVal(token->getLiteralAsReal()));
    default:
      auto t= s__cast(Token,token.ptr());
      ::sprintf(BUF,
          "Bad number near line %d, col %d.\n",
          t->impl().line, t->impl().col);
      throw d::SyntaxError(BUF);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(d::DslToken t, d::DslAst expr) : Ast(t), expr(expr) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr UnaryOp::name() const {
  switch (token->type()) {
    case d::T_PLUS: return "+";
    case d::T_MINUS: return "-";
    default:
      auto t= s__cast(Token,token.ptr());
      ::sprintf(BUF,
          "Bad unary-op near line %d, col %d.\n",
          t->impl().line, t->impl().col);
      throw d::SyntaxError(BUF);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue UnaryOp::eval(d::IEvaluator* e) {
  auto r = expr->eval(e);
  auto r_ = s__cast(EVal,r.ptr());

  if (! (r_->type == d::T_REAL || r_->type == d::T_INTEGER)) {
    auto t=s__cast(Token,token.ptr());
    ::sprintf(BUF,
        "Expected numeric type near line %d, col %d.\n",
        t->impl().line, t->impl().col);
    throw d::SyntaxError(BUF);
  }

  if (token->type() == d::T_MINUS) {
    if (r_->type == d::T_INTEGER)
      return d::DslValue(new EVal( - r_->u.n));
    if (r_->type == d::T_REAL)
      return d::DslValue(new EVal( - r_->u.r));
  }

  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound() : Ast() { _name="Compound"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Compound::visit(d::IAnalyzer* a) {
  for (auto& it : statements) {
    it->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Compound::eval(d::IEvaluator* e) {
  d::DslValue ret;
  for (auto& it : statements) {
    ret=it->eval(e);
  }
  return ret;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment::Assignment(d::DslAst left, d::DslToken op, d::DslAst right)
  : Ast(op), lhs(left), rhs(right) {
  _name=":=";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a) {
  auto lhs_ = s__cast(Ast,lhs.ptr());
  auto t = s__cast(Token,lhs_->token.ptr());
  auto v = t->getLiteralAsStr();
  if (auto s= a->lookup(v); s.isNull()) {
    ::sprintf(BUF,
              "Unknown var %s near line %d col %d.\n",
              v.c_str(), t->impl().line, t->impl().col);
    throw d::SyntaxError(BUF);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Assignment::eval(d::IEvaluator* e) {
  auto lhs_= s__cast(Ast,lhs.ptr());
  auto t = s__cast(Token,lhs_->token.ptr());
  auto v = t->getLiteralAsStr();
  auto r= rhs->eval(e);
  ::printf("Assigning value %s to %s\n",
      r->toString().c_str(),
      v.c_str());
  e->setValue(v, r, false);
  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(d::DslToken t) : Ast(t) {
  _name= s__cast(Token,token.ptr())->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(d::IAnalyzer* a) {
  auto t = s__cast(Token,token.ptr());
  auto n = t->getLiteralAsStr();
  if (auto s= a->lookup(n); s.isNull()) {
    ::sprintf(BUF,
              "Unknown var %s near line %d col %d.\n",
              n.c_str(), t->impl().line, t->impl().col);
    throw d::SyntaxError(BUF);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Var::eval(d::IEvaluator* e) {
  auto t = s__cast(Token,token.ptr());
  auto n = t->getLiteralAsStr();
  return e->getValue(n);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type::Type(d::DslToken token) : Ast(token) {
  _name= s__cast(Token,token.ptr())->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Type::visit(d::IAnalyzer* a) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Type::eval(d::IEvaluator* e) {
  return d::DslValue();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymTable::SymTable(const stdstr& n, d::DslTable outer) : SymTable(n) {
  enclosing=outer;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymTable::SymTable(const std::string& n) : d::Table(n) {
  insert(new BuiltinTypeSymbol("INTEGER"));
  insert(new BuiltinTypeSymbol("REAL"));
  insert(new BuiltinTypeSymbol("STRING"));
  ::printf("Added built-in types.\n");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Param::Param(d::DslAst var, d::DslAst type)
  : Ast(), var_node(var), type_node(type) {
  _name = s__cast(Ast,var_node.ptr())->name();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Param::visit(d::IAnalyzer* a) {
  auto n= s__cast(Ast,type_node.ptr())->name();
  auto t= a->lookup(n);
  if (t.isNull()) {
    auto k= s__cast(Token,token.ptr());
    ::sprintf(BUF,
              "Unknown type %s near line %d col %d.\n",
              n.c_str(), k->impl().line, k->impl().col);
    throw d::SyntaxError("Unknown param type.");
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Param::eval(d::IEvaluator* e) {
  return d::DslValue();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VarDecl::VarDecl(d::DslAst var, d::DslAst type)
  : Ast(s__cast(Ast,var.ptr())->token), var_node(var), type_node(type) {
  _name= s__cast(Ast,var_node.ptr())->name();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarDecl::visit(d::IAnalyzer* a) {
  auto type_name = s__cast(Ast,type_node.ptr())->name();
  auto var_name = s__cast(Ast,var_node.ptr())->name();
  auto type_symbol = a->lookup(type_name);
  if (type_symbol.isNull()) {
    auto k=s__cast(Token,token.ptr());
    ::sprintf(BUF,
              "Unknown type %s near line %d col %d.\n",
              type_name.c_str(), k->impl().line, k->impl().col);
    throw d::SyntaxError(BUF);
  }

  if (auto s = a->lookup(var_name.c_str(), false); s.isSome()) {
    auto k=s__cast(Token,token.ptr());
    ::sprintf(BUF,
              "Duplicate var %s near line %d col %d.\n",
              var_name.c_str(), k->impl().line, k->impl().col);
    throw d::SyntaxError(BUF);
  } else {
    a->define(d::DslSymbol(new d::VarSymbol(var_name, type_symbol)));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue VarDecl::eval(d::IEvaluator* e) {
  d::DslValue v;
  e->setValue(this->name(), v, true);
  return v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block::Block(std::vector<d::DslAst>& decls, d::DslAst compound)
  : Ast(), compound(compound) {
  _name="Block";
  s__ccat(declarations, decls);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Block::visit(d::IAnalyzer* a) {
  for (auto& x : declarations) {
    x->visit(a);
  }
  compound->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Block::eval(d::IEvaluator* e) {
  for (auto& x : declarations) {
    auto v= x->eval(e);
  }
  auto v= compound->eval(e);
  return v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl::ProcedureDecl(const stdstr& proc_name,
    std::vector<d::DslAst>& pms, d::DslAst block_node)
  : Ast()  {
  _name=proc_name;
  block=block_node;
  s__ccat(params, pms);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureDecl::visit(d::IAnalyzer* a) {
  auto fs= new d::FnSymbol( name());
  a->define(d::DslSymbol(fs));
  a->pushScope(fs->name());

  for (auto& p : params) {
    auto p_ = s__cast(Param,p.ptr());
    auto t = p_->type_node;
    auto tn = s__cast(Ast,t.ptr())->name();
    auto pt= a->lookup(tn);
    auto pn = s__cast(Ast,p_->var_node.ptr())->name();
    auto v = d::DslSymbol(new d::VarSymbol(pn, pt));
    a->define(v);
    s__conj(fs->params(), v);
  }
  block->visit(a);
  a->popScope();
  fs->setBody(this->block);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ProcedureDecl::eval(d::IEvaluator* e) {
  return d::DslValue();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall::ProcedureCall(const stdstr& proc_name,
                             std::vector<d::DslAst>& p, d::DslToken token)
  : Ast(token) {
  _name=proc_name;
  s__ccat(args, p);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureCall::visit(d::IAnalyzer* a) {
  for (auto& p : args) {
    p->visit(a);
  }
  //get the corresponding symbol
  auto x = a->lookup(_name);
  if (x.isSome()) {
    proc_symbol = s__cast(d::FnSymbol,x.ptr());
  } else {
    auto k=s__cast(Token,token.ptr());
    ::sprintf(BUF,
              "Unknown proc %s near line %d col %d.\n",
              _name.c_str(), k->impl().line, k->impl().col);
    throw d::SyntaxError(BUF);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ProcedureCall::eval(d::IEvaluator* e) {

  auto fs= s__cast(d::FnSymbol,proc_symbol.ptr());
  ASSERT1(fs->params().size() == args.size());
  e->pushFrame(_name);

  for (auto i=0; i < args.size(); ++i) {
    auto& p= fs->params()[i];
    auto v= args[i]->eval(e);
    e->setValue(s__cast(d::VarSymbol, p.ptr())->name(), v, true);
  }

  auto r= fs->body()->eval(e);
  e->popFrame();
  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(const stdstr& name, d::DslAst block) : Ast() {
  _name=name;
  this->block=block;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(d::IAnalyzer* a) {
  block->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Program::eval(d::IEvaluator* e) {
  return block->eval(e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst variable(SimplePascalParser* ps) {
  auto node = new Var(ps->token());
  ps->eat(d::T_IDENT);
  return d::DslAst(node);
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst factor(SimplePascalParser* ps) {
  auto t= ps->token();
  d::DslAst res;

  switch (t->type()) {
    case d::T_PLUS:
      res= (ps->eat(), d::DslAst(new UnaryOp(t, factor(ps))));
      break;
    case d::T_MINUS:
      res= (ps->eat(), d::DslAst(new UnaryOp(t, factor(ps))));
      break;
    case d::T_INTEGER:
      res= (ps->eat(), d::DslAst(new Num(t)));
      break;
    case d::T_REAL:
      res= (ps->eat(), d::DslAst(new Num(t)));
      break;
    case d::T_STRING:
      res= (ps->eat(), d::DslAst(new String(t)));
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
d::DslAst term(SimplePascalParser* ps) {
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV};
  auto res= factor(ps);
  while (s__contains(ops,ps->cur())) {
    res = new BinOp(res, ps->eat(), factor(ps));
  }
  return res;
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst expr(SimplePascalParser* ps) {
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  d::DslAst res= term(ps);
  while (s__contains(ops,ps->cur())) {
    res= new BinOp(res, ps->eat(), term(ps));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst type_spec(SimplePascalParser* ps) {
  auto t = ps->token();
  switch (t->type()) {
    case T_STR:
    case T_INT:
    case T_REAL: ps->eat(); break;
    default:
      auto k=s__cast(Token,t.ptr());
      ::sprintf(BUF,"Unknown token %d near line %d, col %d.\n",
          t->type(), k->impl().line, k->impl().col);
      throw d::SyntaxError(BUF);
  }
  return d::DslAst(new Type(t));
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<d::DslAst> variable_declaration(SimplePascalParser* ps) {
  std::vector<d::DslAst> vars { d::DslAst(new Var(ps->eat(d::T_IDENT))) };
  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(vars, d::DslAst(new Var(ps->eat(d::T_IDENT))));
  }
  ps->eat(d::T_COLON);
  auto type = type_spec(ps);
  std::vector<d::DslAst> out;
  for (auto &x : vars) {
    s__conj(out, d::DslAst(new VarDecl(x, type)));
  }

  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst assignment_statement(SimplePascalParser* ps) {
  auto left = variable(ps);
  auto t= ps->eat(T_ASSIGN);
  auto right = expr(ps);
  return d::DslAst(new Assignment(left, t, right));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst empty(SimplePascalParser* ps) {
  return d::DslAst(new NoOp());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst proccall_statement(SimplePascalParser* ps) {
  auto token = ps->token();
  auto proc_name = token->getLiteralAsStr();
  std::vector<d::DslAst> pms;

  ps->eat(d::T_IDENT);
  ps->eat(d::T_LPAREN);

  if (!ps->isCur(d::T_RPAREN)) {
    s__conj(pms, expr(ps));
  }

  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(pms,expr(ps));
  }

  return (ps->eat(d::T_RPAREN),
      d::DslAst(new ProcedureCall(proc_name, pms, token)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst statement(SimplePascalParser* ps) {
  d::DslAst node;
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
std::vector<d::DslAst> statement_list(SimplePascalParser* ps) {

  std::vector<d::DslAst> results;
  auto s= statement(ps);

  if (!(s__cast(Ast,s.ptr())->name() == "709394")) {
    s__conj(results,s);
  }

  while (ps->isCur(d::T_SEMI)) {
    ps->eat();
    s= statement(ps);
    if (!(s__cast(Ast,s.ptr())->name() == "709394")) {
      s__conj(results,s);
    }
  }

  if (ps->isCur(d::T_IDENT)) {
    auto k=s__cast(Token, ps->token().ptr());
    ::sprintf(BUF,
        "Unexpected token `%s`",
        k->getLiteralAsStr().c_str());
    throw d::SyntaxError(BUF);
  }

  return results;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst compound_statement(SimplePascalParser* ps) {
  auto nodes = (ps->eat(T_BEGIN),statement_list(ps));
  auto root= (ps->eat(T_END), new Compound());
  for (auto& node : nodes) {
    s__conj(root->statements,node);
  }
  return d::DslAst(root);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<d::DslAst> formal_parameters(SimplePascalParser* ps) {

  std::vector<d::DslToken> param_tokens { ps->eat(d::T_IDENT)};
  std::vector<d::DslAst> pnodes;

  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(param_tokens, ps->eat(d::T_IDENT));
  }

  auto type_node = (ps->eat(d::T_COLON),type_spec(ps));

  for (auto& t : param_tokens) {
    //::printf("param toke= %s\n", t->getLiteralAsStr());
    s__conj(pnodes, d::DslAst(new Param(new Var(t), type_node)));
  }

  return pnodes;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<d::DslAst> formal_parameter_list(SimplePascalParser* ps) {
  std::vector<d::DslAst> out;

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
d::DslAst procedure_declaration(SimplePascalParser* ps) {

  auto proc_name = (ps->eat(T_PROCEDURE), ps->eat(d::T_IDENT));
  std::vector<d::DslAst> params;

  if (ps->isCur(d::T_LPAREN)) {
    params = (ps->eat(), formal_parameter_list(ps));
    ps->eat(d::T_RPAREN);
  }

  auto decl = new ProcedureDecl(proc_name->getLiteralAsStr(),
                                params,
                                (ps->eat(d::T_SEMI), block(ps)));
  //::printf("proc name=%s\n", decl->name().c_str());
  return (ps->eat(d::T_SEMI), d::DslAst(decl));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<d::DslAst> declarations(SimplePascalParser* ps) {
  std::vector<d::DslAst> ds;

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
d::DslAst block(SimplePascalParser* ps) {
  auto decls=declarations(ps);
  return d::DslAst(new Block(decls, compound_statement(ps)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst program(SimplePascalParser* ps) {
  auto var_node = (ps->eat(T_PROGRAM),variable(ps));
  auto prog_name = s__cast(Ast,var_node.ptr())->name();
  auto prog = new Program(prog_name.c_str(),
                          (ps->eat(d::T_SEMI),block(ps)));
  //::printf("program = %s\n", prog->name().c_str());
  return (ps->eat(d::T_DOT), d::DslAst(prog));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SimplePascalParser::~SimplePascalParser() {
  DEL_PTR(lex);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SimplePascalParser::SimplePascalParser(const char* src) {
  lex = new Lexer(src);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SimplePascalParser::isEof() const {
  return lex->ctx().eof;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst SimplePascalParser::parse() {
  return program(this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SimplePascalParser::cur() {
  return lex->ctx().cur->type();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar SimplePascalParser::peek() {
  return d::peek(lex->ctx());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SimplePascalParser::isCur(int type) {
  return lex->ctx().cur->type() == type;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken SimplePascalParser::token() {
  return lex->ctx().cur;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken SimplePascalParser::eat() {
  auto t= lex->ctx().cur;
  lex->ctx().cur=lex->getNextToken();
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken SimplePascalParser::eat(int wanted) {
  auto t= lex->ctx().cur;
  if (t->type() != wanted) {
    auto k=s__cast(Token,t.ptr());
    ::sprintf(BUF, "Expected token %s, found token %s near line %d, col %d.\n",
        Token::typeToString(wanted).c_str(),
        k->toString().c_str(), k->impl().line, k->impl().col);
    throw d::SyntaxError(BUF);
  }
  lex->ctx().cur=lex->getNextToken();
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
