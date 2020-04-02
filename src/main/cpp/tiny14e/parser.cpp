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

#define P_TOKE(ps) s__cast(Token, ps->token().ptr())
#define CAST(t,a) s__cast(t,a.ptr())
#define P_AST(a) CAST(Ast,a)


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst compound_statement(CrenshawParser*, bool);
d::DslAst block(CrenshawParser*);
d::DslAst expr(CrenshawParser*);
d::DslAst b_expr(CrenshawParser*);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue boolFalse() {
  return new SInt(0L);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue boolTrue() {
  return new SInt(1L);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr toStr(d::DslValue e) {
  return e.isSome() ? e->pr_str(1) : "";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SReal r_(0);
SInt n_(0);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double toReal(d::DslValue e) {
  auto v= e.ptr();
  auto ret= 0.0;

  if (v && typeid(*v) == typeid(r_)) {
    ret= CAST(SReal,e)->value;
  }
  if (v && typeid(*v) == typeid(n_)) {
    ret= CAST(SInt,e)->value;
  }

  return ret;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong toInt(d::DslValue e) {
  auto v= e.ptr();
  llong ret=0;

  if (v && typeid(*v) == typeid(n_)) {
    ret= CAST(SInt,e)->value;
  }
  if (v && typeid(*v) == typeid(r_)) {
    ret= CAST(SReal,e)->value;
  }

  return ret;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue cast(d::DslValue v, d::DslSymbol t) {
  auto s = t.ptr()->name();
  if (s == "INTEGER") {
    return new SInt(toInt(v));
  }
  if (s== "REAL") {
    return new SReal(toReal(v));
  }
  if (s== "STRING") {
    return new SStr(toStr(v));
  }
  RAISE(d::SemanticError, "Unknown type %s", s.c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool toBool(d::DslValue e) {
  return toInt(e) != 0L;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BinOp::BinOp(d::DslAst left, d::DslToken op, d::DslAst right)
  : Ast(op), lhs(left), rhs(right) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(d::IAnalyzer* a) {
  lhs->visit(a);
  rhs->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BinOp::name() const { return "BinOp"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BinOp::eval(d::IEvaluator* e) {

  auto lf = lhs->eval(e);
  auto rt = rhs->eval(e);
  auto plf = lf.ptr();
  auto prt = rt.ptr();
  double x, y, z;

  if (typeid(n_) == typeid(*plf)) {
    x= CAST(SInt,lf)->value;
  } else {
    x= CAST(SReal,lf)->value;
  }

  if (typeid(n_) == typeid(*prt)) {
    y= CAST(SInt,rt)->value;
  } else {
    y= CAST(SReal,rt)->value;
  }

  switch (token()->type()) {
    case d::T_MINUS: z = x - y; break;
    case d::T_PLUS: z = x + y; break;
    case d::T_MULT: z = x * y; break;

    case T_INT_DIV:
      z = x / y;
      return new SInt((llong) z);

    case d::T_DIV:
      z = x / y;
      return new SReal(z);

    default:
      RAISE(d::SemanticError,
            "Bad op near line %d(%d).\n",
            token()->srcInfo().first, token()->srcInfo().second);
  }

  //::printf("x = %lf, y = %lf, z= %lf\n", x, y, z);

  return (typeid(*plf) == typeid(r_) ||
          typeid(*prt) == typeid(r_))
         ? d::DslValue(new SReal(z))
         : d::DslValue(new SInt((llong)z));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String::String(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void String::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr String::name() const { return "string"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue String::eval(d::IEvaluator* e) {
  return new SStr(token()->getLiteralAsStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Num::name() const {
  switch (token()->type()) {
    case d::T_INTEGER: return "integer";
    case d::T_REAL: return "real";
    default:
      RAISE(d::SyntaxError,
            "Bad number near line %d(%d).\n",
            token()->srcInfo().first, token()->srcInfo().second);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Num::eval(d::IEvaluator* e) {
  switch (token()->type()) {
    case d::T_INTEGER:
      return new SInt(token()->getLiteralAsInt());
    case d::T_REAL:
      return new SReal(token()->getLiteralAsReal());
    default:
      RAISE(d::SyntaxError,
            "Bad number near line %d(%d).\n",
            token()->srcInfo().first, token()->srcInfo().second);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(d::DslToken t, d::DslAst expr) : Ast(t), expr(expr) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr UnaryOp::name() const {
  switch (token()->type()) {
    case d::T_MINUS: return "-";
    case d::T_PLUS: return "+";
    default:
      RAISE(d::SyntaxError,
            "Bad op near line %d(%d).\n",
            token()->srcInfo().first, token()->srcInfo().second);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue UnaryOp::eval(d::IEvaluator* e) {
  auto r = expr->eval(e);
  auto p= r.ptr();

  if (E_NIL(p) ||
      ! (typeid(*p) == typeid(r_) ||
         typeid(*p) == typeid(n_))) {
    RAISE(d::SyntaxError,
          "Expected number near line %d(%d).\n",
          token()->srcInfo().first, token()->srcInfo().second);
  }

  if (token()->type() == d::T_MINUS) {
    if (typeid(*p) == typeid(n_))
      return new SInt(- CAST(SInt,r)->value);

    if (typeid(*p) == typeid(r_))
      return new SReal(- CAST(SReal,r)->value);
  }

  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Compound::name() const { return "Compound"; }

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
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Assignment::name() const { return ":="; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a) {
  auto pl= P_AST(lhs);
  auto s= a->search(pl->name());
  if (s.isNull()) {
    RAISE(d::SemanticError,
          "Unknown var %s near line %d(%d).\n",
          C_STR(pl->name()),
          pl->token()->srcInfo().first,
          pl->token()->srcInfo().second);
  }
  s__cast(Var,pl)->type_symbol= s->type();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Assignment::eval(d::IEvaluator* e) {
  auto pl= P_AST(lhs);
  auto v = pl->token()->getLiteralAsStr();
  auto t= s__cast(Var,pl)->type_symbol;
  auto r= rhs->eval(e);
  DEBUG("Assigning value %s to %s\n",
        C_STR(r->pr_str()), C_STR(v));
  return e->setValueEx(v, cast(r,t));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Var::name() const {
  return token()->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(d::IAnalyzer* a) {
  auto n = name();
  if (auto x= a->search(n); x.isNull()) {
    RAISE(d::SemanticError,
          "Unknown var %s near line %d(%d).\n",
          n.c_str(),
          token()->srcInfo().first, token()->srcInfo().second);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Var::eval(d::IEvaluator* e) {
  return e->getValue( name());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type::Type(d::DslToken token) : Ast(token) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Type::name() const {
  return token()->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Type::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Type::eval(d::IEvaluator* e) {
  return nullptr;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VarDecl::VarDecl(d::DslAst var, d::DslAst type)
  : Ast(CAST(Var,var)->token()), var_node(var), type_node(type) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr VarDecl::name() const {
  return P_AST(var_node)->name();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarDecl::visit(d::IAnalyzer* a) {
  auto tname = P_AST(type_node)->name();
  auto vname = P_AST(var_node)->name();
  auto tsym = a->search(tname);

  if (tsym.isNull()) {
    RAISE(d::SemanticError,
          "Unknown type %s near line %d(%d).\n",
          C_STR(tname),
          token()->srcInfo().first, token()->srcInfo().second);
  }

  if (auto x = a->find(vname); x.isSome()) {
    RAISE(d::SemanticError,
          "Duplicate var %s near line %d(%d).\n",
          C_STR(vname),
          token()->srcInfo().first, token()->srcInfo().second);
  }

  CAST(Var,var_node)->type_symbol= tsym;
  a->define(new d::VarSymbol(vname, tsym));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue VarDecl::eval(d::IEvaluator* e) {
  auto t= CAST(Var,var_node)->type_symbol;
  d::DslValue v;
  return e->setValue(name(), cast(v, t));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BoolExpr::BoolExpr(d::DslToken t, const AstVec& ts, const TokenVec& ops) : Ast(t) {
  s__ccat(this->ops, ops);
  s__ccat(this->terms, ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BoolExpr::eval(d::IEvaluator* e) {
  auto z1= terms.size();
  auto t1= ops.size();
  ASSERT1(z1 == (t1+1));
  auto i=0;
  auto lhs= terms[0]->eval(e);
  auto res= toBool(lhs);
  if (z1==1) {
    return lhs;
  }
  while (i < t1) {
    auto t= ops[i];
    if (t->type() == T_OR && res) {
      return boolTrue();
    }
    auto rhs= terms[i+1]->eval(e);
    if (t->type() == T_XOR) {
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
    x->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolExpr::name() const { return "BoolExpr"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BoolTerm::BoolTerm(d::DslToken t, const AstVec& ts) : Ast(t) {
  s__ccat(terms, ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolTerm::name() const { return "bterm"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BoolTerm::visit(d::IAnalyzer* a) {
  for (auto& x : terms) {
    x->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BoolTerm::eval(d::IEvaluator* e) {
  ASSERT1(terms.size() > 0);
  auto i=0;
  auto z=terms.size();
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
NotFactor::NotFactor(d::DslToken t, d::DslAst e) : Ast(t) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue NotFactor::eval(d::IEvaluator* e) {
  return !toBool(expr->eval(e)) ? boolTrue() : boolFalse();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void NotFactor::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr NotFactor::name() const { return "notfactor"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RelationOp::RelationOp(d::DslAst left, d::DslToken op, d::DslAst right) : Ast(op) {
  lhs=left;
  rhs=right;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr RelationOp::name() const {
  return token()->impl().txt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void RelationOp::visit(d::IAnalyzer* a) {
  lhs->visit(a);
  rhs->visit(a);
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
      RAISE(d::SemanticError,
            "Unknown op near line %d(%d)\n.",
            token()->srcInfo().first, token()->srcInfo().second);
  }
  return res ? boolTrue() : boolFalse();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block::Block(d::DslToken t, const AstVec& decls, d::DslAst compound)
  : Ast(t), compound(compound) {
  s__ccat(declarations, decls);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Block::name() const { return "Block"; }

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
    x->eval(e);
  }
  return compound->eval(e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl::ProcedureDecl(d::DslToken proc_name,
                             const AstVec& pms, d::DslAst block_node)
  : Ast(proc_name)  {
  block=block_node;
  s__ccat(params, pms);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureDecl::visit(d::IAnalyzer* a) {
  auto fs= new d::FnSymbol( name());

  a->define(fs);
  a->pushScope(name());

  for (auto& p : params) {
    auto pm = CAST(VarDecl,p);
    auto tn = P_AST(pm->type_node)->name();
    auto pn = P_AST(pm->var_node)->name();
    auto pt = a->search(tn);
    auto v = new d::VarSymbol(pn, pt);
    a->define(v);
    s__conj(fs->params(),v);
  }

  block->visit(a);
  a->popScope();
  fs->setBody(this->block);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ProcedureDecl::name() const {
  return token()->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ProcedureDecl::eval(d::IEvaluator* e) {
  return nullptr;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall::ProcedureCall(d::DslToken proc_name, const AstVec& p)
  : Ast(proc_name) {
  s__ccat(args, p);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ProcedureCall::name() const {
  return token()->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureCall::visit(d::IAnalyzer* a) {
  for (auto& p : args) {
    p->visit(a);
  }
  //get the corresponding symbol
  if (auto x = a->search( name()); x.isSome()) {
    proc_symbol = x;
  } else {
    RAISE(d::SemanticError,
          "Unknown proc %s near line %d(%d).\n",
          C_STR(name()),
          token()->srcInfo().first, token()->srcInfo().second);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ProcedureCall::eval(d::IEvaluator* e) {
  auto fs= CAST(d::FnSymbol,proc_symbol);
  ASSERT1(fs->params().size() == args.size());
  e->pushFrame(name());

  for (auto i=0; i < args.size(); ++i) {
    auto p= CAST(d::VarSymbol,fs->params()[i]);
    auto v= args[i]->eval(e);
    e->setValue(p->name(), cast(v, p->type()));
  }

  auto r= fs->body()->eval(e);
  e->popFrame();
  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(d::DslToken pname, d::DslAst block) : Ast(pname) {
  this->block=block;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Program::name() const {
  return token()->getLiteralAsStr();
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
d::DslAst variable(CrenshawParser* ps) {
  return new Var(ps->eat(d::T_IDENT));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst factor(CrenshawParser* ps) {
  auto t= ps->token();
  d::DslAst res;
  switch (t.ptr()->type()) {
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
      res= (ps->eat(), b_expr(ps));
      ps->eat(d::T_RPAREN);
      break;
    default:
      res= variable(ps);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst term(CrenshawParser* ps) {
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV};
  auto res= factor(ps);
  while (s__contains(ops,ps->cur())) {
    res = d::DslAst(new BinOp(res, ps->eat(), factor(ps)));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst expr(CrenshawParser* ps) {
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  auto res= term(ps);
  while (s__contains(ops,ps->cur())) {
    res= d::DslAst(new BinOp(res, ps->eat(), term(ps)));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst relation(CrenshawParser* ps) {
  static std::set<int> ops1 { d::T_GT, d::T_LT, T_GTEQ };
  static std::set<int> ops2 { T_LTEQ, T_EQUALS, T_NOTEQ };
  auto res = expr(ps);
  while (s__contains(ops1, ps->cur()) ||
         s__contains(ops2, ps->cur()) ) {
    res= d::DslAst(new RelationOp(res, ps->eat(), expr(ps)));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst b_factor(CrenshawParser* ps) {
  return relation(ps);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst not_factor(CrenshawParser* ps) {
  if (ps->isCur(T_NOT)) {
    ps->eat();
    return d::DslAst(new NotFactor(ps->getEthereal(), b_factor(ps)));
  } else {
    return b_factor(ps);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst b_term(CrenshawParser* ps) {
  AstVec res {not_factor(ps)};
  while (ps->isCur(T_AND)) {
    s__conj(res, not_factor(ps));
  }
  return new BoolTerm(ps->getEthereal(), res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst b_expr(CrenshawParser* ps) {
  static std::set<int> ops {T_OR, T_XOR};
  AstVec res {b_term(ps)};
  TokenVec ts;
  while (s__contains(ops, ps->cur())) {
    s__conj(ts, ps->token());
    ps->eat();
    s__conj(res, b_term(ps));
  }
  return new BoolExpr(ps->getEthereal(), res, ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst type_spec(CrenshawParser* ps) {
  auto t = ps->token();
  switch (t.ptr()->type()) {
    case T_STR:
    case T_INT:
    case T_REAL: ps->eat(); break;
    default:
      RAISE(d::SyntaxError,
            "Unknown token %d near line %d(%d).\n",
            t.ptr()->type(),
            s__cast(Token,t.ptr())->srcInfo().first,
            s__cast(Token,t.ptr())->srcInfo().second);
  }
  return new Type(t);
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void variable_declaration(CrenshawParser* ps, AstVec& out) {
  AstVec vars { d::DslAst(new Var(ps->eat(d::T_IDENT))) };
  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(vars, new Var(ps->eat(d::T_IDENT)));
  }
  auto type = (ps->eat(d::T_COLON), type_spec(ps));
  for (auto &x : vars) {
    s__conj(out, new VarDecl(x, type));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst assignment_statement(CrenshawParser* ps) {
  auto left = variable(ps);
  auto t= ps->eat(T_ASSIGN);
  auto right = expr(ps);
  return new Assignment(left, t, right);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst empty(CrenshawParser* ps) {
  return new NoOp();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst proccall_statement(CrenshawParser* ps) {
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
          new ProcedureCall(proc_name, pms));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RepeatUntil::RepeatUntil(d::DslToken t, d::DslAst e, d::DslAst c) : Ast(t) {
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
  code->visit(a);
  cond->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr RepeatUntil::name() const {
  return token()->impl().txt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IfThenElse::IfThenElse(d::DslToken t, d::DslAst cond, d::DslAst then, d::DslAst elze)
  : Ast(t) {
  this->cond=cond;
  this->then=then;
  this->elze=elze;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IfThenElse::IfThenElse(d::DslToken t, d::DslAst cond, d::DslAst then) : Ast(t) {
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
  cond->visit(a);
  then->visit(a);
  if (elze.isSome()) elze->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr IfThenElse::name() const {
  return token()->impl().txt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ForLoop::ForLoop(d::DslToken t, d::DslAst v, d::DslAst i, d::DslAst e, d::DslAst code) : Ast(t) {
  var_node=v;
  init=i;
  term=e;
  this->code= code;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ForLoop::eval(d::IEvaluator* e) {
  auto tn= CAST(Var,var_node)->type_symbol;
  auto vn= P_AST(var_node)->name();
  d::DslValue ret;

  e->setValueEx(vn, cast(init->eval(e),tn));
  //::printf("ready\n");
  while (1) {
    auto z= toInt(term->eval(e));
    auto i= toInt(e->getValue(vn));
    //::printf("z = %ld, i= %ld\n", z ,i);
    if (z >= i) {
      ret= code->eval(e);
      e->setValueEx(vn, cast(d::DslValue(new SInt(i+1)), tn));
    } else {
      break;
    }
  }

  return ret;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ForLoop::visit(d::IAnalyzer* a) {
  auto vn = CAST(Var,var_node);
  auto s= a->search(vn->name());
  if (s.isNull()) {
    throw d::SyntaxError("Missing type in forloop decl.");
  }
  vn->visit(a);
  vn->type_symbol = CAST(d::VarSymbol,s)->type();
  init->visit(a);
  term->visit(a);
  code->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ForLoop::name() const {
  return token()->impl().txt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
WhileLoop::WhileLoop(d::DslToken t, d::DslAst e, d::DslAst c) : Ast(t) {
  cond=e;
  code= c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue WhileLoop::eval(d::IEvaluator* e) {
  auto c= cond->eval(e);
  d::DslValue ret;
  while (toBool(c)) {
    ret=code->eval(e);
    c = cond->eval(e);
  }
  return ret;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void WhileLoop::visit(d::IAnalyzer* a) {
  cond->visit(a);
  code->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr WhileLoop::name() const {
  return token()->impl().txt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue VarInput::eval(d::IEvaluator* e) {
  auto e_ = s__cast(EvaluatorAPI,e);
  auto s= type_symbol->name();
  d::DslValue res;

  if (s == "INTEGER") {
    res = d::DslValue(new SInt( e_->readInt()));
  } else if (s == "REAL") {
    res = d::DslValue(new SReal( e_->readFloat()));
  } else if (s == "STRING") {
    res = d::DslValue(new SStr( e_->readString()));
  }

  return e->setValueEx(name(), res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarInput::visit(d::IAnalyzer* a) {
  if (auto s = a->search(name()); s.isSome()) {
    type_symbol= CAST(d::VarSymbol,s)->type();
  } else {
    RAISE(d::SemanticError,
          "Unknown var %s near line %d(%d).\n",
          C_STR(name()),
          token()->srcInfo().first, token()->srcInfo().second);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Read::Read(d::DslToken t, d::DslAst v) : Ast(t) {
  var_node=v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Read::eval(d::IEvaluator* e) {
  auto r= var_node->eval(e);
  if (token()->type() == T_READLN) {
    s__cast(EvaluatorAPI,e)->writeln();
  }
  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Read::visit(d::IAnalyzer* a) {
  var_node->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Read::name() const {
  return token()->impl().txt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Write::Write(d::DslToken t, const AstVec& ts) : Ast(t) {
  s__ccat(terms,ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Write::eval(d::IEvaluator* e) {
  auto e_ = s__cast(EvaluatorAPI,e);
  stdstr out;
  for (auto& x : terms) {
    out += x->eval(e)->pr_str();
  }
  e_->writeString(out);
  if (token()->type() == T_WRITELN) {
    e_->writeln();
  }
  return nullptr;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Write::visit(d::IAnalyzer* a) {
  for (auto& x : terms) {
    x->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Write::name() const {
  return token()->impl().txt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst doRepeat(CrenshawParser* ps) {
  auto w= ps->eat(T_REPEAT);
  auto c = compound_statement(ps,false);
  ps->eat(T_UNTIL);
  ps->eat(d::T_LPAREN);
  auto e = b_expr(ps);
  ps->eat(d::T_RPAREN);
  return new RepeatUntil(w, e, c);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst doIf(CrenshawParser* ps) {
  auto w= ps->eat(T_IF);
  ps->eat(d::T_LPAREN);
  auto c = b_expr(ps);
  ps->eat(d::T_RPAREN);
  auto t = compound_statement(ps,false);
  d::DslAst e;
  if (ps->isCur(T_ELSE)) {
    ps->eat();
    e = compound_statement(ps,false);
  }
  ps->eat(T_ENDIF);
  return new IfThenElse(w, c, t, e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst doFor(CrenshawParser* ps) {
  auto w= ps->eat(T_FOR);
  auto v = new Var(ps->eat(d::T_IDENT));
  ps->eat(T_ASSIGN);
  auto i = expr(ps);
  auto e = expr(ps);
  auto c = compound_statement(ps,false);
  ps->eat(T_ENDFOR);
  return new ForLoop(w, v, i, e, c);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst doWhile(CrenshawParser* ps) {
  auto w= ps->eat(T_WHILE);
  ps->eat(d::T_LPAREN);
  auto e = b_expr(ps);
  ps->eat(d::T_RPAREN);
  auto c = compound_statement(ps,false);
  ps->eat(T_ENDWHILE);
  return new WhileLoop(w, e, c);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst doWrite(CrenshawParser* ps, bool nl) {
  d::DslToken t;
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
  return new Write(t, ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst doRead(CrenshawParser* ps, bool nl) {
  d::DslToken t;
  if (nl) t= ps->eat(T_READLN); else t= ps->eat(T_READ);
  ps->eat(d::T_LPAREN);
  auto v= new VarInput(ps->eat(d::T_IDENT));
  ps->eat(d::T_RPAREN);
  return new Read(t, v);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst statement(CrenshawParser* ps) {
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
      return empty(ps);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void statement_list(CrenshawParser* ps, AstVec& results) {

  auto s= statement(ps);

  if (!(P_AST(s)->name() == "709394")) {
    s__conj(results,s);
  }

  while (ps->isCur(d::T_SEMI)) {
    ps->eat();
    s= statement(ps);
    if (!(P_AST(s)->name() == "709394")) {
      s__conj(results,s);
    }
  }

  if (ps->isCur(d::T_IDENT)) {
    RAISE(d::SyntaxError,
        "Unexpected token `%s`\n",
        P_TOKE(ps)->getLiteralAsStr().c_str());
  }

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst compound_statement(CrenshawParser* ps, bool beginend) {
  if (beginend) {ps->eat(T_BEGIN);}
  AstVec nodes;
  statement_list(ps, nodes);
  if (beginend) {ps->eat(T_END);}
  auto root= new Compound(ps->getEthereal());
  for (auto& node : nodes) {
    s__conj(root->statements,node);
  }
  return root;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void formal_parameters(CrenshawParser* ps, AstVec& pnodes) {

  TokenVec param_tokens { ps->eat(d::T_IDENT) };

  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    s__conj(param_tokens, ps->eat(d::T_IDENT));
  }

  auto type_node = (ps->eat(d::T_COLON),type_spec(ps));

  for (auto& t : param_tokens) {
    //::printf("param toke= %s\n", t->getLiteralAsStr());
    s__conj(pnodes, new VarDecl(new Var(t), type_node));
  }

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void formal_parameter_list(CrenshawParser* ps, AstVec& out) {

  if (!ps->isCur(d::T_IDENT)) {
    return;
  }

  formal_parameters(ps, out);

  while (ps->isCur(d::T_SEMI)) {
    AstVec pms;
    ps->eat();
    formal_parameters(ps,pms);
    s__ccat(out, pms);
  }

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst procedure_declaration(CrenshawParser* ps) {

  auto proc_name = (ps->eat(T_PROCEDURE), ps->eat(d::T_IDENT));
  AstVec params;

  if (ps->isCur(d::T_LPAREN)) {
    ps->eat();
    formal_parameter_list(ps, params);
    ps->eat(d::T_RPAREN);
  }

  auto decl = new ProcedureDecl(proc_name,
                                params,
                                (ps->eat(d::T_SEMI), block(ps)));
  //::printf("proc name=%s\n", decl->name().c_str());
  return (ps->eat(d::T_SEMI), decl);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void declarations(CrenshawParser* ps, AstVec& out) {

  if (ps->isCur(T_VAR)) {
    ps->eat();
    while (ps->isCur(d::T_IDENT)) {
      AstVec vs;
      variable_declaration(ps, vs);
      s__ccat(out,vs);
      ps->eat(d::T_SEMI);
    }
  }

  while (ps->isCur(T_PROCEDURE)) {
    s__conj(out,procedure_declaration(ps));
  }

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst block(CrenshawParser* ps) {
  AstVec decls;

  declarations(ps, decls);
  return new Block(ps->getEthereal(),
                   decls, compound_statement(ps, true));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst program(CrenshawParser* ps) {
  auto prog_name = (ps->eat(T_PROGRAM),ps->eat(d::T_IDENT));
  auto prog = new Program(prog_name,
                          (ps->eat(d::T_SEMI),block(ps)));
  //::printf("program = %s\n", prog->name().c_str());
  return (ps->eat(d::T_DOT), prog);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CrenshawParser::~CrenshawParser() {
  DEL_PTR(lex);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CrenshawParser::CrenshawParser(const char* src) {
  lex = new Lexer(src);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst CrenshawParser::parse() {
  return program(this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken CrenshawParser::getEthereal() {
  return new Token(d::T_ETHEREAL, "",
                   s__pair(int,int,lex->ctx().line, lex->ctx().col));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int CrenshawParser::cur() {
  return lex->ctx().cur->type();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar CrenshawParser::peek() {
  return d::peek(lex->ctx());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool CrenshawParser::isEof() const {
  return lex->ctx().cur->type() == d::T_EOF;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool CrenshawParser::isCur(int type) {
  return lex->ctx().cur->type() == type;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken CrenshawParser::token() {
  return lex->ctx().cur;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken CrenshawParser::eat() {
  auto t= token();
  lex->ctx().cur = lex->getNextToken();
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken CrenshawParser::eat(int wanted) {
  auto t= token();
  if (t->type() != wanted) {
    RAISE(d::SyntaxError,
          "Expected token %s, found token %s near line %d(%d).\n",
          Token::typeToString(wanted).c_str(),
          C_STR(t->pr_str()),
          t->srcInfo().first,
          t->srcInfo().second);
  }
  lex->ctx().cur= lex->getNextToken();
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
