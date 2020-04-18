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
#define PRSTR(a) DCAST(Ast,a)->pr_str()
#define ACAST(x) DCAST(Ast,x)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst expr(BasicParser*);
d::DslAst b_expr(BasicParser*);
d::DslAst statement(BasicParser*);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast() {
  _offset=0;
  _line =0;
  _token=BToken::make(d::T_ETHEREAL,"<?>", DMARK(0,0));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(const std::map<int,d::DslAst>& lines) {
  // lines are sorted, so we get the ordering
  // now store them into a array for fast access.
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
    auto line= DCAST(Ast,vlines[_e->pc()]);
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
  if (auto f= _e->getCurForLoop(); f) {
    RAISE(d::SemanticError,
          "Unmatched for-loop near line %d.", f->begin); }
  //std::cout << "Program ended visit\n";
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Program::pr_str() const {
  stdstr buf;
  for (auto& i : vlines) {
    if (!buf.empty()) buf += "\n";
    buf += N_STR(DCAST(Compound,i)->line()) + " " + PRSTR(i); }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound(int ln, const d::AstVec& vs) {
  s__ccat(stmts,vs);
  line(ln);
  auto pos=0;
  for (auto& s : stmts) {
    DCAST(Ast,s)->line(ln);
    DCAST(Ast,s)->offset(pos++);
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
    if (n && n->isZero()) { break; }
  }
  return P_NIL;
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
    buf += PRSTR(i);
  }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
OnXXX::OnXXX(d::DslToken t, d::DslAst v, d::TokenVec& vs) : Ast(t) {
  var=v;
  for (auto& t : vs) {
    s__conj(targets, (int)t->getInt()); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue OnXXX::eval(d::IEvaluator* e) {
  auto _e= s__cast(Basic,e);
  auto t= token()->type();
  auto v= var->eval(e);
  auto n= cast_number(v,1);
  auto x= n->getInt();
  auto tz= targets.size();
  auto res= WRAP_VAL(P_NIL);

  //if x is 1, it jumps to the first line in the list;
  //if x is 2, it jumps to the second line, and so on.
  if (x > 0 && x <= tz) {
    // get the selected target
    x= targets[x-1];
    if (t == T_GOTO) {
      _e->jump(x); }
    else
    if (t== T_GOSUB) {
      _e->jumpSub(x, line(), offset()); }
    res=NUMBER_VAL(0);
  }

  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void OnXXX::visit(d::IAnalyzer* a) {
  var->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ForNext::ForNext(d::DslToken t, d::DslAst var) : Ast(t) {
  this->var=var;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ForNext::ForNext(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ForNext::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  return _e->jumpFor(_e->getForLoop(_e->pc())), P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ForNext::visit(d::IAnalyzer* a) {
  auto _e = s__cast(Basic,a);
  if (!var) {
    _e->xrefForNext(line(), offset());
  } else {
    var->visit(a);
    _e->xrefForNext(DCAST(Var,var)->name(), line(), offset());
  }
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
  auto P = _e->pc();
  bool quit=1;
  auto f= _e->getForLoop(P);

  //calc step and term
  auto _t= term->eval(e);
  auto _s= step->eval(e);
  auto s= cast_number(_s,1);
  auto t= cast_number(_t,1);
  auto i=t;
  auto z= 0.0;

  if (!f->init) {
    // first invoke
    f->init = init->eval(e);
    i= cast_number(f->init,1);
    z= i->getFloat();
    e->setValue(f->var, f->init);
  } else {
    auto _v= e->getValue(f->var);
    auto v= cast_number(_v,1);
    // do var +/- step
    if (s->isPos()) {
      z = v->getFloat() + s->getFloat(); }
    else
    if (s->isNeg()) {
      z = v->getFloat() - s->getFloat(); }
    // update the var
    if (v->isInt()) {
      e->setValue(f->var, NUMBER_VAL((llong)z)); }
    else {
      e->setValue(f->var, NUMBER_VAL(z)); }
  }
  // test for loop termination
  if (s->isPos()) {
    quit = z > t->getFloat(); }
  else
  if (s->isNeg()) {
      quit = z < t->getFloat(); }
  if (quit)
    _e->endFor(f);
  return quit ? NUMBER_VAL(0) : P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ForLoop::visit(d::IAnalyzer* a) {
  auto vn= DCAST(Var,var)->name();
  auto _a= s__cast(Basic,a);
  var->visit(a);
  init->visit(a);
  term->visit(a);
  step->visit(a);
  _a->addForLoop(ForLoopInfo::make(vn, line(), offset()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ForLoop::pr_str() const {
  return "FOR " + PRSTR(var) +
         " = " + PRSTR(init) +
         " TO " + PRSTR(term) + " STEP " + PRSTR(step);
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
  stdstr buf;
  buf += "IF " + PRSTR(cond) + " THEN " + PRSTR(then);
  return elze ? buf + " ELSE " + PRSTR(elze) : buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Run::Run(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Run::eval(d::IEvaluator*) { return P_NIL; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Run::visit(d::IAnalyzer* a) { }
//
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Restore::Restore(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Restore::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Restore::eval(d::IEvaluator* e) {
  s__cast(Basic,e)->restore();
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
End::End(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue End::eval(d::IEvaluator* e) {
  s__cast(Basic,e)->halt();
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void End::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Data::Data(d::DslToken t, const d::AstVec& vs) : Ast(t) {
  s__ccat(data,vs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Data::eval(d::IEvaluator* e) {
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Data::visit(d::IAnalyzer* a) {
  auto _a = s__cast(Basic,a);
  for (auto& x : data) {
    x->visit(a);
    _a->addData(x->eval(_a));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Data::pr_str() const {
  stdstr b, buf { token()->getStr() };

  for (auto& x : data) {
    if (!b.empty()) b += " , ";
    b += DCAST(Ast,x)->pr_str();
  }

  return b.empty() ? buf : (buf + " " + b);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
GoSubReturn::GoSubReturn(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue GoSubReturn::eval(d::IEvaluator* e) {
  s__cast(Basic,e)->retSub();
  return NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void GoSubReturn::visit(d::IAnalyzer* a) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
GoSub::GoSub(d::DslToken t, d::DslAst e) : Ast(t) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue GoSub::eval(d::IEvaluator* e) {
  //std::cout << "Jumping to subroutine: " << "\n";
  auto _e= s__cast(Basic,e);
  auto res= expr->eval(e);
  auto des= cast_number(res,1);
  //std::cout << "Jumping to subroutine: " << des << "\n";
  _e->jumpSub(des->getInt(), line(), offset());
  return NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void GoSub::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr GoSub::pr_str() const {
  return token()->getStr() + " " + PRSTR(expr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Goto::Goto(d::DslToken t, d::DslAst e) : Ast(t) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Goto::eval(d::IEvaluator* e) {
  auto _e= s__cast(Basic,e);
  auto res= expr->eval(e);
  auto line= cast_number(res,1);
  //std::cout << "Jumping to line: " << line << "\n";
  _e->jump(line->getInt());
  return NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Goto::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Goto::pr_str() const {
  return token()->getStr() + " " + PRSTR(expr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
FuncCall::FuncCall(d::DslAst var, const d::AstVec& pms) {
  fn=var;
  s__ccat(args,pms);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue FuncCall::eval(d::IEvaluator* e) {
  auto n= DCAST(Var,fn)->name();
  auto f= e->getValue(n);
  if (!f)
    RAISE(d::NoSuchVar, "Unknown function/array: %s.", C_STR(n));
  auto fa = cast_array(f,0);
  auto fv = cast_native(f,0);
  if (E_NIL(fa) && E_NIL(fv)) {
    expected("Array var or function", f); }
  //else
  d::ValVec pms;
  for (auto& a : args) {
    s__conj(pms, a->eval(e)); }
  //
  d::VSlice _args(pms);
  return fa
    ? fa->get(_args)
    : (pms.empty() ? fv->invoke(e) : fv->invoke(e,_args));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr FuncCall::pr_str() const {
  stdstr pms, buf { PRSTR(fn) };
  buf += "(";
  for (auto& i : args) {
    if (!pms.empty()) pms += " , ";
    pms += PRSTR(i);
  }
  return buf + pms + ")";
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
  ASSERT(terms.size() > 0,
         "Malformed expr, got %d terms.", (int)terms.size());
  auto i=0;
  auto z=terms.size();
  auto lhs = terms[i]->eval(e);
  auto res= !cast_number(lhs,1)->isZero();
  //just one term?
  if (z==1) { return lhs; }
  if (!res) { return FALSE_VAL(); }
  //
  ++i;
  while (i < z) {
    auto _t = terms[i]->eval(e);
    auto rhs = !cast_number(_t,1)->isZero();
    res= (res && rhs);
    if (res) break;
    ++i;
  }
  return res ? TRUE_VAL() : FALSE_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolTerm::pr_str() const {
  stdstr buf;
  for (auto& i : terms) {
    if (!buf.empty()) buf += " ";
    buf += PRSTR(i);
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
         "Malformed expr, got %d  terms, %d ops.", (int)ts.size(), (int)ops.size());
  s__ccat(terms, ts);
  s__ccat(this->ops, ops);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolExpr::pr_str() const {
  stdstr buf;
  if (terms.size() > 0) {
    buf = PRSTR(terms[0]);
    for (int i=0,pz=ops.size(); i<pz; ++i) {
      buf += " ";
      buf += DCAST(BToken,ops[i])->pr_str();
      buf += " ";
      buf += PRSTR(terms[i+1]);
    }
  }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BoolExpr::eval(d::IEvaluator* e) {
  int z1= terms.size();
  int t1= ops.size();
  ASSERT(z1 == (t1+1),
         "Malformed expr, got %d  terms, %d ops.", z1, t1);
  auto i=0;
  auto lhs= terms[0]->eval(e);
  auto res= !cast_number(lhs,1)->isZero();
  if (z1==1) { return lhs; }
  while (i < t1) {
    auto t= ops[i];
    if (t->type() == T_OR && res) {
      break;
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
  return res ? TRUE_VAL() : FALSE_VAL();
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
  stdstr buf { PRSTR(lhs) };
  buf += " ";
  buf += DCAST(BToken,token())->pr_str();
  buf += " ";
  buf += PRSTR(rhs);
  return "(" + buf + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue RelationOp::eval(d::IEvaluator* e) {
  auto x = lhs->eval(e);
  auto y = rhs->eval(e);
  auto xn= cast_number(x,1);
  auto yn= cast_number(y,1);
  auto ints = xn->isInt() && yn->isInt();
  bool b=0;

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
  return b ? TRUE_VAL() : FALSE_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void RelationOp::visit(d::IAnalyzer* a) {
  lhs->visit(a);
  rhs->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Read::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic, e);
  for (auto& v : vars) {
    auto vn= DCAST(Var,v)->name();
    //std::cout << "reading var = " << vn << "\n";
    auto res= _e->readData();
    ASSERT1(res);
    e->setValue(vn, res);
  }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Read::visit(d::IAnalyzer* a) {
  for (auto& v : vars) {
    v->visit(a);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Read::Read(d::DslToken t, const d::AstVec& v) : Ast(t) {
  s__ccat(vars, v);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NotFactor::NotFactor(d::DslAst e) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr NotFactor::pr_str() const {
  return PRSTR(expr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue NotFactor::eval(d::IEvaluator* e) {
  auto res= expr->eval(e);
  auto i= cast_number(res,1);
  return i->isZero() ? FALSE_VAL() : TRUE_VAL();
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
  return "( " + PRSTR(lhs) + " " +
         DCAST(BToken,token())->pr_str() + " " + PRSTR(rhs) + " )";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(d::IAnalyzer* a) {
  lhs->visit(a);
  rhs->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Num::eval(d::IEvaluator* e) {
  if (token()->type() == d::T_INTEGER) {
    return NUMBER_VAL(token()->getInt());
  } else {
    return NUMBER_VAL(token()->getFloat());
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(d::IAnalyzer* a) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String::String(d::DslToken t) : Ast(t) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr String::pr_str() const {
  return "\"" + token()->pr_str() + "\"";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue String::eval(d::IEvaluator*) {
  return STRING_VAL(token()->getStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void String::visit(d::IAnalyzer*) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(d::DslToken t) : Ast(t) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Var::eval(d::IEvaluator* e) {
  return e->getValue(token()->getStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(d::IAnalyzer*) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(d::DslToken t, d::DslAst e) : Ast(t) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr UnaryOp::pr_str() const {
  return token()->pr_str() + PRSTR(expr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue UnaryOp::eval(d::IEvaluator* e) {
  auto res = expr->eval(e);
  auto n = cast_number(res,1);
  if (token()->type() == d::T_MINUS) {
    if (n->isInt()) {
      res = NUMBER_VAL(- n->getInt()); }
    else {
      res = NUMBER_VAL(- n->getFloat()); } }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(d::IAnalyzer* a) {
  expr->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Print::Print(d::DslToken t, const d::AstVec& es) : Ast(t) {
  s__ccat(exprs, es);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Print::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto lastSemi=false;
  for (auto& i : exprs) {
    auto t= DCAST(Ast,i)->token()->type();
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
      _e->writeString(res->pr_str(0));
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
  stdstr b, buf { token()->getStr() };

  for (auto& i : exprs) {
    if (!b.empty()) b += " ";
    b += PRSTR(i);
  }

  return buf + " " + b;
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
Assignment::Assignment(d::DslAst left, d::DslToken op, d::DslAst right) : Ast(op) {
  lhs=left;
  rhs=right;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Assignment::eval(d::IEvaluator* e) {
  auto t= token()->type();
  auto res= rhs->eval(e);
  if (t == T_ARRAYINDEX) {
    auto fc = DCAST(FuncCall,lhs);
    auto fn = fc->funcName();
    auto args= fc->funcArgs();
    d::ValVec out;
    for (auto& x : args) {
      s__conj(out, x->eval(e)); }
    auto vn = DCAST(Var,fn)->name();
    auto vv= e->getValue(vn);
    auto arr= cast_array(vv,1);
    ensure_data_type(vn,res);
    arr->set(d::VSlice(out), res);
  } else {
    auto vn = DCAST(Var,lhs)->name();
    e->setValue(vn, res);
  }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Assignment::pr_str() const {
  return PRSTR(lhs) + " = " + PRSTR(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a) {
  auto t= token()->type();
  auto v = t == T_ARRAYINDEX
           ? DCAST(FuncCall,lhs)->funcName() : lhs;
  auto vn= DCAST(Var,v)->name();
  if (t == T_ARRAYINDEX) {
    // array must have been defined.
    auto x= a->find(vn);
    ASSERT(x,
           "Expected array var %s, but not found.", C_STR(vn));
  }
  lhs->visit(a);
  rhs->visit(a);
  if (t != T_ARRAYINDEX) {
    a->define(d::Symbol::make(vn)); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArrayDecl::ArrayDecl(d::DslToken t, d::DslAst v, const IntVec& sizes) : Ast(t) {
  auto n= DCAST(Var,v)->name();
  stringType =(n[n.length()-1] == '$');
  var=v;
  s__ccat(ranges,sizes);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ArrayDecl::pr_str() const {
  stdstr b, buf;
  buf = PRSTR(var) + "(";
  for (auto& x : ranges) {
    if (!b.empty()) b += ",";
    b += N_STR(x);
  }
  return buf + b + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ArrayDecl::eval(d::IEvaluator* e) {
  auto n= DCAST(Var,var)->name();
  return e->setValue(n, BArray::make(ranges));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ArrayDecl::visit(d::IAnalyzer* a) {
  auto n= DCAST(Var,var)->name();
  if (auto c= a->find(n); c) {
    RAISE(d::SemanticError,
          "Array var %s defined already.", C_STR(n)); }
  a->define(d::Symbol::make(n, d::Symbol::make("ARRAY")));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Comment::Comment(const d::TokenVec& ts) {
  s__ccat(tkns,ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Comment::eval(d::IEvaluator*) { return P_NIL; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Comment::visit(d::IAnalyzer*) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Comment::pr_str() const {
  stdstr buf;
  for (auto& t : tkns) {
    if (!buf.empty()) buf += " ";
    buf += DCAST(BToken,t)->getStr();
  }
  return "REM " + buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Input::Input(d::DslToken t, d::DslAst var, d::DslAst prompt) : Ast(t) {
  this->var=var;
  this->prompt=prompt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Input::eval(d::IEvaluator* e) {
  auto vn= DCAST(Var,var)->name();
  auto _e= s__cast(Basic,e);
  auto res= _e->readString();
  auto cs= res.c_str();
  d::DslValue v;
  if (vn[vn.size()-1]=='$') {
    v= STRING_VAL(res); }
  else
  if (::strchr(cs, '.')) {
    v= NUMBER_VAL(::atof(cs)); }
  else {
    v= NUMBER_VAL(::atoi(cs)); }

  std::cout << "sfdsfsd " << v->pr_str(1) << "\n";
  return e->setValue(vn,v), P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Input::visit(d::IAnalyzer* a) {
  var->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Input::pr_str() const {
  stdstr buf { token()->getStr() };
  if (prompt) {
    buf += " " + PRSTR(prompt); }
  buf += " ; ";
  return buf + PRSTR(var);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BasicParser::BasicParser(const Tchar* src) {
  lex=new Lexer(src);
  curLine=1;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BasicParser::~BasicParser() { DEL_PTR(lex); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken BasicParser::eat(int wanted) {
  auto t= lex->ctx().cur;
  if (t->type() != wanted) {
    auto i=t->marker();
    RAISE(d::SyntaxError,
          "Expected token %s, got token %s near line %d(%d).",
          C_STR(typeToString(wanted)),
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
  return Assignment::make(lhs, token(n, "[]", t->marker()), val);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst assignment(BasicParser* bp, d::DslToken name) {
  auto t= bp->eat(d::T_EQ);
  auto val= expr(bp);
  return Assignment::make(Var::make(name), t, val);
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
  auto t= bp->eat(T_INPUT);
  d::DslAst prompt;
  if (bp->isCur(d::T_STRING)) {
    prompt= String::make(bp->eat());
    bp->eat(d::T_SEMI);
  }
  return Input::make(t, Var::make(bp->eat(d::T_IDENT)),prompt);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst onXXX(BasicParser* bp) {
  auto c= (bp->eat(T_ON), bp->eat(d::T_IDENT));
  auto m= c->marker();
  if (bp->isCur(T_GOTO) ||
      bp->isCur(T_GOSUB)) {} else {
    RAISE(d::SyntaxError,
          "Expected keyword GOTO/GOSUB near line %d(%d).", m.first, m.second);
  }
  auto g= bp->eat();
  d::TokenVec ts;
  s__conj(ts, bp->eat(d::T_INTEGER));
  while (bp->isCur(d::T_COMMA)) {
    bp->eat();
    s__conj(ts, bp->eat(d::T_INTEGER));
  }
  return OnXXX::make(g,c,ts);
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
    return IfThen::make(_t, c, t, statement(bp));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst forNext(BasicParser* bp) {
  auto t = bp->eat(T_NEXT);
  if (bp->isCur(d::T_IDENT)) {
    auto v = bp->eat(d::T_IDENT);
    return ForNext::make(t,Var::make(v));
  } else {
    return ForNext::make(t);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst forLoop(BasicParser* bp) {
  auto ft= bp->eat(T_FOR);
  auto v= bp->eat(d::T_IDENT);
  bp->eat(d::T_EQ);
  auto b= expr(bp);
  bp->eat(T_TO);
  auto e= expr(bp);
  // force a step = 1 as default
  auto t= BToken::make(d::T_INTEGER, "1", ft->marker());
  DCAST(BToken,t)->setLiteral((llong)1);
  //
  auto s= Num::make(t);
  if (bp->isCur(T_STEP)) {
    s = (bp->eat(), expr(bp)); }
  return ForLoop::make(Var::make(v), b, e, s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst print(BasicParser* bp) {
  auto pt= bp->eat(T_PRINT);
  d::AstVec out;
  while (1) {
    if (bp->isCur(d::T_COLON) ||
        bp->isCur(T_EOL) ||
        bp->isEof()) { break; }
    if (bp->isCur(d::T_SEMI) ||
        bp->isCur(d::T_COMMA)) {
      s__conj(out, PrintSep::make(bp->eat())); }
    else
    if (auto e= expr(bp); e) {
      s__conj(out,e); }
  }
  return Print::make(pt, out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst gotoLine(BasicParser* bp) {
  return Goto::make(bp->eat(T_GOTO), expr(bp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst restore(BasicParser* bp) {
  return Restore::make(bp->eat(T_RESTORE));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst read(BasicParser* bp) {
  auto t= bp->eat(T_READ);
  d::AstVec v;

  s__conj(v, Var::make(bp->eat(d::T_IDENT)));
  while (bp->isCur(d::T_COMMA)) {
    bp->eat();
    s__conj(v, Var::make(bp->eat(d::T_IDENT)));
  }

  return Read::make(t, v);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst data(BasicParser* bp) {
  auto t= bp->eat(T_DATA);
  d::AstVec vs;
  while (1) {
    if (bp->isEof() ||
        bp->isCur(T_EOL) ||
        bp->isCur(d::T_COLON)) {break;}
    auto res= expr(bp);
    s__conj(vs, res);
    if (bp->isCur(d::T_COMMA)) {
      bp->eat();
    }
    //std::cout << "data == " << DCAST(Ast,res)->pr_str() << "\n";
  }
  return Data::make(t, vs);
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
    // a func call, or array access
    if (bp->isCur(d::T_LPAREN)) {
      res = funcall(bp,t); }
    else {
      res= Var::make(t); }
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
  IntVec sizes;
  bp->eat(d::T_LPAREN);
  while (! bp->isEof()) {
    s__conj(sizes, bp->eat(d::T_INTEGER)->getInt());
    if (bp->isCur(d::T_RPAREN)) { break; }
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
  case T_ON:
    res= onXXX(bp);
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
  case T_RESTORE:
    res= restore(bp);
  break;
  case T_READ:
    res= read(bp);
  break;
  case T_DATA:
    res= data(bp);
  break;
  case T_DIM:
    res=declArray(bp);
  break;
  case T_LET:
    bp->eat();
    res=assignment(bp, bp->eat(d::T_IDENT));
  break;
  case d::T_IDENT:
    auto n= bp->eat();
    if (bp->isCur(d::T_EQ)) {
      res=assignment(bp,n); }
    else {
      if (bp->isCur(d::T_LPAREN)) {
        auto fc= funcall(bp, n);
        if (!bp->isCur(d::T_EQ)) {
          res=fc; }
        else { // assignment
          res = assignment(bp, fc); }}
      else {
        RAISE(d::SyntaxError,
              "Unexpected identifier %s.", C_STR(n->pr_str())); }
    }
  break;
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst compound_statements(BasicParser* bp) {
  d::AstVec out;
  bool loop=1;
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
  auto line= -1;

  // usually line number, but can be none.
  if (t->type() == d::T_INTEGER) {
    line = t->getInt();
    bp->eat();
  }

  if (!bp->isEof()) {
    if (bp->isCur(T_EOL)) {
      bp->eat(); }
    else {
      bp->setLine(line);
      res= compound_statements(bp); }
  }

  //std::cout << "parsed line = " << line << "\n";
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst program(BasicParser* bp) {
  std::map<int,d::DslAst> lines;
  d::AstVec raws;
  do {
    if (auto res= parse_line(bp); res) {
      auto n = bp->line();
      if (n < 0)
        s__conj(raws,res);
      else
        lines[n]= res;
    }
  } while (!bp->isEof());
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



