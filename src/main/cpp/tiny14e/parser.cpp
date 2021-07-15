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
 * Copyright © 2013-2021, Kenneth Leung. All rights reserved. */

#include <typeinfo>
#include "types.h"
#include "parser.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e{
namespace a= czlab::aeon;
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst compound_statement(CrenshawParser*, bool);
d::DAst block(CrenshawParser*);
d::DAst expr(CrenshawParser*);
d::DAst b_expr(CrenshawParser*);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr toStr(d::DValue e){ return e ? e->pr_str(1) : ""; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue boolFalse(){ return d::Number::make(0); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue boolTrue(){ return d::Number::make(1); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double toReal(d::DValue e){
  auto n= vcast<d::Number>(e);
  return n ? n->getFloat() : 0;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong toInt(d::DValue e){
  auto n= vcast<d::Number>(e);
  return n ? n->getInt() : 0;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue cast(d::DValue v, d::DSymbol t){
  auto s = t.get()->name();

  if(s == "INTEGER")
    return d::Number::make(toInt(v));

  if(s== "REAL")
    return d::Number::make(toReal(v));

  if(s== "STRING")
    return d::String::make(toStr(v));

  E_SEMANTIC("Unknown type %s", C_STR(s));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool toBool(d::DValue e){ return toInt(e) != 0L; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Ast::name() const{
  return DCAST(d::Token,_token)->getStr(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BinOp::BinOp(d::DAst left, d::DToken op, d::DAst right)
  : Ast(op), lhs(left), rhs(right){
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(d::IAnalyzer* a){
  lhs->visit(a);
  rhs->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BinOp::name() const{ return "BinOp"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BinOp::eval(d::IEvaluator* e){
  auto _A=token()->addr();
  auto lf = lhs->eval(e);
  auto rt = rhs->eval(e);
  auto plf = vcast<d::Number>(lf,_A);
  auto prt = vcast<d::Number>(rt,_A);
  bool ints = plf->isInt() && prt->isInt();
  llong L=0;
  double R=0;
  switch(token()->type()){
    case d::T_MINUS:
      if(ints)
        L = plf->getInt() - prt->getInt();
      else
        R = plf->getFloat() - prt->getFloat();
    break;
    case d::T_PLUS:
      if(ints)
        L = plf->getInt() + prt->getInt();
      else
        R = plf->getFloat() + prt->getFloat();
    break;
    case d::T_MULT:
      if(ints)
        L = plf->getInt() * prt->getInt();
      else
        R = plf->getFloat() * prt->getFloat();
    break;
    case T_INT_DIV:
      ASSERT(ints, "Expecting integers, got %s", "floats");
      ASSERT(!prt->isZero(),
             "Div by int-zero error, %s", C_STR(prt->pr_str()));
      L = plf->getInt() / prt->getInt();
    break;
    case d::T_DIV:
      ASSERT(!prt->isZero(),
             "Div by zero error, %s", C_STR(prt->pr_str()));
      ints=false;
      R = plf->getFloat() / prt->getFloat();
    break;
    default:
      E_SEMANTIC("Bad op near %s", d::pr_addr( token()->addr()).c_str());
  }
  return ints ? d::Number::make(L) : d::Number::make(R);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String::String(d::DToken t) : Ast(t){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void String::visit(d::IAnalyzer* a){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr String::name() const{ return "string"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue String::eval(d::IEvaluator* e){
  return d::String::make(token()->getStr());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(d::DToken t) : Ast(t){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(d::IAnalyzer* a){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Num::name() const
{
  switch(token()->type()){
    case d::T_INT: return "integer";
    case d::T_REAL: return "real";
    default:
      E_SYNTAX("Bad number near %s", d::pr_addr( token()->addr()).c_str()); } }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Num::eval(d::IEvaluator* e)
{
  switch(token()->type()){
    case d::T_INT:
      return d::Number::make(token()->getInt());
    case d::T_REAL:
      return d::Number::make(token()->getFloat());
    default:
      E_SYNTAX("Bad number near %s", d::pr_addr( token()->addr()).c_str()); } }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(d::DToken t, d::DAst expr) : Ast(t), expr(expr){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr UnaryOp::name() const
{
  switch(token()->type()){
    case d::T_MINUS: return "-";
    case d::T_PLUS: return "+";
    default:
      E_SYNTAX("Bad op near %s", d::pr_addr( token()->addr()).c_str()); } }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(d::IAnalyzer* a){ expr->visit(a); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue UnaryOp::eval(d::IEvaluator* e){
  auto _A=token()->addr();
  auto r = expr->eval(e);
  auto p = vcast<d::Number>(r,_A);

  if(token()->type() == d::T_MINUS)
    r= p->isInt() ? d::Number::make(- p->getInt())
                  : d::Number::make(- p->getFloat());

  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound(d::DToken t) : Ast(t){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Compound::name() const{ return "Compound"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Compound::visit(d::IAnalyzer* a){
  for(auto& it : statements)
    it->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Compound::eval(d::IEvaluator* e){
  d::DValue ret;
  for(auto& it : statements){
    ret=it->eval(e);
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment::Assignment(d::DAst left, d::DToken op, d::DAst right)
  : Ast(op), lhs(left), rhs(right){
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Assignment::name() const{ return ":="; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a){
  auto pl= DCAST(Ast,lhs);
  auto pn=pl->name();
  auto s= a->search(pn);
  if(!s){
    E_SEMANTIC("Unbound var %s near %s",
        C_STR(pn), d::pr_addr( pl->token()->addr()).c_str()); }
  s__cast(Var,pl)->type_symbol= s->type();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Assignment::eval(d::IEvaluator* e){
  auto pl= DCAST(Ast,lhs);
  auto v = pl->token()->getStr();
  auto t= s__cast(Var,pl)->type_symbol;
  auto r= rhs->eval(e);
  //DEBUG("Assigning value %s to %s.", C_STR(r->pr_str()), C_STR(v));
  return e->setValueEx(v, cast(r,t));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(d::DToken t) : Ast(t){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Var::name() const{
  return token()->getStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(d::IAnalyzer* a){
  auto n = name();
  if(auto x= a->search(n); !x)
  {
    E_SEMANTIC("Unbound var %s near %s",
               C_STR(n), d::pr_addr( token()->addr()).c_str()); } }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Var::eval(d::IEvaluator* e){
  return e->getValue( name());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type::Type(d::DToken token) : Ast(token){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Type::name() const{
  return token()->getStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Type::visit(d::IAnalyzer* a){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Type::eval(d::IEvaluator* e){ return P_NIL; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VarDecl::VarDecl(d::DAst var, d::DAst type)
  : Ast(DCAST(Var,var)->token()), var_node(var), type_node(type){
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr VarDecl::name() const{ return DCAST(Var,var_node)->name(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarDecl::visit(d::IAnalyzer* a){
  auto tname = DCAST(Ast,type_node)->name();
  auto vname = DCAST(Ast,var_node)->name();
  auto tsym = a->search(tname);
  auto i= token()->addr();

  if(!tsym)
    E_SEMANTIC("Unknown type %s near %s",
               C_STR(tname), d::pr_addr(i).c_str());

  if(auto x = a->find(vname); x)
  {
    E_SEMANTIC("Duplicate var %s near %s",
               C_STR(vname), d::pr_addr(i).c_str()); }

  DCAST(Var,var_node)->type_symbol= tsym;
  a->define(d::VarSymbol::make(vname, tsym));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue VarDecl::eval(d::IEvaluator* e){
  auto t= DCAST(Var,var_node)->type_symbol;
  d::DValue v;
  //std::cout << "var=== " << name() << "\n";
  return e->setValue(name(), cast(v, t));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BoolExpr::BoolExpr(d::DToken k, const d::AstVec& ts, const d::TokenVec& ops) : Ast(k){
  s__ccat(this->ops, ops);
  s__ccat(this->terms, ts);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BoolExpr::eval(d::IEvaluator* e){
  auto z1= terms.size();
  auto t1= ops.size();
  ASSERT(z1 == (t1+1),
         "Malformed expr, %d terms but %d ops.", (int)terms.size(),(int)ops.size());
  auto i=0;
  auto lhs= terms[0]->eval(e);
  auto res= toBool(lhs);
  if(z1==1){ return lhs; }
  while(i < t1){
    auto t= ops[i];
    if(t->type() == T_OR && res){
      return boolTrue();
    }
    auto rhs= terms[i+1]->eval(e);
    if(t->type() == T_XOR){
      res= (res != toBool(rhs));
    }else if(toBool(rhs)){
      res=true;
    }
    ++i;
  }
  return res ? boolTrue() : boolFalse();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BoolExpr::visit(d::IAnalyzer* a){
  for(auto& x : terms)
    x->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolExpr::name() const{ return "BoolExpr"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BoolTerm::BoolTerm(d::DToken k, const d::AstVec& ts) : Ast(k){
  s__ccat(terms, ts);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolTerm::name() const{ return "bterm"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BoolTerm::visit(d::IAnalyzer* a){
  for(auto& x : terms)
    x->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BoolTerm::eval(d::IEvaluator* e){
  ASSERT(terms.size() > 0,
         "Malformed empty expr, got %d terms.", (int)terms.size());
  auto i=0;
  auto z=terms.size();
  auto lhs = terms[i]->eval(e);
  auto res= toBool(lhs);
  if(z==1){ return lhs; }
  if(!res){ return boolFalse(); }
  ++i;
  while(i < z){
    auto _t = terms[i]->eval(e);
    auto rhs = toBool(_t);
    res= (res && rhs);
    if(res) return boolTrue();
    ++i;
  }
  return boolFalse();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NotFactor::NotFactor(d::DToken t, d::DAst e) : Ast(t){
  expr=e;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue NotFactor::eval(d::IEvaluator* e){
  auto r= expr->eval(e);
  return !toBool(r) ? boolTrue() : boolFalse();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void NotFactor::visit(d::IAnalyzer* a){
  expr->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr NotFactor::name() const{ return "notfactor"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RelationOp::RelationOp(d::DAst left, d::DToken op, d::DAst right) : Ast(op){
  lhs=left;
  rhs=right;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//stdstr RelationOp::name() const { return CAST(Token,token())->impl().txt; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void RelationOp::visit(d::IAnalyzer* a){
  lhs->visit(a);
  rhs->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue RelationOp::eval(d::IEvaluator* e){
  auto _A=token()->addr();
  auto lf = lhs->eval(e);
  auto rt = rhs->eval(e);
  auto l = vcast<d::Number>(lf,_A)->getFloat();
  auto r = vcast<d::Number>(rt,_A)->getFloat();
  auto res=false;
  switch(token()->type()){
    case d::T_GT: res = l > r; break;
    case d::T_LT: res = l < r; break;
    case T_GTEQ: res = l >= r; break;
    case T_LTEQ: res = l <= r; break;
    case T_EQUALS: res = l == r; break;
    case T_NOTEQ: res = l != r; break;
    default:
      E_SEMANTIC("Unknown op near %s", d::pr_addr( token()->addr()).c_str());
  }
  return res ? boolTrue() : boolFalse();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block::Block(d::DToken k,const d::AstVec& decls, d::DAst compound) : Ast(k){
  this->compound = compound;
  s__ccat(declarations, decls);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Block::name() const{ return "Block"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Block::visit(d::IAnalyzer* a){
  for(auto& x : declarations){
    x->visit(a);
  }
  compound->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Block::eval(d::IEvaluator* e){
  for(auto& x : declarations){
    x->eval(e);
  }
  return compound->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl::ProcedureDecl(d::DToken proc_name,
                             const d::AstVec& pms, d::DAst block_node)
  : Ast(proc_name){
  block=block_node;
  s__ccat(params, pms);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureDecl::visit(d::IAnalyzer* a){
  auto fs= d::FnSymbol::make( name());
  auto fp= DCAST(d::FnSymbol,fs);
  a->define(fs);
  a->pushScope(name());

  for(auto& p : params){
    auto pm = DCAST(VarDecl,p);
    auto tn = DCAST(Ast,pm->type_node)->name();
    auto pn = DCAST(Ast,pm->var_node)->name();
    auto pt = a->search(tn);
    auto v = d::VarSymbol::make(pn, pt);
    a->define(v);
    s__conj(fp->params(),v);
  }
  block->visit(a);
  a->popScope();
  fp->setBody(this->block);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ProcedureDecl::name() const{
  return token()->getStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue ProcedureDecl::eval(d::IEvaluator* e){
  return P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall::ProcedureCall(d::DToken proc_name, const d::AstVec& p)
  : Ast(proc_name){
  s__ccat(args, p);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ProcedureCall::name() const{
  return token()->getStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureCall::visit(d::IAnalyzer* a)
{
  for(auto& p : args)
    p->visit(a);
  //get the corresponding symbol
  if(auto x = a->search( name()); x){
    proc_symbol = x;
  }else{
    E_SEMANTIC("Unknown proc %s near %s",
               C_STR(name()), d::pr_addr( token()->addr()).c_str()); } }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue ProcedureCall::eval(d::IEvaluator* e){
  auto fs= DCAST(d::FnSymbol,proc_symbol);
  auto fz= fs->params().size();
  auto az= args.size();
  ASSERT(fz == az,
         "Mismatch sizes, params= %d, but args= %d.", (int)fz, (int)az);
  e->pushFrame(name());
  for(auto i=0; i < args.size(); ++i){
    auto p= DCAST(d::VarSymbol,fs->params()[i]);
    auto v= args[i]->eval(e);
    e->setValue(p->name(), cast(v, p->type()));
  }
  auto r= fs->body()->eval(e);
  e->popFrame();
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(d::DToken pname, d::DAst block) : Ast(pname){
  this->block=block;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Program::name() const{
  return token()->getStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(d::IAnalyzer* a){
  block->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Program::eval(d::IEvaluator* e){
  return block->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst variable(CrenshawParser* ps){
  return Var::make(ps->eat(d::T_IDENT));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst factor(CrenshawParser* ps){
  auto t= ps->token();
  d::DAst res;
  switch(t->type()){
    case d::T_PLUS:
      res= (ps->eat(), UnaryOp::make(t, factor(ps)));
      break;
    case d::T_MINUS:
      res= (ps->eat(), UnaryOp::make(t, factor(ps)));
      break;
    case d::T_REAL:
    case d::T_INT:
      res= (ps->eat(), Num::make(t));
      break;
    case d::T_STRING:
      res= (ps->eat(), String::make(t));
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
d::DAst term(CrenshawParser* ps){
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV};
  auto res= factor(ps);
  while(s__contains(ops,ps->cur()))
    res = BinOp::make(res, ps->eat(), factor(ps));
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst expr(CrenshawParser* ps){
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  auto res= term(ps);
  while(s__contains(ops,ps->cur()))
    res= BinOp::make(res, ps->eat(), term(ps));
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst relation(CrenshawParser* ps){
  static std::set<int> ops1 { d::T_GT, d::T_LT, T_GTEQ };
  static std::set<int> ops2 { T_LTEQ, T_EQUALS, T_NOTEQ };
  auto res = expr(ps);
  while(s__contains(ops1, ps->cur()) ||
        s__contains(ops2, ps->cur()) )
    res= RelationOp::make(res, ps->eat(), expr(ps));
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst b_factor(CrenshawParser* ps){
  return relation(ps);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst not_factor(CrenshawParser* ps)
{
  if(!ps->isCur(T_NOT))
    return b_factor(ps);
  else
    return NotFactor::make(ps->eat(), b_factor(ps)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst b_term(CrenshawParser* ps){
  auto k=ps->token();
  d::AstVec res {not_factor(ps)};
  while(ps->isCur(T_AND)){
    ps->eat();
    s__conj(res, not_factor(ps));
  }
  return BoolTerm::make(k, res);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst b_expr(CrenshawParser* ps){
  static std::set<int> ops {T_OR, T_XOR};
  auto k= ps->token();
  d::AstVec res {b_term(ps)};
  d::TokenVec ts;
  while(s__contains(ops, ps->cur())){
    s__conj(ts, ps->token());
    ps->eat();
    s__conj(res, b_term(ps));
  }
  return BoolExpr::make(k, res, ts);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst type_spec(CrenshawParser* ps){
  auto t = ps->token();
  switch(t->type()){
    case T_STR:
    case T_INT:
    case T_REAL: ps->eat(); break;
    default:
      E_SYNTAX("Unknown token %d near %s",
               t->type(), d::pr_addr( DCAST(d::Token,t)->addr()).c_str());
  }
  return Type::make(t);
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void variable_declaration(CrenshawParser* ps, d::AstVec& out){
  d::AstVec vars { Var::make(ps->eat(d::T_IDENT)) };
  while(ps->isCur(d::T_COMMA))
  {
    ps->eat();
    s__conj(vars, Var::make(ps->eat(d::T_IDENT))); }
  auto type = (ps->eat(d::T_COLON), type_spec(ps));
  for(auto &x : vars)
    s__conj(out, VarDecl::make(x, type));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst assignment_statement(CrenshawParser* ps){
  auto left = variable(ps);
  auto t= ps->eat(T_ASSIGN);
  auto right = expr(ps);
  return Assignment::make(left, t, right);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst empty(CrenshawParser* ps){
  return NoOp::make();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst proccall_statement(CrenshawParser* ps)
{
  auto proc_name = ps->eat(d::T_IDENT);
  ps->eat(d::T_LPAREN);
  d::AstVec pms;

  if(!ps->isCur(d::T_RPAREN)){
    s__conj(pms, expr(ps));
  }

  while(ps->isCur(d::T_COMMA)){
    ps->eat();
    s__conj(pms,expr(ps));
  }

  return (ps->eat(d::T_RPAREN), ProcedureCall::make(proc_name, pms)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RepeatUntil::RepeatUntil(d::DToken t, d::DAst e, d::DAst c) : Ast(t){
  cond=e;
  code= c;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue RepeatUntil::eval(d::IEvaluator* e){
  auto ret= code->eval(e);
  auto c= cond->eval(e);
  auto i = 1;
  while(toBool(c)){
    ret=code->eval(e);
    c = cond->eval(e);
    //DEBUG("looping.....%d.", i);
    ++i;
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void RepeatUntil::visit(d::IAnalyzer* a){
  code->visit(a);
  cond->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//stdstr RepeatUntil::name() const { return CAST(Token,token())->impl().txt; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IfThenElse::IfThenElse(d::DToken t, d::DAst cond, d::DAst then, d::DAst elze)
  : Ast(t){
  this->cond=cond;
  this->then=then;
  this->elze=elze;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IfThenElse::IfThenElse(d::DToken t, d::DAst cond, d::DAst then) : Ast(t){
  this->cond=cond;
  this->then=then;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue IfThenElse::eval(d::IEvaluator* e){
  auto c= cond->eval(e);
  d::DValue ret;

  if(toBool(c)){
    ret= then->eval(e);
  }else if(elze){
    ret= elze->eval(e);
  }

  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void IfThenElse::visit(d::IAnalyzer* a){
  cond->visit(a);
  then->visit(a);
  if(elze) elze->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//stdstr IfThenElse::name() const { return CAST(Token,token())->impl().txt; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ForLoop::ForLoop(d::DToken t, d::DAst v, d::DAst i, d::DAst e, d::DAst code) : Ast(t){
  var_node=v;
  init=i;
  term=e;
  this->code= code;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue ForLoop::eval(d::IEvaluator* e){
  auto tn= DCAST(Var,var_node)->type_symbol;
  auto vn= DCAST(Var,var_node)->name();
  d::DValue ret;

  auto _i= init->eval(e);
  e->setValueEx(vn, cast(_i,tn));
  //::printf("ready\n");
  while(1){
    auto _t= term->eval(e);
    auto z= toInt(_t);
    auto i= toInt(e->getValue(vn));
    //::printf("z = %ld, i= %ld\n", z ,i);
    if(z >= i){
      ret= code->eval(e);
      auto xxx= d::Number::make(i+1);
      e->setValueEx(vn, cast(xxx, tn));
    }else{
      break;
    }
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ForLoop::visit(d::IAnalyzer* a){
  auto vn = DCAST(Var,var_node);
  auto s= a->search(vn->name());
  if(!s)
    RAISE(d::SyntaxError,
          "Missing type in forloop decl, var = %s.", C_STR(vn->name()));

  vn->visit(a);
  vn->type_symbol = DCAST(d::VarSymbol,s)->type();
  init->visit(a);
  term->visit(a);
  code->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//stdstr ForLoop::name() const { return CAST(Token,token())->impl().txt; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
WhileLoop::WhileLoop(d::DToken t, d::DAst e, d::DAst c) : Ast(t){
  cond=e;
  code= c;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue WhileLoop::eval(d::IEvaluator* e){
  auto c= cond->eval(e);
  d::DValue ret;
  while(toBool(c)){
    ret=code->eval(e);
    c = cond->eval(e);
  }
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void WhileLoop::visit(d::IAnalyzer* a){
  cond->visit(a);
  code->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//stdstr WhileLoop::name() const { return CAST(Token,token())->impl().txt; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue VarInput::eval(d::IEvaluator* e){
  auto e_ = s__cast(EvaluatorAPI,e);
  auto s= type_symbol->name();
  d::DValue res;

  if(s == "INTEGER"){
    res = d::Number::make( e_->readInt());
  }else if(s == "REAL"){
    res = d::Number::make( e_->readFloat());
  }else if(s == "STRING"){
    res = d::String::make( e_->readString());
  }

  return e->setValueEx(name(), res);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarInput::visit(d::IAnalyzer* a){
  if(auto s = a->search(name()); s){
    type_symbol= DCAST(d::VarSymbol,s)->type();
  }else{
    E_SEMANTIC("Unknown var %s near %s",
               C_STR(name()), d::pr_addr( DCAST(d::Token,token())->addr()).c_str()); } }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Read::Read(d::DToken t, d::DAst v) : Ast(t){
  var_node=v;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Read::eval(d::IEvaluator* e){
  auto r= var_node->eval(e);
  if(token()->type() == T_READLN)
    s__cast(EvaluatorAPI,e)->writeln();
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Read::visit(d::IAnalyzer* a){
  var_node->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//stdstr Read::name() const { return CAST(Token,token())->impl().txt; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Write::Write(d::DToken t, const d::AstVec& ts) : Ast(t){
  s__ccat(terms,ts);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Write::eval(d::IEvaluator* e){
  auto e_ = s__cast(EvaluatorAPI,e);
  stdstr out;
  for(auto& x : terms)
    out += x->eval(e)->pr_str();

  e_->writeString(out);
  if(token()->type() == T_WRITELN){
    e_->writeln();
  }
  return P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Write::visit(d::IAnalyzer* a){
  for(auto& x : terms)
    x->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//stdstr Write::name() const { return CAST(Token,token())->impl().txt; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst doRepeat(CrenshawParser* ps){
  auto w= ps->eat(T_REPEAT);
  auto c = compound_statement(ps,false);
  ps->eat(T_UNTIL);
  ps->eat(d::T_LPAREN);
  auto e = b_expr(ps);
  ps->eat(d::T_RPAREN);
  return RepeatUntil::make(w, e, c);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst doIf(CrenshawParser* ps){
  auto w= ps->eat(T_IF);
  ps->eat(d::T_LPAREN);
  auto c = b_expr(ps);
  ps->eat(d::T_RPAREN);
  auto t = compound_statement(ps,false);
  d::DAst e;
  if(ps->isCur(T_ELSE)){
    ps->eat();
    e = compound_statement(ps,false);
  }
  ps->eat(T_ENDIF);
  return IfThenElse::make(w, c, t, e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst doFor(CrenshawParser* ps){
  auto w= ps->eat(T_FOR);
  auto v = Var::make(ps->eat(d::T_IDENT));
  ps->eat(T_ASSIGN);
  auto i = expr(ps);
  auto e = expr(ps);
  auto c = compound_statement(ps,false);
  ps->eat(T_ENDFOR);
  return ForLoop::make(w, v, i, e, c);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst doWhile(CrenshawParser* ps){
  auto w= ps->eat(T_WHILE);
  ps->eat(d::T_LPAREN);
  auto e = b_expr(ps);
  ps->eat(d::T_RPAREN);
  auto c = compound_statement(ps,false);
  ps->eat(T_ENDWHILE);
  return WhileLoop::make(w, e, c);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst doWrite(CrenshawParser* ps, bool nl){
  d::DToken t;
  d::AstVec ts;
  if(nl) t= ps->eat(T_WRITELN); else t= ps->eat(T_WRITE);
  ps->eat(d::T_LPAREN);
  while(!ps->isCur(d::T_RPAREN)){
    s__conj(ts, expr(ps));
    if(!ps->isCur(d::T_RPAREN)){
      ps->eat(d::T_COMMA);
    }
  }
  ps->eat(d::T_RPAREN);
  return Write::make(t, ts);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst doRead(CrenshawParser* ps, bool nl){
  d::DToken t;
  if(nl) t= ps->eat(T_READLN); else t= ps->eat(T_READ);
  ps->eat(d::T_LPAREN);
  auto v= VarInput::make(ps->eat(d::T_IDENT));
  ps->eat(d::T_RPAREN);
  return Read::make(t, v);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst statement(CrenshawParser* ps){
  switch(ps->cur()){
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
void statement_list(CrenshawParser* ps, d::AstVec& results){

  auto s= statement(ps);

  if(!(DCAST(Ast,s)->name() == "709394"))
    s__conj(results,s);

  while(ps->isCur(d::T_SEMI)){
    ps->eat();
    s= statement(ps);
    if(!(DCAST(Ast,s)->name() == "709394"))
      s__conj(results,s);
  }

  if(ps->isCur(d::T_IDENT))
    RAISE(d::SyntaxError,
          "Unexpected token `%s`.", C_STR(ps->token()->getStr()));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst compound_statement(CrenshawParser* ps, bool beginend){
  auto X=ps->token();
  if(beginend){ps->eat(T_BEGIN);}
  d::AstVec nodes;
  statement_list(ps, nodes);
  if(beginend){ps->eat(T_END);}
  auto root= Compound::make(X);
  auto pr= DCAST(Compound,root);
  for(auto& node : nodes)
    s__conj(pr->statements,node);
  return root;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void formal_parameters(CrenshawParser* ps, d::AstVec& pnodes){

  d::TokenVec param_tokens { ps->eat(d::T_IDENT) };

  while(ps->isCur(d::T_COMMA))
  {
    ps->eat();
    s__conj(param_tokens, ps->eat(d::T_IDENT)); }

  auto type_node = (ps->eat(d::T_COLON),type_spec(ps));
  for(auto& t : param_tokens)
    //::printf("param toke= %s\n", t->getLiteralAsStr());
    s__conj(pnodes, VarDecl::make(Var::make(t), type_node));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void formal_parameter_list(CrenshawParser* ps, d::AstVec& out){

  if(!ps->isCur(d::T_IDENT)){
    return;
  }

  formal_parameters(ps, out);

  while(ps->isCur(d::T_SEMI)){
    d::AstVec pms;
    ps->eat();
    formal_parameters(ps,pms);
    s__ccat(out, pms);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst procedure_declaration(CrenshawParser* ps){

  auto proc_name = (ps->eat(T_PROCEDURE), ps->eat(d::T_IDENT));
  d::AstVec params;

  if(ps->isCur(d::T_LPAREN)){
    ps->eat();
    formal_parameter_list(ps, params);
    ps->eat(d::T_RPAREN);
  }

  auto decl = ProcedureDecl::make(proc_name,
                                  params,
                                  (ps->eat(d::T_SEMI), block(ps)));
  //std::cout << "proc name= " << CAST(Ast,decl)->name() << "\n";
  return (ps->eat(d::T_SEMI), decl);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void declarations(CrenshawParser* ps, d::AstVec& out){

  if(ps->isCur(T_VAR)){
    ps->eat();
    while(ps->isCur(d::T_IDENT)){
      d::AstVec vs;
      variable_declaration(ps, vs);
      s__ccat(out,vs);
      ps->eat(d::T_SEMI);
    }
  }

  while(ps->isCur(T_PROCEDURE))
    s__conj(out,procedure_declaration(ps));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst block(CrenshawParser* ps)
{
  auto k= ps->token();
  d::AstVec decls;
  declarations(ps, decls);
  return Block::make(k, decls, compound_statement(ps, true)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst program(CrenshawParser* ps){
  auto prog_name = (ps->eat(T_PROGRAM),ps->eat(d::T_IDENT));
  auto prog = Program::make(prog_name,
                            (ps->eat(d::T_SEMI),block(ps)));
  //::printf("program = %s\n", prog->name().c_str());
  return (ps->eat(d::T_DOT), prog);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CrenshawParser::~CrenshawParser(){
  DEL_PTR(lex);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CrenshawParser::CrenshawParser(const char* src){
  lex = new Lexer(src);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst CrenshawParser::parse(){
  return program(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int CrenshawParser::cur(){
  return lex->ctx().cur->type();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar CrenshawParser::peek(){
  return d::peek(lex->ctx());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool CrenshawParser::isEof() const{
  return lex->ctx().cur->type() == d::T_EOF;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool CrenshawParser::isCur(int type){
  return lex->ctx().cur->type() == type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken CrenshawParser::token(){
  return lex->ctx().cur;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken CrenshawParser::eat(){
  auto t= token();
  lex->ctx().cur = lex->getNextToken();
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken CrenshawParser::eat(int wanted){
  auto t= token();
  if(t->type() != wanted)
    E_SYNTAX("Expected token %s, found %s near %s",
             C_STR(typeToString(wanted)),
             C_STR(t->pr_str()),
             d::pr_addr( DCAST(d::Token,t)->addr()).c_str());
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
