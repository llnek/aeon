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

#include "parser.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::spi{
namespace a= czlab::aeon;
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define INT_VAL(x) d::Number::make( (llong)(x))
#define FLT_VAL(x) d::Number::make((double)x)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst compound_statement(SimplePascalParser*);
d::DAst block(SimplePascalParser*);
d::DAst expr(SimplePascalParser*);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr& m, d::DValue v, d::Addr k)
{
  RAISE(d::BadArg,
        "Wanted `%s`, got %s near %s",
        m.c_str(), v->pr_str(1).c_str(), d::pr_addr(k).c_str()); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue expected(cstdstr& m, d::DValue v)
{
  RAISE(d::BadArg,
        "Wanted `%s`, got %s", m.c_str(), v->pr_str(1).c_str()); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast(d::DToken t){
  token=t;
  ASSERT1(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BinOp::BinOp(d::DAst left, d::DToken op, d::DAst right)
  : Ast(op), lhs(left), rhs(right){
  _name= "BinOp";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(d::IAnalyzer* a){
  lhs->visit(a);
  rhs->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BinOp::eval(d::IEvaluator* e){
  auto lf = lhs->eval(e);
  auto rt = rhs->eval(e);
  auto lf_= DCAST(d::Number,lf);
  auto rt_= DCAST(d::Number,rt);
  double x, y, z;

  if(lf_->isInt())
    x= (double) lf_->getInt();
  else
    x= lf_->getFloat();

  if(rt_->isInt())
    y= (double) rt_->getInt();
  else
    y= rt_->getFloat();

  switch(token->type()){
    case d::T_MINUS: z = x - y; break;
    case d::T_PLUS: z = x + y; break;
    case d::T_MULT: z = x * y; break;

    case T_INT_DIV:
      z=x/y;
      return INT_VAL(z);

    case d::T_DIV:
      z = x / y;
      return FLT_VAL(z);

    default:
      E_SYNTAX("Bad binary-op near %s",
               d::pr_addr(token->addr()).c_str());
  }
  return lf_->isInt() && rt_->isInt() ? INT_VAL(z) : FLT_VAL(z);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String::String(d::DToken t) : Ast(t){ _name="string"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void String::visit(d::IAnalyzer* a){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue String::eval(d::IEvaluator* e){
  return d::String::make(token->getStr());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(d::DToken t) : Ast(t){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(d::IAnalyzer* a){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Num::name() const{
  switch(token->type()){
    case d::T_INT: return "integer";
    case d::T_REAL: return "real";
    default: return "Can't happen!";
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Num::eval(d::IEvaluator* e){
  switch(token->type()){
    case d::T_INT:
      return INT_VAL(token->getInt());
    case d::T_REAL:
      return FLT_VAL(token->getFloat());
    default:
      E_SYNTAX("Bad number near %s",
               d::pr_addr(token->addr()).c_str());
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(d::DToken t, d::DAst expr) : Ast(t), expr(expr){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr UnaryOp::name() const{
  switch(token->type()){
    case d::T_PLUS: return "+";
    case d::T_MINUS: return "-";
    default:
      E_SYNTAX("Bad unary-op near %s",
               d::pr_addr(token->addr()).c_str());
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(d::IAnalyzer* a){
  expr->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue UnaryOp::eval(d::IEvaluator* e){
  auto r = expr->eval(e);
  auto r_ = vcast<d::Number>(r);

  if(!r_)
    E_SYNTAX("Expected numeric type near %s",
             d::pr_addr(token->addr()).c_str());

  if(token->type() == d::T_MINUS){
    if(r_->isInt())
      return INT_VAL(- r_->getInt());
    else
      return FLT_VAL(- r_->getFloat());
  }

  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound(d::DToken t) : Ast(t){ _name="Compound"; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Compound::visit(d::IAnalyzer* a){
  for(auto& it : statements)
    it->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Compound::eval(d::IEvaluator* e){
  d::DValue ret;
  for(auto& it : statements)
    ret=it->eval(e);
  return ret;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment::Assignment(d::DAst left, d::DToken op, d::DAst right)
  : Ast(op), lhs(left), rhs(right){
  _name=":=";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a){
  auto lhs_ = DCAST(Ast,lhs);
  auto v = lhs_->token->getStr();
  if(auto s= a->search(v); !s){
    E_SYNTAX("Unbound var %s near %s",
             v.c_str(),
             d::pr_addr( lhs_->token->addr()).c_str());
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Assignment::eval(d::IEvaluator* e){
  auto lhs_= DCAST(Ast,lhs);
  auto v = lhs_->token->getStr();
  auto r= rhs->eval(e);
  //DEBUG("Assigning: %s := %s.", C_STR(v), C_STR(r->pr_str(1)));
  return e->setValueEx(v, r);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(d::DToken t) : Ast(t){
  _name= token->getStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(d::IAnalyzer* a){
  auto n = token->getStr();
  if(auto s= a->search(n); !s) {
    E_SYNTAX("Unbound var %s near %s",
             n.c_str(),
             d::pr_addr( token->addr()).c_str());
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Var::eval(d::IEvaluator* e){
  return e->getValue(token->getStr());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type::Type(d::DToken token) : Ast(token){
  _name= token->getStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Type::visit(d::IAnalyzer* a){ }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Type::eval(d::IEvaluator* e){
  return DVAL_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymTable::SymTable(cstdstr& n, d::DTable outer) : SymTable(n){
  enclosing=outer;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymTable::SymTable(cstdstr& n) : d::Table(n){
  insert(BuiltinTypeSymbol::make("INTEGER"));
  insert(BuiltinTypeSymbol::make("REAL"));
  insert(BuiltinTypeSymbol::make("STRING"));
  //DEBUG("Added built-in types: %s.", "int,float,string");
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Param::Param(d::DAst var, d::DAst type) : Ast(DCAST(Ast,var)->token){
  var_node=var;
  type_node=type;
  _name = DCAST(Var,var_node)->name();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Param::visit(d::IAnalyzer* a){
  auto n= DCAST(Ast,type_node)->name();
  if(auto t= a->search(n); !t){
    E_SYNTAX("Unbound type %s near %s",
             n.c_str(),
             d::pr_addr( token->addr()).c_str());
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Param::eval(d::IEvaluator* e){
  return DVAL_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VarDecl::VarDecl(d::DAst var, d::DAst type)
  : Ast(DCAST(Ast,var)->token), var_node(var), type_node(type){
  _name= DCAST(Ast,var_node)->name();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarDecl::visit(d::IAnalyzer* a){
  auto type_name = DCAST(Ast,type_node)->name();
  auto var_name = DCAST(Var,var_node)->name();
  auto type_symbol = a->search(type_name);
  auto i=token->addr();
  if(!type_symbol)
    E_SYNTAX("Unbound type %s near %s",
             C_STR(type_name), d::pr_addr(i).c_str());

  if(auto s = a->find(var_name); s){
    E_SYNTAX("Duplicate var %s near %s",
             C_STR(var_name), d::pr_addr(i).c_str());
  }else{
    a->define(d::VarSymbol::make(var_name, type_symbol));
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue VarDecl::eval(d::IEvaluator* e){
  d::DValue v;
  return e->setValue(this->name(), v);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block::Block(d::DToken k,std::vector<d::DAst>& decls, d::DAst compound)
  : Ast(k), compound(compound){
  _name="Block";
  s__ccat(declarations, decls);
}
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
    auto v= x->eval(e);
  }
  return compound->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl::ProcedureDecl(d::DToken k,cstdstr& proc_name,
    d::AstVec& pms, d::DAst block_node) : Ast(k){
  _name=proc_name;
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
    auto p_ = s__cast(Param,p.get());
    auto pn = DCAST(Ast,p_->var_node)->name();
    auto tn = DCAST(Ast,p_->type_node)->name();
    auto pt= a->search(tn);
    auto v = d::VarSymbol::make(pn, pt);
    s__conj(fp->params(), a->define(v));
  }

  block->visit(a);
  a->popScope();
  fp->setBody(this->block);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue ProcedureDecl::eval(d::IEvaluator* e){
  return P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall::ProcedureCall(cstdstr& proc_name,
                             d::AstVec& p, d::DToken token) : Ast(token){
  _name=proc_name;
  s__ccat(args, p);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureCall::visit(d::IAnalyzer* a){
  for(auto& p : args){
    p->visit(a);
  }
  //get the corresponding symbol
  if(auto x = a->search(_name); x){
    proc_symbol = x;
  }else{
    E_SYNTAX("Unbound proc %s near %s",
             _name.c_str(),
             d::pr_addr( token->addr()).c_str());
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue ProcedureCall::eval(d::IEvaluator* e){
  auto fs= DCAST(d::FnSymbol,proc_symbol);
  auto z= args.size();
  ASSERT1(fs->params().size() == z)
  e->pushFrame(_name);

  for(auto i=0; i < z; ++i){
    auto& p= fs->params()[i];
    auto v= args[i]->eval(e);
    e->setValue(DCAST(d::VarSymbol, p)->name(), v);
  }

  auto r= fs->body()->eval(e);
  e->popFrame();
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(d::DToken k,cstdstr& name, d::DAst block) : Ast(k){
  _name=name;
  this->block=block;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(d::IAnalyzer* a){
  block->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Program::eval(d::IEvaluator* e){
  //std::cout << "program start....\n";
  return block->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst variable(SimplePascalParser* ps){
  auto node = Var::make(ps->token());
  return (ps->eat(d::T_IDENT), node);
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst factor(SimplePascalParser* ps){
  auto t= ps->token();
  d::DAst res;
  switch(t->type()){
    case d::T_PLUS:
      res= (ps->eat(), UnaryOp::make(t, factor(ps)));
      break;
    case d::T_MINUS:
      res= (ps->eat(), UnaryOp::make(t, factor(ps)));
      break;
    case d::T_INT:
      res= (ps->eat(), Num::make(t));
      break;
    case d::T_REAL:
      res= (ps->eat(), Num::make(t));
      break;
    case d::T_STRING:
      res= (ps->eat(), String::make(t));
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
d::DAst term(SimplePascalParser* ps){
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV};
  auto res= factor(ps);
  while(s__contains(ops,ps->cur()))
    res = BinOp::make(res, ps->eat(), factor(ps));
  return res;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst expr(SimplePascalParser* ps){
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  d::DAst res= term(ps);
  while(s__contains(ops,ps->cur()))
    res= BinOp::make(res, ps->eat(), term(ps));
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst type_spec(SimplePascalParser* ps){
  auto t = ps->token();
  switch(t->type()){
    case T_STR:
    case T_INT:
    case T_REAL: ps->eat(); break;
    default:
      E_SYNTAX("Unknown token %d near %s",
               t->type(), d::pr_addr( t->addr()).c_str());
  }
  return Type::make(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::AstVec variable_declaration(SimplePascalParser* ps){
  d::AstVec vars { Var::make(ps->eat(d::T_IDENT)) };
  while(ps->isCur(d::T_COMMA)){
    ps->eat();
    s__conj(vars, Var::make(ps->eat(d::T_IDENT)));
  }
  auto type = (ps->eat(d::T_COLON), type_spec(ps));
  d::AstVec out;
  for(auto &x : vars)
    s__conj(out, VarDecl::make(x, type));
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst assignment_statement(SimplePascalParser* ps){
  auto left = variable(ps);
  auto t= ps->eat(T_ASSIGN);
  auto right = expr(ps);
  return Assignment::make(left, t, right);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst empty(SimplePascalParser* ps){
  return NoOp::make(ps->token());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst proccall_statement(SimplePascalParser* ps){
  auto token = ps->token();
  auto pn = token->getStr();
  d::AstVec pms;

  ps->eat(d::T_IDENT);
  ps->eat(d::T_LPAREN);

  if(!ps->isCur(d::T_RPAREN))
    s__conj(pms, expr(ps));

  while(ps->isCur(d::T_COMMA)){
    ps->eat();
    s__conj(pms,expr(ps));
  }

  return (ps->eat(d::T_RPAREN), ProcedureCall::make(pn, pms, token));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst statement(SimplePascalParser* ps){
  d::DAst node;
  switch(ps->cur()){
    case T_BEGIN:
      node = compound_statement(ps);
      break;
    case d::T_IDENT:
      if(ps->peek() == '(')
        node = proccall_statement(ps);
      else
        node = assignment_statement(ps);
      break;
    default:
      node = empty(ps);
      break;
  }
  return node;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::AstVec statement_list(SimplePascalParser* ps){
  auto s= statement(ps);
  d::AstVec results;
  if(!(DCAST(Ast,s)->name() == "709394"))
    s__conj(results,s);

  while(ps->isCur(d::T_SEMI)){
    s= (ps->eat(), statement(ps));
    if(!(DCAST(Ast,s)->name() == "709394"))
      s__conj(results,s);
  }

  if(ps->isCur(d::T_IDENT))
    RAISE(d::SyntaxError,
          "Unexpected token `%s`.", C_STR(ps->token()->getStr()));

  return results;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst compound_statement(SimplePascalParser* ps){
  auto X=ps->token();
  auto nodes = (ps->eat(T_BEGIN),statement_list(ps));
  auto root= (ps->eat(T_END), Compound::make(X));
  auto pr= DCAST(Compound,root);
  for(auto& node : nodes)
    s__conj(pr->statements,node);
  return root;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::AstVec formal_parameters(SimplePascalParser* ps){

  d::TokenVec  param_tokens { ps->eat(d::T_IDENT) };
  d::AstVec pnodes;

  while(ps->isCur(d::T_COMMA)){
    ps->eat();
    s__conj(param_tokens, ps->eat(d::T_IDENT));
  }

  auto type_node = (ps->eat(d::T_COLON),type_spec(ps));
  for(auto& t : param_tokens){
    //::printf("param toke= %s\n", t->getLiteralAsStr());
    s__conj(pnodes, Param::make(Var::make(t), type_node));
  }

  return pnodes;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::AstVec formal_parameter_list(SimplePascalParser* ps){
  d::AstVec out;

  if(!ps->isCur(d::T_IDENT))
    return out;

  auto pnodes = formal_parameters(ps);
  while(ps->isCur(d::T_SEMI)){
    auto pms = (ps->eat(), formal_parameters(ps));
    s__ccat(pnodes, pms);
  }

  return pnodes;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst procedure_declaration(SimplePascalParser* ps){

  auto pn = (ps->eat(T_PROCEDURE), ps->eat(d::T_IDENT));
  d::AstVec params;

  if(ps->isCur(d::T_LPAREN)){
    params = (ps->eat(), formal_parameter_list(ps));
    ps->eat(d::T_RPAREN);
  }

  auto decl = ProcedureDecl::make(ps->token(),
                                  pn->getStr(),
                                  params,
                                  (ps->eat(d::T_SEMI), block(ps)));
  //::printf("proc name=%s\n", decl->name().c_str());
  return (ps->eat(d::T_SEMI), decl);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::AstVec declarations(SimplePascalParser* ps){
  d::AstVec ds;
  if(ps->isCur(T_VAR)){
    ps->eat();
    while(ps->isCur(d::T_IDENT)){
      auto vs = variable_declaration(ps);
      s__ccat(ds,vs);
      ps->eat(d::T_SEMI);
    }
  }
  while(ps->isCur(T_PROCEDURE))
    s__conj(ds,procedure_declaration(ps));

  return ds;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst block(SimplePascalParser* ps){
  auto k=ps->token();
  auto decls=declarations(ps);
  return Block::make(k,decls, compound_statement(ps));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst program(SimplePascalParser* ps){
  auto var_node = (ps->eat(T_PROGRAM),variable(ps));
  auto pn= DCAST(Ast,var_node)->name();
  auto k= ps->token();
  auto prog = Program::make(k,pn, (ps->eat(d::T_SEMI),block(ps)));
  //::printf("program = %s\n", prog->name().c_str());
  return (ps->eat(d::T_DOT), prog);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SimplePascalParser::~SimplePascalParser(){
  DEL_PTR(lex);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SimplePascalParser::SimplePascalParser(const Tchar* src){
  lex = new Lexer(src);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SimplePascalParser::isEof() const{
  return lex->ctx().eof;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst SimplePascalParser::parse(){
  return program(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SimplePascalParser::cur(){
  return lex->ctx().cur->type();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar SimplePascalParser::peek(){
  return d::peek(lex->ctx());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SimplePascalParser::isCur(int type){
  return lex->ctx().cur->type() == type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken SimplePascalParser::token(){
  return lex->ctx().cur;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken SimplePascalParser::eat(){
  auto t= lex->ctx().cur;
  lex->ctx().cur=lex->getNextToken();
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken SimplePascalParser::eat(int wanted){
  auto t= lex->ctx().cur;
  if(t->type() != wanted){
    E_SYNTAX("Expected token %s, got token %s near %s",
             C_STR(typeToString(wanted)),
             C_STR(t->pr_str()),
             d::pr_addr( t->addr()).c_str());
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
