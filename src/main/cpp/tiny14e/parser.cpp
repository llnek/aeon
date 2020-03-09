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

#include <typeinfo>
#include "parser.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#define ptoke(ps) s__cast(Token, ps->token().ptr())
#define past(a) s__cast(Ast, a.ptr())

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {
using namespace czlab::dsl;
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst compound_statement(CrenshawParser*, bool);
DslAst block(CrenshawParser*);
DslAst expr(CrenshawParser*);
DslAst b_expr(CrenshawParser*);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue boolFalse() {
  return d::DslValue(new SInt(0L));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue boolTrue() {
  return d::DslValue(new SInt(1L));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr toStr(DslValue e) {
  return e.isSome() ? e.ptr()->toString() : "";
}
SReal r_(0);
SInt n_(0);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double toReal(DslValue e) {
  auto v= e.ptr();
  auto ret= 0.0;
  if (v && typeid(*v) == typeid(r_)) {
    ret= s__cast(SReal,v)->value;
  }
  if (v && typeid(*v) == typeid(n_)) {
    ret=(double) s__cast(SInt,v)->value;
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong toInt(DslValue e) {
  auto v= e.ptr();
  llong ret=0;

  if (v && typeid(*v) == typeid(n_)) {
    ret= v__cast(llong, s__cast(SInt,v)->value);
  }
  if (v && typeid(*v) == typeid(r_)) {
    ret= v__cast(llong, s__cast(SReal,v)->value);
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue cast(DslValue v, DslSymbol t) {
  auto s = t.ptr()->name;
  if (s == "INTEGER") {
    return d::DslValue(new SInt(toInt(v)));
  }
  if (s== "REAL") {
    return d::DslValue(new SReal(toReal(v)));
  }
  if (s== "STRING") {
    return d::DslValue(new SStr(toStr(v)));
  }
  raise(d::SemanticError, "Unknown type %s", s.c_str());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool toBool(DslValue e) {
  return toInt(e) != 0L;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BinOp::BinOp(DslAst left, DslToken op, DslAst right)
  : Ast(op), lhs(left), rhs(right) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(d::IAnalyzer* a) {
  lhs.ptr()->visit(a);
  rhs.ptr()->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BinOp::name() { return "BinOp"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BinOp::eval(d::IEvaluator* e) {

  auto lf = lhs.ptr()->eval(e);
  auto rt = rhs.ptr()->eval(e);
  auto plf = lf.ptr();
  auto prt = rt.ptr();
  double x, y, z;

  if (typeid(n_) == typeid(*plf)) {
    x= v__cast(double, s__cast(SInt,plf)->value);
  } else {
    x= s__cast(SReal,plf)->value;
  }

  if (typeid(n_) == typeid(*prt)) {
    y= v__cast(double, s__cast(SInt,prt)->value);
  } else {
    y= s__cast(SReal,prt)->value;
  }

  switch (token()->type()) {
    case d::T_MINUS: z = x - y; break;
    case d::T_PLUS: z = x + y; break;
    case d::T_MULT: z = x * y; break;
    case T_INT_DIV:
    case d::T_DIV: z = x / y; break;
    default:
      raise(d::SemanticError,
          "Bad op near line %d(%d).\n",
          token()->impl.line, token()->impl.col);
  }

  //::printf("x = %lf, y = %lf, z= %lf\n", x, y, z);

  switch (token()->type()) {
    case T_INT_DIV:
      return d::DslValue(new SInt(v__cast(llong,z)));
    case d::T_DIV:
      return d::DslValue(new SReal(z));
    default:
      return (typeid(*plf) == typeid(r_) ||
              typeid(*prt) == typeid(r_))
        ? d::DslValue(new SReal(z))
        : d::DslValue(new SInt(v__cast(llong,z)));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String::String(DslToken t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void String::visit(d::IAnalyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr String::name() { return "string"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue String::eval(d::IEvaluator* e) {
  return d::DslValue(new SStr(token()->getLiteralAsStr()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(DslToken t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(d::IAnalyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Num::name() {
  switch (token()->type()) {
    case d::T_INTEGER: return "integer";
    case d::T_REAL: return "real";
    default:
      raise(d::SyntaxError,
          "Bad number near line %d(%d).\n",
          token()->impl.line, token()->impl.col);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Num::eval(d::IEvaluator* e) {
  switch (token()->type()) {
    case d::T_INTEGER:
      return d::DslValue(new SInt(token()->getLiteralAsInt()));
    case d::T_REAL:
      return d::DslValue(new SReal(token()->getLiteralAsReal()));
    default:
      raise(d::SyntaxError,
          "Bad number near line %d(%d).\n",
          token()->impl.line, token()->impl.col);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(DslToken t, DslAst expr) : Ast(t), expr(expr) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr UnaryOp::name() {
  switch (token()->type()) {
    case d::T_MINUS: return "-";
    case d::T_PLUS: return "+";
    default:
      raise(d::SyntaxError,
          "Bad op near line %d(%d).\n",
          token()->impl.line, token()->impl.col);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(d::IAnalyzer* a) {
  expr.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue UnaryOp::eval(d::IEvaluator* e) {
  auto r = expr.ptr()->eval(e);
  auto p= r.ptr();
  if (E_NIL(p) ||
      ! (typeid(*p) == typeid(r_) ||
         typeid(*p) == typeid(n_))) {
    raise(d::SyntaxError,
        "Expected number near line %d(%d).\n",
        token()->impl.line, token()->impl.col);
  }
  if (token()->type() == d::T_MINUS) {
    if (typeid(*p) == typeid(n_))
      return d::DslValue(new SInt(- s__cast(SInt,p)->value));

    if (typeid(*p) == typeid(r_))
      return d::DslValue(new SReal(- s__cast(SReal,p)->value));
  }
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound(DslToken t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Compound::name() { return "Compound"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Compound::visit(d::IAnalyzer* a) {
  for (auto& it : statements) {
    it.ptr()->visit(a);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Compound::eval(d::IEvaluator* e) {
  d::DslValue ret;
  for (auto& it : statements) {
    ret=it.ptr()->eval(e);
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment::Assignment(DslAst left, DslToken op, DslAst right)
  : Ast(op), lhs(left), rhs(right) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Assignment::name() { return ":="; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a) {
  auto a_ = s__cast(AnalyzerAPI,a);
  auto pl= past(lhs);
  auto s= a_->lookup(pl->name());
  if (s.isNull()) {
    auto t = pl->token();
    raise(d::SemanticError,
        "Unknown var %s near line %d(%d).\n",
        pl->name().c_str(), t->impl.line, t->impl.col);
  }
  s__cast(Var,pl)->type_symbol= s.ptr()->type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Assignment::eval(d::IEvaluator* e) {
  auto pl= past(lhs);
  auto v = pl->token()->getLiteralAsStr();
  auto t= s__cast(Var,pl)->type_symbol;
  auto r= rhs.ptr()->eval(e);
  auto e_ = s__cast(EvaluatorAPI,e);
  DEBUG("Assigning value %s to %s\n",
      r.ptr()->toString().c_str(), v.c_str());
  e_->setValue(v, cast(r,t), false);
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(DslToken t) : Ast(t) {}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Var::name() {
  return token()->getLiteralAsStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(d::IAnalyzer* a) {
  auto a_ = s__cast(AnalyzerAPI,a);
  auto n = name();
  if (auto x= a_->lookup(n); x.isNull()) {
    raise(d::SemanticError,
        "Unknown var %s near line %d(%d).\n",
        n.c_str(),
        token()->impl.line, token()->impl.col);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Var::eval(d::IEvaluator* e) {
  return s__cast(EvaluatorAPI,e)->getValue( name());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type::Type(DslToken token) : Ast(token) { }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Type::name() {
  return token()->getLiteralAsStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Type::visit(d::IAnalyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Type::eval(d::IEvaluator* e) {
  return d::DslValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VarDecl::VarDecl(DslAst var, DslAst type)
: Ast(s__cast(Var,var.ptr())->token()), var_node(var), type_node(type) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr VarDecl::name() {
  return past(var_node)->name();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarDecl::visit(d::IAnalyzer* a) {
  auto type_name = past(type_node)->name();
  auto var_name = past(var_node)->name();
  auto a_ = s__cast(AnalyzerAPI,a);
  auto type_symbol = a_->lookup(type_name);
  if (type_symbol.isNull()) {
    raise(d::SemanticError,
        "Unknown type %s near line %d(%d).\n",
        type_name.c_str(),
        token()->impl.line, token()->impl.col);
  }
  if (auto x = a_->lookup(var_name, false); x.isSome()) {
    raise(d::SemanticError,
        "Duplicate var %s near line %d(%d).\n",
        var_name.c_str(),
        token()->impl.line, token()->impl.col);
  }
  s__cast(Var,var_node.ptr())->type_symbol= type_symbol;
  a_->define(DslSymbol(new d::VarSymbol(var_name, type_symbol)));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue VarDecl::eval(d::IEvaluator* e) {
  auto t= s__cast(Var,var_node.ptr())->type_symbol;
  d::DslValue v;
  return s__cast(EvaluatorAPI,e)->setValue(name(), cast(v, t), true);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BoolExpr::BoolExpr(DslToken t, const AstVec& ts, const TokenVec& ops) : Ast(t) {
  s__ccat(this->ops, ops);
  s__ccat(this->terms, ts);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BoolExpr::eval(d::IEvaluator* e) {
  auto z1= terms.size();
  auto t1 = ops.size();
  ASSERT1(z1 == (t1+1));
  auto i=0;
  auto lhs= terms[0]->eval(e);
  auto res= toBool(lhs);
  if (z1==1) {
    return lhs;
  }
  while (i < t1) {
    auto t= ops[i];
    if (t.ptr()->type() == T_OR && res) {
      return boolTrue();
    }
    auto rhs= terms[i+1]->eval(e);
    if (t.ptr()->type() == T_XOR) {
      if (res != toBool(rhs)) {
        res=true;
      }
    } else {
      if (toBool(rhs)) {
        res=true;
      }
    }
    ++i;
  }
  return res ? boolTrue() : boolFalse();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BoolExpr::visit(d::IAnalyzer* a) {
  for (auto& x : terms) {
    x.ptr()->visit(a);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolExpr::name() { return "BoolExpr"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BoolTerm::BoolTerm(DslToken t, const AstVec& ts) : Ast(t) {
  s__ccat(terms, ts);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolTerm::name() { return "bterm"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BoolTerm::visit(d::IAnalyzer* a) {
  for (auto& x : terms) {
    x.ptr()->visit(a);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BoolTerm::eval(d::IEvaluator* e) {
  ASSERT1(terms.size() > 0);
  auto i=0;
  auto z= terms.size();
  auto lhs = terms[i]->eval(e);
  auto res= toBool(lhs);
  if (z==1) {
    return lhs;
  }
  else if (!res) {
    return boolFalse();
  }
  ++i;
  while (i < z) {
    auto rhs = toBool(terms[i]->eval(e));
    res= (res && rhs);
    if (res) return boolTrue();
    ++i;
  }
  return boolFalse();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NotFactor::NotFactor(DslToken t, DslAst e) : Ast(t) {
  expr=e;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue NotFactor::eval(d::IEvaluator* e) {
  return !toBool(expr->eval(e)) ? boolTrue() : boolFalse();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void NotFactor::visit(d::IAnalyzer* a) {
  expr.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr NotFactor::name() { return "notfactor"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RelationOp::RelationOp(DslAst left, DslToken op, DslAst right) : Ast(op) {
  lhs=left;
  rhs=right;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr RelationOp::name() {
  return token()->impl.text;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void RelationOp::visit(d::IAnalyzer* a) {
  lhs.ptr()->visit(a);
  rhs.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue RelationOp::eval(d::IEvaluator* e) {
  auto l = toInt(lhs->eval(e));
  auto r = toInt(rhs->eval(e));
  auto res=false;
  switch (token()->type()) {
    case d::T_GT: res = l > r; break;
    case d::T_LT: res = l < r; break;
    case T_GTEQ: res = l >= r; break;
    case T_LTEQ: res = l <= r; break;
    case T_EQUALS: res = l == r; break;
    case T_NOTEQ: res = l != r; break;
    default:
      raise(d::SemanticError,
          "Unknown op near line %d(%d)\n.",
          token()->impl.line, token()->impl.col);
  }
  return res ? boolTrue() : boolFalse();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block::Block(DslToken t, const AstVec& decls, DslAst compound)
  : Ast(t), compound(compound) {
  s__ccat(declarations, decls);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Block::name() { return "Block"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Block::visit(d::IAnalyzer* a) {
  for (auto& x : declarations) {
    x.ptr()->visit(a);
  }
  compound.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Block::eval(d::IEvaluator* e) {
  for (auto& x : declarations) {
    x.ptr()->eval(e);
  }
  return compound.ptr()->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl::ProcedureDecl(DslToken proc_name,
                             const AstVec& pms, DslAst block_node)
  : Ast(proc_name)  {
  block=block_node;
  s__ccat(params, pms);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureDecl::visit(d::IAnalyzer* a) {
  auto fs= new d::FnSymbol( name());
  auto a_ = s__cast(AnalyzerAPI,a);

  a_->define(DslSymbol(fs));
  a_->pushScope(name());

  for (auto& p : params) {
    auto pm = s__cast(VarDecl,p.ptr());
    auto tn = past(pm->type_node)->name();
    auto pt = a_->lookup(tn);
    auto pn = past(pm->var_node)->name();
    auto v = DslSymbol(new d::VarSymbol(pn, pt));
    a_->define(v);
    s__conj(fs->params,v);
  }
  block.ptr()->visit(a);
  a_->popScope();
  fs->block = this->block;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ProcedureDecl::name() {
  return token()->getLiteralAsStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ProcedureDecl::eval(d::IEvaluator* e) {
  return d::DslValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall::ProcedureCall(DslToken proc_name, const AstVec& p)
  : Ast(proc_name) {
  s__ccat(args, p);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ProcedureCall::name() {
  return token()->getLiteralAsStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureCall::visit(d::IAnalyzer* a) {
  for (auto& p : args) {
    p.ptr()->visit(a);
  }
  //get the corresponding symbol
  if (auto x = s__cast(AnalyzerAPI,a)->lookup( name()); x.isSome()) {
    proc_symbol = x;
  } else {
    raise(d::SemanticError,
        "Unknown proc %s near line %d(%d).\n",
        name().c_str(), token()->impl.line, token()->impl.col);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ProcedureCall::eval(d::IEvaluator* e) {
  auto fs= s__cast(d::FnSymbol,proc_symbol.ptr());
  ASSERT1(fs->params.size() == args.size());
  auto e_ = s__cast(EvaluatorAPI,e);
  e_->push(name());

  for (auto i=0; i < args.size(); ++i) {
    auto p= s__cast(d::VarSymbol,fs->params[i].ptr());
    auto v= args[i]->eval(e);
    e_->setValue(p->name, cast(v, p->type), true);
  }

  auto r= (fs->block)->eval(e);
  e_->pop();
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(DslToken pname, DslAst block) : Ast(pname) {
  this->block=block;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Program::name() {
  return token()->getLiteralAsStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(d::IAnalyzer* a) {
  block.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Program::eval(d::IEvaluator* e) {
  return block.ptr()->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst variable(CrenshawParser* ps) {
  return DslAst(new Var(ps->eat(d::T_IDENT)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst factor(CrenshawParser* ps) {
  auto t= ps->token();
  DslAst res;
  switch (t.ptr()->type()) {
    case d::T_PLUS:
      res= (ps->eat(), DslAst(new UnaryOp(t, factor(ps))));
      break;
    case d::T_MINUS:
      res= (ps->eat(), DslAst(new UnaryOp(t, factor(ps))));
      break;
    case d::T_INTEGER:
      res= (ps->eat(), DslAst(new Num(t)));
      break;
    case d::T_REAL:
      res= (ps->eat(), DslAst(new Num(t)));
      break;
    case d::T_STRING:
      res= (ps->eat(), DslAst(new String(t)));
      break;
    case d::T_LPAREN:
      res= (ps->eat(), b_expr(ps));
      ps->eat(d::T_RPAREN);
      break;
    default:
      res= variable(ps);
  }
  return res;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst term(CrenshawParser* ps) {
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV};
  auto res= factor(ps);
  while (contains(ops,ps->cur())) {
    res = DslAst(new BinOp(res, ps->eat(), factor(ps)));
  }
  return res;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst expr(CrenshawParser* ps) {
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  auto res= term(ps);
  while (contains(ops,ps->cur())) {
    res= DslAst(new BinOp(res, ps->eat(), term(ps)));
  }
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst relation(CrenshawParser* ps) {
  static std::set<int> ops1 { d::T_GT, d::T_LT, T_GTEQ };
  static std::set<int> ops2 { T_LTEQ, T_EQUALS, T_NOTEQ };
  auto res = expr(ps);
  while (contains(ops1, ps->cur()) ||
         contains(ops2, ps->cur()) ) {
    res= DslAst(new RelationOp(res, ps->eat(), expr(ps)));
  }
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst b_factor(CrenshawParser* ps) {
  return relation(ps);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst not_factor(CrenshawParser* ps) {
  if (ps->isCur(T_NOT)) {
    ps->eat();
    return DslAst(new NotFactor(ps->getEthereal(), b_factor(ps)));
  } else {
    return b_factor(ps);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst b_term(CrenshawParser* ps) {
  AstVec res {not_factor(ps)};
  while (ps->isCur(T_AND)) {
    s__conj(res, not_factor(ps));
  }
  return DslAst(new BoolTerm(ps->getEthereal(), res));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst b_expr(CrenshawParser* ps) {
  static std::set<int> ops {T_OR, T_XOR};
  AstVec res {b_term(ps)};
  TokenVec ts;
  while (contains(ops, ps->cur())) {
    s__conj(ts, ps->token());
    ps->eat();
    s__conj(res, b_term(ps));
  }
  return DslAst(new BoolExpr(ps->getEthereal(), res, ts));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst type_spec(CrenshawParser* ps) {
  auto t = ps->token();
  switch (t.ptr()->type()) {
    case T_STR:
    case T_INT:
    case T_REAL: ps->eat(); break;
    default:
      raise(d::SyntaxError,
          "Unknown token %d near line %d(%d).\n",
          t.ptr()->type(),
          s__cast(Token,t.ptr())->impl.line,
          s__cast(Token,t.ptr())->impl.col);
  }
  return DslAst(new Type(t));
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
AstVec variable_declaration(CrenshawParser* ps) {
  AstVec vars { DslAst(new Var(ps->eat(d::T_IDENT))) };
  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(vars, DslAst(new Var(ps->eat(d::T_IDENT))));
  }
  auto type = (ps->eat(d::T_COLON), type_spec(ps));
  AstVec out;
  for (auto &x : vars) {
    s__conj(out, DslAst(new VarDecl(x, type)));
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst assignment_statement(CrenshawParser* ps) {
  auto left = variable(ps);
  auto t= ps->eat(T_ASSIGN);
  auto right = expr(ps);
  return DslAst(new Assignment(left, t, right));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst empty(CrenshawParser* ps) {
  return DslAst(new NoOp());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst proccall_statement(CrenshawParser* ps) {
  auto proc_name = ps->eat(d::T_IDENT);
  ps->eat(d::T_LPAREN);
  AstVec pms;

  if (!ps->isCur(d::T_RPAREN)) {
    s__conj(pms, expr(ps));
  }

  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(pms,expr(ps));
  }

  return (ps->eat(d::T_RPAREN),
      DslAst(new ProcedureCall(proc_name, pms)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RepeatUntil::RepeatUntil(DslToken t, DslAst e, DslAst c) : Ast(t) {
  cond=e;
  code= c;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue RepeatUntil::eval(d::IEvaluator* e) {
  auto ret= code->eval(e);
  auto c= cond->eval(e);
  auto i = 1;
  while (toBool(c)) {
    ret=code->eval(e);
    c = cond->eval(e);
    DEBUG("looping.....%d\n", i);
    ++i;
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void RepeatUntil::visit(d::IAnalyzer* a) {
  code.ptr()->visit(a);
  cond.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr RepeatUntil::name() {
  return token()->impl.text;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IfThenElse::IfThenElse(DslToken t, DslAst cond, DslAst then, DslAst elze)
  : IfThenElse(t,cond,then) {
  this->elze=elze;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IfThenElse::IfThenElse(DslToken t, DslAst cond, DslAst then) : Ast(t) {
  this->cond=cond;
  this->then=then;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue IfThenElse::eval(d::IEvaluator* e) {
  auto c= cond->eval(e);
  d::DslValue ret;

  if (toBool(c)) {
    ret= then->eval(e);
  }
  else
  if (elze.isSome()) {
    ret= elze->eval(e);
  }

  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void IfThenElse::visit(d::IAnalyzer* a) {
  cond.ptr()->visit(a);
  then.ptr()->visit(a);
  if (elze.isSome()) elze.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr IfThenElse::name() {
  return token()->impl.text;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ForLoop::ForLoop(DslToken t, DslAst v, DslAst i, DslAst e, DslAst code) : Ast(t) {
  var_node=v;
  init=i;
  term=e;
  this->code= code;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ForLoop::eval(d::IEvaluator* e) {
  auto tn= s__cast(Var,var_node.ptr())->type_symbol;
  auto e_ = s__cast(EvaluatorAPI,e);
  auto vn= past(var_node)->name();
  d::DslValue ret;

  e_->setValue(vn, cast(init->eval(e),tn),false);
  //::printf("ready\n");
  while (1) {
    auto z= toInt(term->eval(e));
    auto i= toInt(e_->getValue(vn));
    //::printf("z = %ld, i= %ld\n", z ,i);
    if (z >= i) {
      ret= code->eval(e);
      e_->setValue(vn, cast(d::DslValue(new SInt(i+1)), tn),false);
    } else {
      break;
    }
  }

  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ForLoop::visit(d::IAnalyzer* a) {
  auto vn = s__cast(Var,var_node.ptr());
  auto s= s__cast(AnalyzerAPI,a)->lookup(vn->name());
  if (s.isNull()) {
    throw d::SyntaxError("Missing type in forloop decl.");
  }
  vn->visit(a);
  vn->type_symbol = s__cast(d::VarSymbol,s.ptr())->type;
  init.ptr()->visit(a);
  term.ptr()->visit(a);
  code.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ForLoop::name() {
  return token()->impl.text;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
WhileLoop::WhileLoop(DslToken t, DslAst e, DslAst c) : Ast(t) {
  cond=e;
  code= c;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue WhileLoop::eval(d::IEvaluator* e) {
  auto c= cond->eval(e);
  d::DslValue ret;
  while (toBool(c)) {
    ret=code.ptr()->eval(e);
    c = cond.ptr()->eval(e);
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void WhileLoop::visit(d::IAnalyzer* a) {
  cond.ptr()->visit(a);
  code.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr WhileLoop::name() {
  return token()->impl.text;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VarInput::VarInput(DslToken t) : Var(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue VarInput::eval(d::IEvaluator* e) {
  auto e_ = s__cast(EvaluatorAPI,e);
  auto s= type_symbol.ptr()->name;
  d::DslValue res;

  if (s == "INTEGER") {
    res = d::DslValue(new SInt( e_->readInt()));
  } else if (s == "REAL") {
    res = d::DslValue(new SReal( e_->readFloat()));
  } else if (s == "STRING") {
    res = d::DslValue(new SStr( e_->readString()));
  }
  e_->setValue(name(), res, false);
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarInput::visit(d::IAnalyzer* a) {
  auto a_ = s__cast(AnalyzerAPI,a);
  if (auto s = a_->lookup(name()); s.isSome()) {
    type_symbol= s__cast(VarSymbol,s.ptr())->type;
  } else {
    raise(d::SemanticError,
        "Unknown var %s near line %d(%d).\n",
        name().c_str(),
        token()->impl.line, token()->impl.col);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Read::Read(DslToken t, DslAst v) : Ast(t) {
  var_node=v;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Read::eval(d::IEvaluator* e) {
  auto r= var_node.ptr()->eval(e);
  if (token()->type() == T_READLN) {
    s__cast(EvaluatorAPI,e)->writeln();
  }
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Read::visit(d::IAnalyzer* a) {
  var_node.ptr()->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Read::name() {
  return token()->impl.text;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Write::Write(DslToken t, const AstVec& ts) : Ast(t) {
  s__ccat(terms,ts);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Write::eval(d::IEvaluator* e) {
  auto e_ = s__cast(EvaluatorAPI,e);
  stdstr out;
  for (auto& x : terms) {
    out += x.ptr()->eval(e).ptr()->toString();
  }
  e_->writeString(out);
  if (token()->type() == T_WRITELN) {
    e_->writeln();
  }
  return d::DslValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Write::visit(d::IAnalyzer* a) {
  for (auto& x : terms) {
    x.ptr()->visit(a);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Write::name() {
  return token()->impl.text;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst doRepeat(CrenshawParser* ps) {
  auto w= ps->eat(T_REPEAT);
  auto c = compound_statement(ps,false);
  ps->eat(T_UNTIL);
  ps->eat(d::T_LPAREN);
  auto e = b_expr(ps);
  ps->eat(d::T_RPAREN);
  return DslAst(new RepeatUntil(w, e, c));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst doIf(CrenshawParser* ps) {
  auto w= ps->eat(T_IF);
  ps->eat(d::T_LPAREN);
  auto c = b_expr(ps);
  ps->eat(d::T_RPAREN);
  auto t = compound_statement(ps,false);
  DslAst e;
  if (ps->isCur(T_ELSE)) {
    ps->eat();
    e = compound_statement(ps,false);
  }
  ps->eat(T_ENDIF);
  return DslAst(new IfThenElse(w, c, t, e));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst doFor(CrenshawParser* ps) {
  auto w= ps->eat(T_FOR);
  auto v = DslAst(new Var(ps->eat(d::T_IDENT)));
  ps->eat(T_ASSIGN);
  auto i = expr(ps);
  auto e = expr(ps);
  auto c = compound_statement(ps,false);
  ps->eat(T_ENDFOR);
  return DslAst(new ForLoop(w, v, i, e, c));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst doWhile(CrenshawParser* ps) {
  auto w= ps->eat(T_WHILE);
  ps->eat(d::T_LPAREN);
  auto e = b_expr(ps);
  ps->eat(d::T_RPAREN);
  auto c = compound_statement(ps,false);
  ps->eat(T_ENDWHILE);
  return DslAst(new WhileLoop(w, e, c));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst doWrite(CrenshawParser* ps, bool nl) {
  DslToken t;
  AstVec ts;
  if (nl) t= ps->eat(T_WRITELN); else t= ps->eat(T_WRITE);
  ps->eat(d::T_LPAREN);
  while (!ps->isCur(d::T_RPAREN)) {
    s__conj(ts, expr(ps));
    if (!ps->isCur(d::T_RPAREN)) {
      ps->eat(d::T_COMMA);
    }
  }
  ps->eat(d::T_RPAREN);
  return DslAst(new Write(t, ts));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst doRead(CrenshawParser* ps, bool nl) {
  DslToken t;
  if (nl) t= ps->eat(T_READLN); else t= ps->eat(T_READ);
  ps->eat(d::T_LPAREN);
  auto v= DslAst(new VarInput(ps->eat(d::T_IDENT)));
  ps->eat(d::T_RPAREN);
  return DslAst(new Read(t, v));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst statement(CrenshawParser* ps) {
  switch (ps->cur()) {
    case T_BEGIN:
      return compound_statement(ps, true);
    case T_WRITELN:
      return doWrite(ps,true);
    case T_WRITE:
      return doWrite(ps,false);
    case T_READLN:
      return doRead(ps,true);
    case T_READ:
      return doRead(ps,false);
    case T_FOR:
      return doFor(ps);
    case T_IF:
      return doIf(ps);
    case T_WHILE:
      return doWhile(ps);
    case T_REPEAT:
      return doRepeat(ps);
    case d::T_IDENT:
      return (ps->peek() == '(')
        ? proccall_statement(ps)
        : assignment_statement(ps);
    default:
      /*
      auto z=(Token*) ps->token();
      ::sprintf(BUF,"Unknown token %s near line %d, col %d.\n",
          z->toString().c_str(), z->impl.line, z->impl.col);
      throw d::SyntaxError(BUF);
      */
      return empty(ps);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
AstVec statement_list(CrenshawParser* ps) {

  auto s= statement(ps);
  AstVec results;

  if (!(past(s)->name() == "709394")) {
    s__conj(results,s);
  }

  while (ps->isCur(d::T_SEMI)) {
    ps->eat();
    s= statement(ps);
    if (!(past(s)->name() == "709394")) {
      s__conj(results,s);
    }
  }

  if (ps->isCur(d::T_IDENT)) {
    raise(d::SyntaxError,
        "Unexpected token `%s`\n",
        ptoke(ps)->getLiteralAsStr().c_str());
  }

  return results;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst compound_statement(CrenshawParser* ps, bool beginend) {
  if (beginend) {ps->eat(T_BEGIN);}
  auto nodes = statement_list(ps);
  if (beginend) {ps->eat(T_END);}
  auto root= new Compound(ps->getEthereal());
  for (auto& node : nodes) {
    s__conj(root->statements,node);
  }
  return DslAst(root);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
AstVec formal_parameters(CrenshawParser* ps) {

  TokenVec param_tokens { ps->eat(d::T_IDENT) };
  AstVec pnodes;

  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(param_tokens, ps->eat(d::T_IDENT));
  }

  auto type_node = (ps->eat(d::T_COLON),type_spec(ps));

  for (auto& t : param_tokens) {
    //::printf("param toke= %s\n", t->getLiteralAsStr());
    s__conj(pnodes, DslAst(new VarDecl(DslAst(new Var(t)), type_node)));
  }

  return pnodes;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
AstVec formal_parameter_list(CrenshawParser* ps) {
  AstVec out;

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
DslAst procedure_declaration(CrenshawParser* ps) {

  auto proc_name = (ps->eat(T_PROCEDURE), ps->eat(d::T_IDENT));
  AstVec params;

  if (ps->isCur(d::T_LPAREN)) {
    params = (ps->eat(), formal_parameter_list(ps));
    ps->eat(d::T_RPAREN);
  }

  auto decl = new ProcedureDecl(proc_name,
                                params,
                                (ps->eat(d::T_SEMI), block(ps)));
  //::printf("proc name=%s\n", decl->name().c_str());
  return (ps->eat(d::T_SEMI), DslAst(decl));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
AstVec declarations(CrenshawParser* ps) {
  AstVec ds;

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
DslAst block(CrenshawParser* ps) {
  auto decls=declarations(ps);
  return DslAst(new Block(ps->getEthereal(), decls, compound_statement(ps, true)));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst program(CrenshawParser* ps) {
  auto prog_name = (ps->eat(T_PROGRAM),ps->eat(d::T_IDENT));
  auto prog = new Program(prog_name,
                          (ps->eat(d::T_SEMI),block(ps)));
  //::printf("program = %s\n", prog->name().c_str());
  return (ps->eat(d::T_DOT), DslAst(prog));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CrenshawParser::~CrenshawParser() {
  del_ptr(lex);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CrenshawParser::CrenshawParser(const char* src) {
  lex = new Lexer(src);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslAst CrenshawParser::parse() {
  return program(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken CrenshawParser::getEthereal() {
  return DslToken(new Token(d::T_ETHEREAL, "", s__pair(int,int,lex->ctx.line, lex->ctx.col)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int CrenshawParser::cur() {
  return lex->ctx.cur->type();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char CrenshawParser::peek() {
  return d::peek(lex->ctx);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool CrenshawParser::isCur(int type) {
  return lex->ctx.cur->type() == type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken CrenshawParser::token() {
  return lex->ctx.cur;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken CrenshawParser::eat() {
  auto t= token();
  lex->ctx.cur = lex->getNextToken();
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken CrenshawParser::eat(int wanted) {
  auto t= token();
  if (t.ptr()->type() != wanted) {
    raise(d::SyntaxError,
        "Expected token %s, found token %s near line %d(%d).\n",
        Token::typeToString(wanted).c_str(),
        t->toString().c_str(),
        s__cast(Token,t.ptr())->impl.line,
        s__cast(Token,t.ptr())->impl.col);
  }
  lex->ctx.cur= lex->getNextToken();
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
/*
<b-expression> ::= <b-term> [<orop> <b-term>]*
 <b-term>       ::= <not-factor> [AND <not-factor>]*
 <not-factor>   ::= [NOT] <b-factor>
 <b-factor>     ::= <b-literal> | <b-variable> | <relation>
 <relation>     ::= | <expression> [<relop> <expression]

 <expression>   ::= <term> [<addop> <term>]*
 <term>         ::= <signed factor> [<mulop> factor]*
 <signed factor>::= [<addop>] <factor>
 <factor>       ::= <integer> | <variable> | (<b-expression>)

 */
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF
