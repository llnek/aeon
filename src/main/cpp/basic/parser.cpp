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
namespace czlab::basic {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define TKN(x) CAST(Token,x)
#define AST(x) CAST(Ast,x)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst expr(BasicParser*);
d::DslAst b_expr(BasicParser*);
d::DslAst statement(BasicParser*);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast() {
  _offset=0;
  _line =0;
  _token=Token::make(d::T_ETHEREAL,"<?>", s__pair(int,int,0,0));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(const std::map<llong,d::DslAst>& lines) {
  auto pos=0;
  for (auto i=lines.begin(), e=lines.end(); i!=e; ++i) {
    mlines[i->first] = pos++;
    s__conj(vlines, i->second);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Program::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto len=vlines.size();
  d::DslValue last;
  _e->init_counters();
  //std::cout << "len = " << len << "\n" << pr_str() << "\n";
  while (_e->isOn() &&
         _e->incr_pc() < len) {
    auto line= AST(vlines[_e->pc()]);
    //std::cout << line->pr_str() << "\n";
    last= line->eval(e);
  }
  return (_e->finz_counters(), last);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(d::IAnalyzer* a) {
  //std::cout << "Program starting visit\n";
  auto _e = s__cast(Basic,a);
  _e->installProgram(mlines);
  for (auto& i : vlines) {
    i->visit(a);
  }
  auto f= _e->getCurForLoop();
  if (f)
    RAISE(d::SemanticError,
          "Unmatched for-loop near line %d.", (int) f->begin);
  //std::cout << "Program ended visit\n";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Program::pr_str() const {
  stdstr buf;
  for (auto& i : vlines) {
    auto c= CAST(Compound,i);
    buf += N_STR(c->line());
    buf += " ";
    buf += AST(i)->pr_str();
    buf += "\n";
  }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound(llong ln, const d::AstVec& vs) {
  s__ccat(stmts,vs);
  line(ln);
  auto pos=0;
  for (auto& s : stmts) {
    AST(s)->line(ln);
    AST(s)->offset(pos++);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Compound::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto len= stmts.size();
  auto pos= _e->poffset();
  //std::cout << "line = " << line() << "\n";
  for (; pos < len; ++pos) {
    //std::cout << "offset = " << pos << "\n";
    auto last= stmts[pos]->eval(e);
    auto n= cast_number(last,0);
    if (n && n->isZero()) {
      break;
    }
  }
  return d::DslValue(P_NIL);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Compound::visit(d::IAnalyzer* a) {
  for (auto& i : stmts) {
    i->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Compound::pr_str() const {
  stdstr buf;
  for (auto& i : stmts) {
    if (!buf.empty()) buf += ":";
    buf += AST(i)->pr_str();
  }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LibFunc::LibFunc(const stdstr& name, Invoker k) : Function(name) {
  fn=k;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LibFunc::invoke(d::IEvaluator* e, d::VSlice args) {
  return fn(e, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LibFunc::invoke(d::IEvaluator* e) {
  d::ValVec vs;
  return invoke(e, d::VSlice(vs));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LibFunc::pr_str(bool p) const {
  return "#native@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ForNext::ForNext(d::DslAst var) {
  this->var=var;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ForNext::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto f= _e->getForLoop(_e->pc());
  return NUMBER_VAL(_e->jumpFor(f));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ForNext::visit(d::IAnalyzer* a) {
  auto n= AST(var)->token()->getLiteralAsStr();
  auto _e = s__cast(Basic,a);
  var->visit(a);
  _e->xrefForNext(n, line(), offset());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ForNext::pr_str() const {
  return AST(var)->pr_str();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ForLoop::ForLoop(d::DslAst var, d::DslAst init, d::DslAst term, d::DslAst step) {
  this->var=var;
  this->init= init;
  this->term= term;
  this->step=step;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ForLoop::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto P= _e->pc();
  auto quit=true;
  auto f= _e->getForLoop(P);
  if (!f->init) {
    // first invoke
    f->init = this->init->eval(e);
    cast_number(f->init,1);
    e->setValue(f->var, f->init);
    f->step = this->step->eval(e);
    auto i= cast_number(f->init,1);
    auto s= cast_number(f->step,1);
    auto _t = term->eval(e);
    auto t= cast_number(_t,1);
    if (s->isPos()) {
      quit = (i->getFloat() > t->getFloat());
    }
    else
    if (s->isNeg()) {
      quit = (i->getFloat() < t->getFloat());
    }
    if (quit) { _e->endFor(f); }
  } else {
    auto _v= e->getValue(f->var);
    auto v= cast_number(_v,1);
    auto s= cast_number(f->step,1);
    auto _t= term->eval(e);
    auto t= cast_number(_t,1);
    auto z = 0.0;
    if (s->isPos()) {
      z = v->getFloat() + s->getFloat();
      quit = z > t->getFloat();
    }
    else
    if (s->isNeg()) {
      z = v->getFloat() - s->getFloat();
      quit = z < t->getFloat();
    }
    if (quit) {
      _e->endFor(f);
    }
    else
    if (v->isInt()) {
      e->setValue(f->var, NUMBER_VAL((llong)z));
    } else {
      e->setValue(f->var, NUMBER_VAL(z));
    }
  }

  return quit ? NUMBER_VAL(0) : d::DslValue(P_NIL);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ForLoop::visit(d::IAnalyzer* a) {
  auto vn= AST(var)->token()->getLiteralAsStr();
  var->visit(a);
  init->visit(a);
  term->visit(a);
  step->visit(a);
  s__cast(Basic,a)->addForLoop(ForLoopInfo::make(vn, line(), offset()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ForLoop::pr_str() const {
  stdstr buf {"FOR"};

  buf += " ";
  buf += AST(var)->pr_str();
  buf += " = ";
  buf += AST(init)->pr_str();
  buf += " TO ";
  buf += AST(term)->pr_str();
  buf += " STEP ";
  buf += AST(step)->pr_str();

  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IfThen::IfThen(d::DslToken _t, d::DslAst c, d::DslAst t, d::DslAst z) : Ast(_t) {
  cond=c;
  then=t;
  elze=z;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IfThen::IfThen(d::DslToken _t, d::DslAst c, d::DslAst t) : Ast(_t) {
  cond=c;
  then=t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue IfThen::eval(d::IEvaluator* e) {
  auto c= cond->eval(e);
  auto n= cast_number(c,1);
  return !n->isZero() ? then->eval(e) : (elze ? elze->eval(e) : P_NIL);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void IfThen::visit(d::IAnalyzer* a) {
  cond->visit(a);
  then->visit(a);
  if (elze) elze->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr IfThen::pr_str() const {
  stdstr buf { "IF" };

  buf += " ";
  buf += AST(cond)->pr_str();
  buf += " THEN ";
  buf += AST(then)->pr_str();
  if (elze) {
    buf += " ELSE ";
    buf += AST(elze)->pr_str();
  }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Run::Run(d::DslToken t) : Ast(t) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Run::eval(d::IEvaluator*) {
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Run::visit(d::IAnalyzer* a) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Run::pr_str() const {
  return token()->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
End::End(d::DslToken t) : Ast(t) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue End::eval(d::IEvaluator* e) {
  s__cast(Basic,e)->halt();
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void End::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr End::pr_str() const {
  return token()->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
GoSubReturn::GoSubReturn(d::DslToken t) : Ast(t) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue GoSubReturn::eval(d::IEvaluator* e) {
  s__cast(Basic,e)->retSub();
  return NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void GoSubReturn::visit(d::IAnalyzer* a) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr GoSubReturn::pr_str() const {
  return token()->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
GoSub::GoSub(d::DslToken t, d::DslAst e) : Ast(t) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue GoSub::eval(d::IEvaluator* e) {
  //std::cout << "Jumping to subroutine: " << "\n";
  auto res= expr->eval(e);
  auto target= cast_number(res,1)->getInt();
  //std::cout << "Jumping to subroutine: " << target << "\n";
  s__cast(Basic,e)->jumpSub(target, line(), offset());
  return NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void GoSub::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr GoSub::pr_str() const {
  stdstr buf { token()->getLiteralAsStr() };

  buf += " ";
  buf += AST(expr)->pr_str();

  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Goto::Goto(d::DslToken t, d::DslAst e) : Ast(t) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Goto::eval(d::IEvaluator* e) {
  auto res= expr->eval(e);
  auto line= cast_number(res,1)->getInt();
  //std::cout << "Jumping to line: " << line << "\n";
  s__cast(Basic,e)->jump(line);
  // pass false back to stop compound statement.
  return NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Goto::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Goto::pr_str() const {
  return token()->getLiteralAsStr() + " " + AST(expr)->pr_str();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
FuncCall::FuncCall(d::DslAst t, const d::AstVec& pms) {
  fn=t;
  s__ccat(args,pms);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue FuncCall::eval(d::IEvaluator* e) {
  auto n= AST(fn)->token()->getLiteralAsStr();
  auto f= e->getValue(n);
  if (!f)
    RAISE(d::NoSuchVar, "Unknown function/array: %s", C_STR(n));
  auto fa = cast_array(f,0);
  auto fv = cast_native(f,0);
  if (E_NIL(fa) && E_NIL(fv)) {
    expected("Array var or function", f);
  }
  d::ValVec pms;
  for (auto& a : args) {
    s__conj(pms, a->eval(e));
  }
  d::VSlice _args(pms);
  if (fa)
    return fa->get(_args);
  else
    return pms.empty() ? fv->invoke(e) : fv->invoke(e,_args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr FuncCall::pr_str() const {
  stdstr buf { AST(fn)->pr_str() };
  stdstr pms;
  buf += "(";
  for (auto& i : args) {
    if (!pms.empty()) pms += " , ";
    pms += AST(i)->pr_str();
  }
  buf += pms;
  buf += ")";
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void FuncCall::visit(d::IAnalyzer* a) {
  fn->visit(a);
  for (auto& i : args) {
    i->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BoolTerm::BoolTerm(const d::AstVec& ts) {
  s__ccat(terms, ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BoolTerm::eval(d::IEvaluator* e) {
  ASSERT(terms.size() > 0, "Malformed expression, got %d terms.", (int)terms.size());
  auto i=0;
  auto z=terms.size();
  auto lhs = terms[i]->eval(e);
  auto res= !cast_number(lhs,1)->isZero();
  if (z==1) { return lhs; }
  if (!res) { return NUMBER_VAL(0); }
  ++i;
  while (i < z) {
    auto _t = terms[i]->eval(e);
    auto rhs = !cast_number(_t,1)->isZero();
    res= (res && rhs);
    if (res) return NUMBER_VAL(1);
    ++i;
  }
  return NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolTerm::pr_str() const {
  stdstr buf;
  for (auto& i : terms) {
    if (!buf.empty()) buf += " ";
    buf += AST(i)->pr_str();
  }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BoolTerm::visit(d::IAnalyzer* a) {
  for (auto& i : terms) {
    i->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BoolExpr::BoolExpr(const d::AstVec& ts, const d::TokenVec& ops) {
  ASSERT(ts.size() == (ops.size()+1),
         "Malformed expression, got %d  terms, %d ops.", (int)ts.size(), (int)ops.size());
  s__ccat(terms, ts);
  s__ccat(this->ops, ops);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolExpr::pr_str() const {
  if (terms.size()==0) { return ""; }
  stdstr buf { AST(terms[0])->pr_str() };
  for (int i=0,pz=ops.size(); i<pz; ++i) {
    buf += " ";
    buf += TKN(ops[i])->pr_str();
    buf += " ";
    buf += AST(terms[i+1])->pr_str();
  }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BoolExpr::eval(d::IEvaluator* e) {
  int z1= terms.size();
  int t1= ops.size();
  ASSERT(z1 == (t1+1),
         "Malformed expression, got %d  terms, %d ops.", z1, t1);
  auto i=0;
  auto lhs= terms[0]->eval(e);
  auto res= !cast_number(lhs,1)->isZero();
  if (z1==1) { return lhs; }
  while (i < t1) {
    auto t= ops[i];
    if (t->type() == T_OR && res) {
      return NUMBER_VAL(1);
    }
    auto _r= terms[i+1]->eval(e);
    auto rhs= !cast_number(_r,1)->isZero();
    if (t->type() == T_XOR) {
      res= (res != rhs);
    }
    else
    if (rhs) { res=true; }
    ++i;
  }
  return NUMBER_VAL(res ? 1 : 0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BoolExpr::visit(d::IAnalyzer* a) {
  for (auto& i : terms) {
    i->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RelationOp::RelationOp(d::DslAst left, d::DslToken op, d::DslAst right) : Ast(op) {
  lhs= left;
  rhs= right;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr RelationOp::pr_str() const {
  stdstr buf { AST(lhs)->pr_str() };
  buf += " ";
  buf += TKN(token())->pr_str();
  buf += " ";
  buf += AST(rhs)->pr_str();
  return "(" + buf + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue RelationOp::eval(d::IEvaluator* e) {
  auto x = lhs->eval(e);
  auto y = rhs->eval(e);
  auto xn= cast_number(x,1);
  auto yn= cast_number(y,1);
  auto ints = xn->isInt() && yn->isInt();
  auto b=false;

  switch (token()->type()) {
    case T_NOTEQ:
      b= x->equals(y) ? 0 : 1;
    break;
    case d::T_EQ:
      b= x->equals(y) ? 1 : 0;
    break;
    case T_GTEQ:
      b= ints
           ? xn->getInt() >= yn->getInt()
           : xn->getFloat() >= yn->getFloat();
    break;
    case T_LTEQ:
      b= ints
           ? xn->getInt() <= yn->getInt()
           : xn->getFloat() <= yn->getFloat();
    break;
    case d::T_GT:
      b= ints
           ? xn->getInt() > yn->getInt()
           : xn->getFloat() > yn->getFloat();
    break;
    case d::T_LT:
    b= ints
         ? xn->getInt() < yn->getInt()
         : xn->getFloat() < yn->getFloat();
    break;
  }
  return NUMBER_VAL(b ? 1 : 0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void RelationOp::visit(d::IAnalyzer* a) {
  lhs->visit(a);
  rhs->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NotFactor::NotFactor(d::DslAst e) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr NotFactor::pr_str() const {
  return AST(expr)->pr_str();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue NotFactor::eval(d::IEvaluator* e) {
  auto res= expr->eval(e);
  auto i= cast_number(res,1);
  return NUMBER_VAL( i->isZero() ? 0 : 1 );
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void NotFactor::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BinOp::BinOp(d::DslAst left, d::DslToken op, d::DslAst right) : Ast(op) {
  lhs=left;
  rhs=right;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BinOp::eval(d::IEvaluator* e) {
  auto lf= lhs->eval(e);
  auto rt= rhs->eval(e);
  return op_math(lf, token()->type(), rt);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BinOp::pr_str() const {
  stdstr buf;
  buf += "( ";
  buf += AST(lhs)->pr_str();
  buf += " ";
  buf += TKN(token())->pr_str();
  buf += " ";
  buf += AST(rhs)->pr_str();
  buf += " )";
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(d::IAnalyzer* a) {
  lhs->visit(a);
  rhs->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(d::DslToken t) : Ast(t) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Num::pr_str() const {
  return TKN(token())->pr_str();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Num::eval(d::IEvaluator* e) {
  if (token()->type() == d::T_INTEGER) {
    return NUMBER_VAL(token()->getLiteralAsInt());
  } else {
    return NUMBER_VAL(token()->getLiteralAsReal());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(d::IAnalyzer* a) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String::String(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr String::pr_str() const {
  return "\"" + TKN(token())->pr_str() + "\"";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue String::eval(d::IEvaluator*) {
  return STRING_VAL(token()->getLiteralAsStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void String::visit(d::IAnalyzer*) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(d::DslToken t) : Ast(t) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Var::pr_str() const { return TKN(token())->pr_str(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Var::eval(d::IEvaluator* e) {
  return e->getValue(token()->getLiteralAsStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(d::IAnalyzer*) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(d::DslToken t, d::DslAst e) : Ast(t) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr UnaryOp::pr_str() const {
  stdstr buf;
  buf += TKN(token())->pr_str();
  buf += AST(expr)->pr_str();
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue UnaryOp::eval(d::IEvaluator* e) {
  auto res = expr->eval(e);
  auto n = cast_number(res,1);
  if (token()->type() == d::T_MINUS) {
    if (n->isInt()) {
      res = NUMBER_VAL(- n->getInt());
    } else {
      res = NUMBER_VAL(- n->getFloat());
    }
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Print::Print(const d::AstVec& es) {
  s__ccat(exprs, es);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Print::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto lastSemi=false;
  for (auto& i : exprs) {
    auto t= AST(i)->token()->type();
    lastSemi=false;
    if (t == d::T_COMMA) {
      _e->writeString(" ");
    }
    else
    if (t == d::T_SEMI) {
      lastSemi=true;
    }
    else
    if (auto res= i->eval(e); res) {
      _e->writeString(res->pr_str(1));
    }
  }

  if (! lastSemi) {
    _e->writeln();
  }

  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Print::visit(d::IAnalyzer* a) {
  for (auto& i : exprs) {
    i->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Print::pr_str() const {
  stdstr buf {"PRINT"};
  stdstr b;

  for (auto& i : exprs) {
    if (!b.empty()) b += " ";
    b += AST(i)->pr_str();
  }

  buf += " ";
  buf += b;
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PrintSep::PrintSep(d::DslToken t) : Ast(t) {  }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue PrintSep::eval(d::IEvaluator* e) {
  return NUMBER_VAL(token()->type());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void PrintSep::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr PrintSep::pr_str() const {
  return token()->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment::Assignment(d::DslAst left, d::DslToken op, d::DslAst right) : Ast(op) {
  lhs=left;
  rhs=right;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Assignment::eval(d::IEvaluator* e) {
  auto t= token()->type();
  auto v= lhs;
  auto res= rhs->eval(e);
  if (t == T_ARRAYINDEX) {
    auto fc = CAST(FuncCall,lhs);
    auto fn = fc->funcName();
    auto args= fc->funcArgs();
    d::ValVec out;
    for (auto& x : args) {
      s__conj(out, x->eval(e));
    }
    auto vn= CAST(Var,fn)->name();
    auto vv= e->getValue(vn);
    auto arr= cast_array(vv,1);
    arr->set(d::VSlice(out), res);
  } else {
    auto vn= CAST(Var,lhs)->name();
    e->setValue(vn, res);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Assignment::pr_str() const {
  stdstr buf;
  buf += AST(lhs)->pr_str();
  buf += " = ";
  buf += AST(rhs)->pr_str();
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a) {
  auto t= token()->type();
  auto v= lhs;
  if (t == T_ARRAYINDEX) {
    v= CAST(FuncCall,lhs)->funcName();
  }
  auto vn= CAST(Var,v)->name();
  if (t == T_ARRAYINDEX) {
    // array must have been defined.
    auto x= a->find(vn);
    ASSERT(x,
           "Expected array var %s, but not found.", C_STR(vn));
  }
  lhs->visit(a);
  rhs->visit(a);
  if (t != T_ARRAYINDEX) {
    a->define(d::Symbol::make(vn));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArrayDecl::ArrayDecl(d::DslToken t,
    d::DslAst v,
    const std::vector<llong>& sizes) : Ast(t) {
  auto n= CAST(Var,v)->token()->getLiteralAsStr();
  if (n[n.length()-1] == '$') {
    stringType=true;
  } else {
    stringType=false;
  }
  var=v;
  s__ccat(ranges,sizes);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ArrayDecl::pr_str() const {
  stdstr buf;
  stdstr b;
  buf += AST(var)->pr_str();
  buf += "(";
  for (auto& x : ranges) {
    if (!b.empty()) b += ",";
    b += N_STR(x);
  }
  buf += b;
  buf += ")";
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ArrayDecl::eval(d::IEvaluator* e) {
  auto n= CAST(Var,var)->token()->getLiteralAsStr();
  return e->setValue(n, BArray::make(ranges));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ArrayDecl::visit(d::IAnalyzer* a) {
  auto n= CAST(Var,var)->token()->getLiteralAsStr();
  if (auto c= a->find(n); c) {
    RAISE(d::SemanticError,
          "Array var %s defined already.\n", C_STR(n));
  }
  a->define(d::Symbol::make(n, d::Symbol::make("ARRAY")));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Comment::Comment(const d::TokenVec& ts) {
  s__ccat(tkns,ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Comment::eval(d::IEvaluator*) {
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Comment::visit(d::IAnalyzer*) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Comment::pr_str() const {
  stdstr buf;
  for (auto& t : tkns) {
    if (!buf.empty()) buf += " ";
    buf += TKN(t)->getLiteralAsStr();
  }
  return "REM " + buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Input::Input(d::DslAst var, d::DslAst prompt) {
  this->var=var;
  this->prompt=prompt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Input::eval(d::IEvaluator*) {
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Input::visit(d::IAnalyzer* a) {
  var->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Input::pr_str() const {
  stdstr buf { "INPUT" };
  if (prompt) {
    buf += " " + AST(prompt)->pr_str();
  }
  buf += " ; ";
  buf += AST(var)->pr_str();
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BasicParser::BasicParser(const char* src) {
  lex=new Lexer(src);
  curLine=1;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BasicParser::~BasicParser() { DEL_PTR(lex); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken BasicParser::getEthereal() {
  return Token::make(d::T_ETHEREAL, "<?>",
                     s__pair(int,int,lex->ctx().line, lex->ctx().col));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken BasicParser::eat(int wanted) {
  auto t= lex->ctx().cur;
  if (t->type() != wanted) {
    auto i=t->srcInfo();
    RAISE(d::SyntaxError,
          "Expected token %s, got token %s near line %d(%d).\n",
          C_STR(Token::typeToString(wanted)),
          C_STR(t->pr_str()), i.first, i.second);
  }
  lex->ctx().cur=lex->getNextToken();
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken BasicParser::eat() {
  auto t= lex->ctx().cur;
  lex->ctx().cur=lex->getNextToken();
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BasicParser::isEof() const {
  return lex->ctx().cur->type() == d::T_EOF;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst assignment(BasicParser* bp, d::DslAst lhs) {
  auto t= bp->eat(d::T_EQ);
  auto n= T_ARRAYINDEX;
  auto val= expr(bp);
  return Assignment::make(lhs, token(n, "[]", TKN(t)->srcInfo()), val);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst assignment(BasicParser* bp, d::DslToken name) {
  auto t= bp->eat(d::T_EQ);
  auto val= expr(bp);
  return Assignment::make(Var::make(name),t, val);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst funcall(BasicParser* bp, d::DslToken name) {
  d::AstVec pms;

  bp->eat(d::T_LPAREN);
  if (!bp->isCur(d::T_RPAREN)) {
    s__conj(pms, expr(bp));
  }

  while (bp->isCur(d::T_COMMA)) {
    bp->eat();
    s__conj(pms, expr(bp));
  }

  return (bp->eat(d::T_RPAREN), FuncCall::make(Var::make(name), pms));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst skipComment(BasicParser* bp) {
  d::TokenVec tkns;
  bp->eat(T_REM);
  while (1) {
    if (bp->isEof() || bp->isCur(T_EOL))
    break;
    s__conj(tkns, bp->eat());
  }
  return Comment::make(tkns);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst input(BasicParser* bp) {
  d::DslAst prompt;
  bp->eat(T_INPUT);
  if (bp->isCur(d::T_STRING)) {
    prompt= String::make(bp->eat());
  }
  bp->eat(d::T_SEMI);
  return Input::make(Var::make(bp->eat(d::T_IDENT)),prompt);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst ifThen(BasicParser* bp) {
  auto _t= bp->eat(T_IF);
  auto c= b_expr(bp);
  auto t= (bp->eat(T_THEN), statement(bp));
  if (!bp->isCur(T_ELSE)) {
    return IfThen::make(_t, c,t);
  } else {
    bp->eat();
    return IfThen::make(_t, c,t, statement(bp));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst forNext(BasicParser* bp) {
  auto v = (bp->eat(T_NEXT),bp->eat(d::T_IDENT));
  return ForNext::make(Var::make(v));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst forLoop(BasicParser* bp) {
  auto v= (bp->eat(T_FOR),bp->eat(d::T_IDENT));
  auto b= (bp->eat(d::T_EQ), expr(bp));
  auto e= (bp->eat(T_TO), expr(bp));
  // force a step = 1 as default
  auto t= Token::make(d::T_INTEGER, "1", s__pair(llong,llong,0,0));
  CAST(Token,t)->impl().num.setInt(1);
  auto s= Num::make(t);
  if (bp->isCur(T_STEP)) {
    s = (bp->eat(), expr(bp));
  }
  return ForLoop::make(Var::make(v), b, e, s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst print(BasicParser* bp) {
  bp->eat(T_PRINT);
  d::AstVec out;
  while (1) {
    if (bp->isCur(d::T_COLON) ||
        bp->isCur(T_EOL) ||
        bp->isEof()) { break; }
    if (bp->isCur(d::T_SEMI) ||
        bp->isCur(d::T_COMMA)) {
      s__conj(out, PrintSep::make(bp->eat()));
    }
    else
    if (auto e= expr(bp); e) {
      s__conj(out,e);
    }
  }
  return Print::make(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst gotoLine(BasicParser* bp) {
  return Goto::make(bp->eat(T_GOTO), expr(bp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst gosub(BasicParser* bp) {
  return GoSub::make(bp->eat(T_GOSUB), expr(bp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst returnSub(BasicParser* bp) {
  return GoSubReturn::make(bp->eat(T_RETURN));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst runProg(BasicParser* bp) {
  return Run::make(bp->eat(T_RUN));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst endProg(BasicParser* bp) {
  return End::make( bp->eat(T_END));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst relation(BasicParser* bp) {
  static std::set<int> ops1 { d::T_GT, d::T_LT, T_GTEQ };
  static std::set<int> ops2 { T_LTEQ, d::T_EQ, T_NOTEQ };
  auto res = expr(bp);
  while (s__contains(ops1, bp->cur()) ||
         s__contains(ops2, bp->cur()) ) {
    res= RelationOp::make(res, bp->eat(), expr(bp));
  }
  return res;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst b_factor(BasicParser* bp) { return relation(bp); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst not_factor(BasicParser* bp) {
  if (bp->isCur(T_NOT)) {
    bp->eat();
    return NotFactor::make(b_factor(bp));
  } else {
    return b_factor(bp);
  }
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst b_term(BasicParser* bp) {
  d::AstVec res { not_factor(bp) };
  while (bp->isCur(T_AND)) {
    bp->eat();
    s__conj(res, not_factor(bp));
  }
  return BoolTerm::make(res);
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst b_expr(BasicParser* bp) {
  static std::set<int> ops {T_OR, T_XOR};
  d::AstVec res { b_term(bp) };
  d::TokenVec ts;
  while (s__contains(ops, bp->cur())) {
    s__conj(ts, bp->token());
    bp->eat();
    s__conj(res, b_term(bp));
  }
  return BoolExpr::make(res, ts);
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst factor(BasicParser* bp) {
  auto t= bp->token();
  d::DslAst res;
  switch (t->type()) {
    case d::T_PLUS:
    case d::T_MINUS:
      res= (bp->eat(), UnaryOp::make(t, factor(bp)));
    break;
    case d::T_REAL:
    case d::T_INTEGER:
      res= (bp->eat(), Num::make(t));
    break;
    case d::T_STRING:
      res= (bp->eat(), String::make(t));
    break;
    case d::T_LPAREN:
      res= (bp->eat(), b_expr(bp));
      bp->eat(d::T_RPAREN);
    break;
    case d::T_IDENT:
      bp->eat();
      if (bp->isCur(d::T_LPAREN)) {
        // a func call, or array access
        res = funcall(bp,t);
      } else {
        res= Var::make(t);
      }
      break;
    default:
    break;
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst term2(BasicParser* bp) {
  static std::set<int> ops { T_POWER };
  auto res= factor(bp);
  while (s__contains(ops,bp->cur())) {
    //res = d::DslAst(new BinOp(res, bp->eat(), factor(bp)));
    //handles right associativity
    res = BinOp::make(res, bp->eat(), term2(bp));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst term(BasicParser* bp) {
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV, T_MOD};
  auto res= term2(bp);
  while (s__contains(ops,bp->cur())) {
    res = BinOp::make(res, bp->eat(), term2(bp));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst expr(BasicParser* bp) {
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  auto res= term(bp);
  while (s__contains(ops,bp->cur())) {
    res= BinOp::make(res, bp->eat(), term(bp));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst declArray(BasicParser* bp) {
  auto _t = bp->eat(T_DIM);
  auto t= bp->eat(d::T_IDENT);
  std::vector<llong> sizes;
  bp->eat(d::T_LPAREN);
  while (! bp->isEof()) {
    s__conj(sizes, bp->eat(d::T_INTEGER)->getLiteralAsInt());
    if (bp->isCur(d::T_RPAREN)) {
      break;
    }
    bp->eat(d::T_COMMA);
  }
  bp->eat(d::T_RPAREN);
  return ArrayDecl::make(_t, Var::make(t), sizes);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst statement(BasicParser* bp) {
  d::DslAst res;
  switch (bp->cur()) {
    case T_REM:
      res= skipComment(bp);
    break;

    case T_INPUT:
      res= input(bp);
    break;

    case T_IF:
      res= ifThen(bp);
    break;

    case T_FOR:
      res= forLoop(bp);
    break;

    case T_NEXT:
      res= forNext(bp);
    break;

    case T_PRINT:
      res= print(bp);
    break;

    case T_GOTO:
      res= gotoLine(bp);
    break;

    case T_RETURN:
      res= returnSub(bp);
    break;

    case T_END:
      res= endProg(bp);
    break;

    case T_RUN:
      res= runProg(bp);
    break;

    case T_GOSUB:
      res= gosub(bp);
    break;

    case T_DIM:
      res=declArray(bp);
    break;

    case T_LET:
      res=(bp->eat(),assignment(bp, bp->eat(d::T_IDENT)));
    break;

    case d::T_IDENT:
      auto n= bp->eat();
      if (bp->isCur(d::T_EQ)) {
        res=assignment(bp,n);
      }
      else {
        if (bp->isCur(d::T_LPAREN)) {
          auto fc= funcall(bp, n);
          if (!bp->isCur(d::T_EQ)) {
            res=fc;
          } else {
            // assignment
            res = assignment(bp, fc);
          }
        } else {
          RAISE(d::SyntaxError,
                "Unexpected identifier %s.", C_STR(TKN(n)->pr_str()));
        }
      }
    break;
  }

  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst compound_statements(BasicParser* bp) {
  auto loop=true;
  d::AstVec out;
  while (loop && !bp->isEof()) {
    switch (bp->cur()) {
      case d::T_COLON:
        bp->eat();
      break;

      case T_EOL:
        //std::cout << "just ate a EOL\n";
        loop=false;
        bp->eat();
      break;

      default:
        s__conj(out, statement(bp));
      break;
    }
  }

  return Compound::make(bp->line(), out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst parse_line(BasicParser* bp) {

  auto t= bp->token();
  d::DslAst res;
  llong line= -1;

  // usually line number, but can be none.
  if (t->type() == d::T_INTEGER) {
    line = t->getLiteralAsInt();
    bp->eat();
  }

  if (!bp->isEof()) {
    if (bp->isCur(T_EOL)) {
      bp->eat();
    } else {
      bp->setLine(line);
      res= compound_statements(bp);
    }
  }

  //std::cout << "parsed line = " << line << "\n";
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst program(BasicParser* bp) {
  std::map<llong,d::DslAst> lines;
  d::AstVec raws;
  do {
    if (auto res= parse_line(bp); res) {
      auto n = bp->line();
      if (n < 0)
        s__conj(raws,res);
      else
        lines[n]= res;
    }
  }
  while (!bp->isEof());

  //std::cout << "parsed ALL lines, total count = " << lines.size() << "\n";
  return Program::make(lines);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst BasicParser::parse() {
  return program(this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int BasicParser::cur() {
  return lex->ctx().cur->type();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar BasicParser::peek() {
  return d::peek(lex->ctx());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BasicParser::isCur(int type) {
  return lex->ctx().cur->type() == type;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken BasicParser::token() {
  return lex->ctx().cur;
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

/*
<b-expression> ::= <b-term> [<orop> <b-term>]*
 <b-term>       ::= <not-factor> [AND <not-factor>]*
 <not-factor>   ::= [NOT] <b-factor>
 <b-factor>     ::= <b-literal> | <b-variable> | <relation>
 <relation>     ::= | <expression> [<relop> <expression]

 <expression>   ::= <term> [<addop> <term>]*
 <term>         ::= t2 [<mulop> t2]*
 <t2>         ::= <signed factor> [<moreop> factor]*
 <signed factor>::= [<addop>] <factor>
 <factor>       ::= <integer> | <variable> | (<b-expression>)



Expr    ← Sum
Sum     ← Product (('+' / '-') Product)*
Product ← Power (('*' / '/') Power)*
Power   ← Value ('^' Power)?
Value   ← [0-9]+ / '(' Expr ')'
 */



