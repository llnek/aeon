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
d::DAst variable(BasicParser*);
d::DAst expr(BasicParser*);
d::DAst b_expr(BasicParser*);
d::DAst statement(BasicParser*);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst mkvar(BasicParser* bp) {
  return Var::make(bp->eat(d::T_IDENT));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(d::DToken k, const std::map<int,d::DAst>& lines) : Ast(k) {
  // lines are sorted, so we get the ordering
  // now store them into a array for fast access.
  auto pos=0;
  for (auto i=lines.begin(), e=lines.end(); i!=e; ++i) {
    mlines[_1_(i)] = pos;
    ++pos;
    s__conj(vlines, _2_(i)); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Program::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto len= vlines.size();
  auto last= DVAL_NIL;

  //std::cout << "len = " << len << "\n" << pr_str() << "\n";
  while (_e->isOn() &&
         _e->incr_pc() < len)
    last = vlines[_e->pc()]->eval(e);

  return last;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(d::IAnalyzer* a) {
  //std::cout << "Program starting visit\n";
  auto _e = s__cast(Basic,a);
  _e->install(mlines);

  for (auto& i : vlines)
  i->visit(a);

  if (auto f= _e->getCurForLoop(); f)
    E_SEMANTIC("Unmatched for-loop at line#%d", f->begin);
  //std::cout << "Program ended visit\n";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Program::pr_str() const {
  stdstr buf;
  for (auto& i : vlines)
  { if (!buf.empty()) buf += "\n";
    buf += N_STR(DCAST(Compound,i)->line()) + " " + PRN(i); }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound(d::DToken k, int ln, const d::AstVec& vs) : Ast(k) {
  s__ccat(stmts,vs);
  line(ln);
  auto pos=0;
  for (auto& s : stmts)
  { DCAST(Ast,s)->line(ln);
    DCAST(Ast,s)->offset(pos++); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Compound::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto len= stmts.size();
  auto pos= _e->poffset();

  //std::cout << "line = " << line() << "\n";
  for (; pos < len; ++pos)
  { auto ps= DCAST(Ast,stmts[pos]);
    auto res= ps->eval(e);
    auto n= vcast<d::Number>(res);
    if (n && n->isZero()) {break;} }

  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Compound::pr_str() const {
  stdstr buf;
  for (auto& i : stmts)
    buf += stdstr(buf.empty()?"":":") + PRN(i);
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
OnXXX::OnXXX(d::DToken t, d::DAst v, d::TokenVec& vs) : Ast(t) {
  var=v;
  for (auto& t : vs)
    s__conj(targets, (int) t->getInt());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue OnXXX::eval(d::IEvaluator* e) {
  auto _e= s__cast(Basic,e);
  auto v= var->eval(e);
  auto _A=tok()->addr();
  auto n= vcast<d::Number>(v,_A);
  auto x= n->getInt();
  auto tz= targets.size();
  auto res= DVAL_NIL;

  //if x is 1, it jumps to the first line in the list;
  //if x is 2, it jumps to the second line, and so on.
  if (x > 0 && x <= tz) {
    // get the selected target
    auto t= tok()->type();
    x= targets[x-1];
    if (t == T_GOTO)
      _e->jump(x);
    else
    if (t== T_GOSUB)
      _e->jumpSub(x, line(), offset());
    res=NUMBER_VAL(0);
  }

  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue ForNext::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  _e->jumpFor(_e->getForLoop(_e->pc(),offset()));
  return NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ForNext::visit(d::IAnalyzer* a) {
  auto _e = s__cast(Basic,a);
  if (!var)
    _e->xrefForNext(line(), offset());
  else
  { var->visit(a);
    _e->xrefForNext(PNAME(Var,var), line(), offset()); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ForNext::pr_str() const {
  stdstr buf { tok()->pr_str() };
  if (var)
    buf += " " + PNAME(Var,var);
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue ForLoop::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto _A= tok()->addr();
  auto P = _e->pc();
  bool quit=1;
  auto f= _e->getForLoop(P,offset());

  //calc step and term
  auto _t= term->eval(e);
  auto _s= step->eval(e);
  auto s= vcast<d::Number>(_s,_A);
  auto t= vcast<d::Number>(_t,_A);
  auto i=t;
  auto z= 0.0;

  // first invoke
  if (!f->init) {
    f->init = init->eval(e);
    i= vcast<d::Number>(f->init,_A);
    z= i->getFloat();
    e->setValue(f->var, f->init);
  } else {
    auto _v= e->getValue(f->var);
    auto v= vcast<d::Number>(_v,_A);
    // do var +/- step
    z = v->getFloat() + s->getFloat();
    // update the var
    e->setValue(f->var,
                v->isInt()
                ? NUMBER_VAL((llong) z) : NUMBER_VAL(z)); }

  // test for loop termination
  if (s->isPos())
    quit = z > t->getFloat();
  if (s->isNeg())
    quit = z < t->getFloat();

  if (quit)
    _e->endFor(f);

  return quit ? NUMBER_VAL(0) : DVAL_NIL;
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
  return stdstr("FOR ") +
         PRN(var) +
         " = " +
         PRN(init) +
         " TO " +
         PRN(term) +
         " STEP " +
         PRN(step);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue IfThen::eval(d::IEvaluator* e) {
  auto c= cond->eval(e);
  auto n= vcast<d::Number>(c,tok()->addr());
  return !n->isZero()
         ? then->eval(e) : (elze ? elze->eval(e) : DVAL_NIL);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr IfThen::pr_str() const {
  stdstr buf;
  buf += stdstr("IF ") +
         PRN(cond) +
         " THEN " +
         PRN(then);
  return elze ? buf + " ELSE " + PRN(elze) : buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Run::eval(d::IEvaluator*) { return DVAL_NIL; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Restore::eval(d::IEvaluator* e) {
  return s__cast(Basic,e)->restore(), DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue End::eval(d::IEvaluator* e) {
  return s__cast(Basic,e)->halt(), DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Data::eval(d::IEvaluator* e) {
  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Data::visit(d::IAnalyzer* a) {
  auto _a = s__cast(Basic,a);
  for (auto& x : data)
  { x->visit(a);
    _a->addData(x->eval(_a)); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Data::pr_str() const {
  stdstr b, buf { tok()->getStr() };

  for (auto& x : data)
    b += stdstr(b.empty()?"":",") + PRN(x);

  return b.empty() ? buf : (buf + " " + b);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue GoSubReturn::eval(d::IEvaluator* e) {
  return s__cast(Basic,e)->retSub(), NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue GoSub::eval(d::IEvaluator* e) {
  //std::cout << "Jumping to subroutine: " << "\n";
  auto _e= s__cast(Basic,e);
  auto res= expr->eval(e);
  auto des= vcast<d::Number>(res,tok()->addr());
  //std::cout << "Jumping to subroutine: " << des << "\n";
  return _e->jumpSub(des->getInt(), line(), offset()), NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr GoSub::pr_str() const {
  return tok()->getStr() + " " + PRN(expr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Goto::eval(d::IEvaluator* e) {
  auto _e= s__cast(Basic,e);
  auto res= expr->eval(e);
  auto line= vcast<d::Number>(res,tok()->addr());
  //std::cout << "Jumping to line: " << line << "\n";
  return _e->jump(line->getInt()), NUMBER_VAL(0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Goto::pr_str() const {
  return tok()->getStr() + " " + PRN(expr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue FuncCall::eval(d::IEvaluator* e) {
  auto pvar= DCAST(Var,fn);
  auto _A=tok()->addr();
  auto n= pvar->name();
  auto f= e->getValue(n);

  if (!f)
    RAISE(d::NoSuchVar, "Unknown function/array: %s", n.c_str());

  auto fv = vcast<LibFunc>(f);
  auto fd = vcast<Lambda>(f);
  auto fa = vcast<BArray>(f);

  if (E_NIL(fa) &&
      E_NIL(fv) && E_NIL(fd))
    expected("Array var or function", f, _A);

  d::ValVec pms;
  for (auto& a : args)
    s__conj(pms, a->eval(e));

  d::VSlice _args(pms);
  auto ff = X_NIL(fd) ? (Function*) fd : (Function*) fv;

  return fa
    ? fa->get(_args)
    : (pms.empty() ? ff->invoke(e) : ff->invoke(e,_args));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr FuncCall::pr_str() const {
  stdstr pms, buf { PRN(fn) };
  buf += "(";
  for (auto& i : args)
    pms += stdstr(pms.empty()?"":",") + PRN(i);
  return buf + pms + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BoolTerm::eval(d::IEvaluator* e) {
  auto _A=tok()->addr();
  auto z=terms.size();
  auto i=0;
  auto ti= terms[i];
  auto lhs = ti->eval(e);
  auto res= !vcast<d::Number>(lhs,_A)->isZero();
  //just one term?
  if (z==1) return lhs;
  if (!res) return FALSE_VAL();
  //
  ++i;
  while (i < z)
  { auto ti= terms[i];
    auto _t = ti->eval(e);
    auto rhs = !vcast<d::Number>(_t,_A)->isZero();
    res= (res && rhs);
    if (res) break; else ++i; }
  return res ? TRUE_VAL() : FALSE_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolTerm::pr_str() const {
  stdstr buf;
  for (auto& i : terms)
    buf += stdstr(buf.empty() ? "" : " ") + PRN(i);
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolExpr::pr_str() const {
  stdstr buf;
  if (terms.size() > 0)
  { buf = PRN(terms[0]);
    for (int i=0,pz=ops.size(); i<pz; ++i)
    { buf += " ";
      buf += PRK(ops[i]);
      buf += " ";
      buf += PRN(terms[i+1]); } }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BoolExpr::eval(d::IEvaluator* e) {
  auto _A=tok()->addr();
  int z1= terms.size();
  int t1= ops.size();
  auto i=0;
  auto ti= terms[i];
  auto lhs= ti->eval(e);
  auto res= !vcast<d::Number>(lhs,_A)->isZero();
  if (z1==1) { return lhs; }
  while (i < t1) {
    auto t= ops[i];
    if (t->type() == T_OR && res) {
      break;
    }
    auto ti=terms[i+1];
    auto _r= ti->eval(e);
    auto rhs= !vcast<d::Number>(_r,_A)->isZero();
    if (t->type() == T_XOR)
      res= (res != rhs);
    else
    if (rhs) { res=true; }
    ++i;
  }
  return res ? TRUE_VAL() : FALSE_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr RelationOp::pr_str() const {
  stdstr buf { PRN(lhs) };
  buf += " ";
  buf += PRK(tok());
  buf += " ";
  buf += PRN(rhs);
  return "(" + buf + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue RelationOp::eval(d::IEvaluator* e) {
  auto _A= tok()->addr();
  auto k= tok()->type();
  auto x = lhs->eval(e);
  auto y = rhs->eval(e);
  auto s1= vcast<d::String>(x);
  auto s2= vcast<d::String>(y);
  if (s1 && s2) {
    switch (k) {
    case d::T_EQ: return NUMBER_VAL(s1->impl()==s2->impl()?1:0);
    case T_NOTEQ: return NUMBER_VAL(s1->impl()==s2->impl()?0:1);
    }
    E_SEMANTIC("Bad op on strings near %s", d::pr_addr(_A).c_str()); }
  // fall through to numbers
  auto xn= vcast<d::Number>(x,_A);
  auto yn= vcast<d::Number>(y,_A);
  auto ints = xn->isInt() && yn->isInt();
  bool b=0;

  switch (k) {
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
stdstr Read::pr_str() const {
  stdstr buf { PRK(tok()) };
  stdstr b;
  for (auto& v : vars)
    b += stdstr(b.empty()?"":" , ") + PRN(v);
  return buf + " " + b;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Read::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic, e);
  auto _A= tok()->addr();
  for (auto& v : vars)
  { auto z= DCAST(Ast,v)->tok()->type();
    auto res= _e->readData();
    if (!res)
      E_SEMANTIC("Can't read data near %s", d::pr_addr(_A).c_str());
    stdstr pv;
    if (z==T_ARRAYINDEX) {
      auto fc= DCAST(FuncCall,v);
      d::ValVec out;
      auto& args= fc->funcArgs();
      for (auto& x : args)
        s__conj(out, x->eval(e));
      auto fcn=fc->funcName();
      pv=PNAME(Var,fcn);
      auto vv= e->getValue(pv);
      auto arr= vcast<BArray>(vv,_A);
      ensure_data_type(pv,res);
      arr->set(d::VSlice(out), res);
    } else {
      pv= PNAME(Var,v);
      e->setValue(pv, res); } }
  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr NotFactor::pr_str() const {
  return PRN(expr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue NotFactor::eval(d::IEvaluator* e) {
  auto res= expr->eval(e);
  auto i= vcast<d::Number>(res,tok()->addr());
  return i->isZero() ? TRUE_VAL() : FALSE_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue BinOp::eval(d::IEvaluator* e) {
  auto _A= tok()->addr();
  auto t= tok()->type();
  auto lf= lhs->eval(e);
  auto rt= rhs->eval(e);
  auto s1= vcast<d::String>(lf);
  auto s2= vcast<d::String>(rt);
  auto n1= vcast<d::Number>(lf);
  auto n2= vcast<d::Number>(rt);

  if (n1 && n2)
    return op_math(lf, t, rt);

  if (s1 && s2 && t==d::T_PLUS)
    return STRING_VAL(s1->impl() + s2->impl());

  E_SEMANTIC("Bad op `%s` near %s",
               typeToString(t).c_str(), d::pr_addr(_A).c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BinOp::pr_str() const {
  return stdstr("( ") +
         PRN(lhs) +
         " " +
         PRK(tok()) +
         " " +
         PRN(rhs) + " )";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Defun::eval(d::IEvaluator* e) {
  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Defun::visit(d::IAnalyzer* a) {
  var->visit(a);
  StrVec vs;

  for (auto& p : params)
  { p->visit(a);
    s__conj(vs, PNAME(Var,p)); }

  auto vn = PNAME(Var,var);
  auto _a = s__cast(Basic, a);
  body->visit(a);
  _a->addLambda(Lambda::make(vn, vs, body));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Num::eval(d::IEvaluator* e) {
  if (tok()->type() == d::T_INT) {
    return NUMBER_VAL(tok()->getInt()); }
  else {
    return NUMBER_VAL(tok()->getFloat()); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr String::pr_str() const {
  return "\"" + tok()->pr_str() + "\"";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue String::eval(d::IEvaluator*) {
  return STRING_VAL(tok()->getStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Var::eval(d::IEvaluator* e) {
  return e->getValue(tok()->getStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr UnaryOp::pr_str() const {
  return tok()->pr_str() + PRN(expr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue UnaryOp::eval(d::IEvaluator* e) {
  auto res = expr->eval(e);
  auto n = vcast<d::Number>(res,tok()->addr());
  if (tok()->type() == d::T_MINUS)
  { if (n->isInt())
      res = NUMBER_VAL(- n->getInt());
    else
      res = NUMBER_VAL(- n->getFloat()); }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Print::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto k= tok()->type();
  auto lastSemi=false;
  for (auto& i : exprs)
  { auto t= DCAST(Ast,i)->tok()->type();
    lastSemi=false;
    if (t == d::T_COMMA)
      _e->writeString(" ");
    else
    if (t == d::T_SEMI)
      lastSemi=true;
    else
    if (auto res= i->eval(e); res)
      _e->writeString(res->pr_str(0)); }

  if (k==T_PRINTLN || ! lastSemi) { _e->writeln(); }

  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Print::pr_str() const {
  stdstr b, buf { tok()->getStr() };
  for (auto& i : exprs)
    b += stdstr(b.empty()?"":" ") + PRN(i);
  return buf + " " + b;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue PrintSep::eval(d::IEvaluator* e) {
  return NUMBER_VAL(tok()->type());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Assignment::eval(d::IEvaluator* e) {
  auto t= DCAST(Ast,lhs)->tok()->type();
  auto _A=tok()->addr();
  auto res= rhs->eval(e);

  if (t == T_ARRAYINDEX) {
    auto fc = DCAST(FuncCall,lhs);
    auto fn = fc->funcName();
    auto& args= fc->funcArgs();
    d::ValVec out;

    for (auto& x : args)
      s__conj(out, x->eval(e));

    auto vn = PNAME(Var,fn);
    auto vv= e->getValue(vn);
    auto arr= vcast<BArray>(vv,_A);
    ensure_data_type(vn,res);
    arr->set(d::VSlice(out), res);
  } else {
    e->setValue(PNAME(Var,lhs), res); }

  return DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Assignment::pr_str() const {
  return PRN(lhs) +
         stdstr(" = ") + PRN(rhs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a) {
  auto t= DCAST(Ast,lhs)->tok()->type();
  auto _A=tok()->addr();
  auto v= t == T_ARRAYINDEX
          ? DCAST(FuncCall,lhs)->funcName() : lhs;
  auto vn= PNAME(Var,v);

  if (t == T_ARRAYINDEX)
  { // array must have been defined.
    auto x= a->find(vn);
    if (!x)
      E_SEMANTIC("Wanted array var %s near %s",
                   vn.c_str(), d::pr_addr(_A).c_str()); }

  lhs->visit(a);
  rhs->visit(a);

  if (t != T_ARRAYINDEX)
    a->define(d::Symbol::make(vn));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArrayDecl::ArrayDecl(d::DToken t, d::DAst v, const IntVec& sizes) : Ast(t) {
  auto n= PNAME(Var,v);
  stringType =(n[n.length()-1] == '$');
  var=v;
  s__ccat(ranges,sizes);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ArrayDecl::pr_str() const {
  stdstr b, buf;
  buf = PRN(var) + stdstr("(");
  for (auto& x : ranges)
    b += stdstr(b.empty()?"":",") + N_STR(x);
  return buf + b + ")";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue ArrayDecl::eval(d::IEvaluator* e) {
  auto n= PNAME(Var,var);
  return e->setValue(n, BArray::make(ranges));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ArrayDecl::visit(d::IAnalyzer* a) {
  auto n= PNAME(Var,var);
  auto _A=tok()->addr();
  if (auto c= a->find(n); c)
    E_SEMANTIC("Duplicate array var %s near %s.",
                 n.c_str(), d::pr_addr(_A).c_str());
  a->define(d::Symbol::make(n, d::Symbol::make("ARRAY")));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Comment::eval(d::IEvaluator*) { return P_NIL; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Comment::pr_str() const {
  stdstr buf;
  for (auto& t : tkns)
    buf += (buf.empty()?"":" ") + DCAST(d::Token,t)->getStr();
  return stdstr(tok()->type()==d::T_QUOTE ? "'" : "REM") + " " + buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Input::eval(d::IEvaluator* e) {
  auto vn= PNAME(Var,var);
  auto _e= s__cast(Basic,e);
  auto res= _e->readString();
  auto cs= res.c_str();
  auto v= DVAL_NIL;

  if (vn[vn.size()-1]=='$')
    v= STRING_VAL(res);
  else
  if (::strchr(cs, '.'))
    v= NUMBER_VAL(::atof(cs));
  else
    v= NUMBER_VAL(::atoi(cs));

  return e->setValue(vn,v), DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Input::pr_str() const {
  stdstr buf { tok()->getStr() };
  if (prompt)
    buf += stdstr(" ") + PRN(prompt) + ";";
  return buf + " " + PRN(var);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BasicParser::BasicParser(const Tchar* src) {
  lex=new Lexer(src);
  curLine=1;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BasicParser::~BasicParser() { DEL_PTR(lex); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken BasicParser::eat(int wanted) {
  auto t= lex->ctx().cur;
  if (t->type() != wanted)
  { auto _A=t->addr();
    E_SYNTAX("Wanted token %s, got %s near %s",
              typeToString(wanted).c_str(), PSTR(t), d::pr_addr(_A).c_str()); }
  return (lex->ctx().cur=lex->getNextToken(), t);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken BasicParser::eat() {
  auto t= lex->ctx().cur;
  return (lex->ctx().cur=lex->getNextToken(), t);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool BasicParser::isEof() const {
  return lex->ctx().cur->type() == d::T_EOF;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst assignment(BasicParser* bp, d::DAst lhs) {
  auto z= DCAST(Ast,lhs)->tok();
  auto t= bp->eat(d::T_EQ);
  auto val= expr(bp);
  if (z->type() != d::T_IDENT)
    //array, mark it
    DCAST(FuncCall,lhs)->setArray(z->addr());
  return Assignment::make(lhs, t, val);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst funcall(BasicParser* bp, d::DAst name) {
  auto t= bp->eat(d::T_LPAREN);
  d::AstVec pms;

  if (!bp->isCur(d::T_RPAREN))
    s__conj(pms, expr(bp));

  while (bp->isCur(d::T_COMMA))
  { bp->eat();
    s__conj(pms, expr(bp)); }

  bp->eat(d::T_RPAREN);
  return FuncCall::make(
      d::Token::make(T_FUNCALL,"()",t->addr()),name, pms);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst skipComment(BasicParser* bp) {
  auto k= bp->eat();
  d::TokenVec tkns;
  while (1)
  { if (bp->isEof() ||
        bp->isCur(T_EOL))
    break;
    s__conj(tkns, bp->eat()); }
  return Comment::make(k, tkns);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst input(BasicParser* bp) {
  auto t= bp->eat(T_INPUT);
  auto _A= t->addr();
  d::DAst prompt;
  if (bp->isCur(d::T_STRING))
  { prompt= String::make(bp->eat());
    if (bp->isCur(d::T_SEMI) ||
        bp->isCur(d::T_COMMA))
      bp->eat();
    else
      E_SYNTAX("Wanted semi-colon near %s", d::pr_addr(_A).c_str());
  }
  return Input::make(t, mkvar(bp),prompt);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst defun(BasicParser* bp) {
  auto t= bp->eat(T_DEF);
  auto fn= bp->eat(d::T_IDENT);
  d::AstVec vs;

  if (bp->isCur(d::T_LPAREN)) {
    bp->eat();
    if (!bp->isCur(d::T_RPAREN))
    { s__conj(vs, mkvar(bp));
      while (bp->isCur(d::T_COMMA))
      { bp->eat();
        s__conj(vs, mkvar(bp)); } }
    bp->eat(d::T_RPAREN);
  } else if (bp->isCur(d::T_EQ)) {
  } else {
    auto _A= t->addr();
    E_SYNTAX("Malformed def near %s", d::pr_addr(_A).c_str()); }

  bp->eat(d::T_EQ);
  return Defun::make(t, Var::make(fn), vs, expr(bp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst onXXX(BasicParser* bp) {
  auto c= (bp->eat(T_ON), bp->eat(d::T_IDENT));
  auto _A= c->addr();
  if (bp->isCur(T_GOTO) ||
      bp->isCur(T_GOSUB)) {} else {
    E_SYNTAX("Wanted GOTO/GOSUB near %s", d::pr_addr(_A).c_str()); }
  auto g= bp->eat();
  d::TokenVec ts;
  s__conj(ts, bp->eat(d::T_INT));
  while (bp->isCur(d::T_COMMA))
  { bp->eat();
    s__conj(ts, bp->eat(d::T_INT)); }
  return OnXXX::make(g,c,ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst gotoInt(BasicParser* bp) {
  auto gs= typeToString(T_GOTO);
  auto k= bp->eat(d::T_INT);
  auto _A=k->addr();
  auto g= d::Token::make(T_GOTO, gs,_A);
  return Goto::make(g, Num::make(k));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst ifThen(BasicParser* bp) {
  auto _t= bp->eat(T_IF);
  auto c= b_expr(bp);
  bp->eat(T_THEN);
  auto t= bp->isCur(d::T_INT)
          ? gotoInt(bp) : statement(bp);
  if (!bp->isCur(T_ELSE))
    return IfThen::make(_t, c,t);
  else {
    bp->eat();
    auto e= bp->isCur(d::T_INT)
            ? gotoInt(bp) : statement(bp);
    return IfThen::make(_t, c, t, e); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst forNext(BasicParser* bp) {
  auto t = bp->eat(T_NEXT);
  if (!bp->isCur(d::T_IDENT))
    return ForNext::make(t);
  else {
    auto v = bp->eat(d::T_IDENT);
    //std::cout << "fornext var= " << v->pr_str() << "\n";
    return ForNext::make(t,Var::make(v)); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst forLoop(BasicParser* bp) {
  auto ft= bp->eat(T_FOR);
  auto v= bp->eat(d::T_IDENT);
  bp->eat(d::T_EQ);
  auto b= expr(bp);
  bp->eat(T_TO);
  auto e= expr(bp);
  // force a step = 1 as default
  auto t= d::Token::make("1", ft->addr(), (llong)1);
  //
  auto s= Num::make(t);
  if (bp->isCur(T_STEP))
    s = (bp->eat(), expr(bp));
  return ForLoop::make(ft, Var::make(v), b, e, s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst print(BasicParser* bp, bool newline) {
  auto pt= bp->eat();
  d::AstVec out;
  while (1)
  { if (bp->isCur(d::T_COLON) ||
        bp->isCur(T_EOL) ||
        bp->isEof()) { break; }
    if (bp->isCur(d::T_SEMI) ||
        bp->isCur(d::T_COMMA))
      s__conj(out, PrintSep::make(bp->eat()));
    else
    if (auto e= expr(bp); e) { s__conj(out,e); } }
  return Print::make(pt, out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst gotoLine(BasicParser* bp) {
  return Goto::make(bp->eat(T_GOTO), expr(bp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst restore(BasicParser* bp) {
  return Restore::make(bp->eat(T_RESTORE));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst read(BasicParser* bp) {
  auto t= bp->eat(T_READ);
  d::AstVec v;

  s__conj(v, variable(bp));

  while (bp->isCur(d::T_COMMA))
  { bp->eat();
    s__conj(v, variable(bp)); }

  return Read::make(t, v);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst data(BasicParser* bp) {
  auto t= bp->eat(T_DATA);
  d::AstVec vs;
  while (1)
  { if (bp->isEof() ||
        bp->isCur(T_EOL) ||
        bp->isCur(d::T_COLON)) {break;}
    auto res= expr(bp);
    s__conj(vs, res);
    if (bp->isCur(d::T_COMMA)) { bp->eat(); } }
  return Data::make(t, vs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst gosub(BasicParser* bp) {
  return GoSub::make(bp->eat(T_GOSUB), expr(bp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst returnSub(BasicParser* bp) {
  return GoSubReturn::make(bp->eat(T_RETURN));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst runProg(BasicParser* bp) {
  return Run::make(bp->eat(T_RUN));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst endProg(BasicParser* bp) {
  return End::make( bp->eat(T_END));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst relation(BasicParser* bp) {
  static std::set<int> ops1 { d::T_GT, d::T_LT, T_GTEQ };
  static std::set<int> ops2 { T_LTEQ, d::T_EQ, T_NOTEQ };
  auto res = expr(bp);
  while (s__contains(ops1, bp->cur()) ||
         s__contains(ops2, bp->cur()) )
    res= RelationOp::make(res, bp->eat(), expr(bp));
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst b_factor(BasicParser* bp) { return relation(bp); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst not_factor(BasicParser* bp) {
  if (!bp->isCur(T_NOT))
    return b_factor(bp);

  return NotFactor::make(bp->eat(), b_factor(bp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst b_term(BasicParser* bp) {
  d::AstVec res { not_factor(bp) };
  auto k= DCAST(Ast,res[0])->tok();
  while (bp->isCur(T_AND))
  { bp->eat();
    s__conj(res, not_factor(bp)); }
  return BoolTerm::make(k, res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst b_expr(BasicParser* bp) {
  static std::set<int> ops {T_OR, T_XOR};
  d::AstVec res { b_term(bp) };
  d::TokenVec ts;
  auto k= DCAST(Ast,res[0])->tok();
  while (s__contains(ops, bp->cur()))
  { s__conj(ts, bp->tok());
    bp->eat();
    s__conj(res, b_term(bp)); }
  return BoolExpr::make(k, res, ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst variable(BasicParser* bp) {
  auto t= mkvar(bp);
  // a func call, or array access
  if (!bp->isCur(d::T_LPAREN)) {
    return t;
  } else {
    auto m=DCAST(Ast,t)->tok()->addr();
    auto fc= funcall(bp,t);
    DCAST(FuncCall,fc)->setArray(m);
    return fc;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst factor(BasicParser* bp) {
  auto t= bp->tok();
  d::DAst res;
  switch (t->type()) {
  case d::T_PLUS:
  case d::T_MINUS:
    res= (bp->eat(), UnaryOp::make(t, factor(bp)));
  break;
  case d::T_REAL:
  case d::T_INT:
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
    res= variable(bp);
  break;
  default:
    E_SYNTAX("Bad expression near %s",
             d::pr_addr(t->addr()).c_str());
  break;
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst term2(BasicParser* bp) {
  static std::set<int> ops { T_POWER };
  auto res= factor(bp);

  //res = d::DAst(new BinOp(res, bp->eat(), factor(bp)));
  //handles right associativity
  while (s__contains(ops,bp->cur()))
    res = BinOp::make(res, bp->eat(), term2(bp));

  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst term(BasicParser* bp) {
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV, T_MOD};
  auto res= term2(bp);
  while (s__contains(ops,bp->cur()))
    res = BinOp::make(res, bp->eat(), term2(bp));
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst expr(BasicParser* bp) {
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  auto res= term(bp);
  while (s__contains(ops,bp->cur()))
    res= BinOp::make(res, bp->eat(), term(bp));
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst declArray(BasicParser* bp) {
  auto _t = bp->eat(T_DIM);
  auto t= bp->eat(d::T_IDENT);
  IntVec sizes;
  bp->eat(d::T_LPAREN);
  while (! bp->isEof())
  { s__conj(sizes, bp->eat(d::T_INT)->getInt());
    if (bp->isCur(d::T_RPAREN))
    break;
    bp->eat(d::T_COMMA); }
  bp->eat(d::T_RPAREN);
  return ArrayDecl::make(_t, Var::make(t), sizes);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst statement(BasicParser* bp) {
  d::DAst res;
  switch (bp->cur()) {
  case T_DEF:
    res= defun(bp);
  break;
  case d::T_QUOTE:
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
    res= print(bp,0);
  break;
  case T_PRINTLN:
    res= print(bp,1);
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
  case T_LET: {
    auto t= bp->eat();
    auto _A=t->addr();
    if (!bp->isCur(d::T_IDENT))
      E_SYNTAX("Bad LET stmt near %s.", C_STR(d::pr_addr(_A)));
    res=statement(bp);
    if (!DCAST(Ast,res)->tok()->type(d::T_EQ))
      E_SYNTAX("Bad LET stmt near %s.", C_STR(d::pr_addr(_A)));
  }
  break;
  case d::T_IDENT: {
    auto n= bp->eat();
    if (bp->isCur(d::T_EQ))
      res=assignment(bp, Var::make(n));
    else {
      if (bp->isCur(d::T_LPAREN))
      { auto fc= funcall(bp, Var::make(n));
        if (!bp->isCur(d::T_EQ))
          res=fc;
        else
          res = assignment(bp, fc); }
      else {
        E_SYNTAX("Unexpected id %s.", C_STR(n->pr_str())); } }
  }
  break;
  default: {
    auto t= bp->tok();
    E_SYNTAX("Bad statement near %s", d::pr_addr(t->addr()).c_str());
  }
  break;
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst compound_statements(BasicParser* bp) {
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
  auto k=DCAST(Ast,out[0])->tok();
  return Compound::make(k,bp->line(), out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst parse_line(BasicParser* bp) {

  auto t= bp->tok();
  d::DAst res;
  auto line= -1;

  // usually line number, but can be none.
  if (t->type() == d::T_INT) {
    line = t->getInt();
    bp->eat();
  }

  if (!bp->isEof())
  { if (bp->isCur(T_EOL))
      bp->eat();
    else {
      bp->setLine(line);
      res= compound_statements(bp); } }

  //std::cout << "parsed line = " << line << "\n";
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst program(BasicParser* bp) {
  std::map<int,d::DAst> lines;
  d::AstVec raws;
  do {
    if (auto res= parse_line(bp); res)
    { auto n = bp->line();
      if (n < 0)
        s__conj(raws,res); else lines[n]= res; }
  } while (!bp->isEof());
  //std::cout << "parsed ALL lines, total count = " << lines.size() << "\n";
  return Program::make(d::Token::make(T_PROGRAM,"<>",DMARK(1,1)), lines);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DAst BasicParser::parse() {
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
d::DToken BasicParser::tok() {
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



