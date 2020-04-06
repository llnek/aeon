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

#include <iostream>
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
  _line =0;
  _token=new Token(d::T_ETHEREAL,"?", s__pair(int,int,0,0));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(const std::map<llong,d::DslAst>& lines) {
  auto pos=0;
  for (auto i=lines.begin(), e=lines.end(); i!=e;++i) {
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
  auto _e = s__cast(Basic,a);
  _e->installProgram(mlines);
  for (auto& i : vlines) {
    i->visit(a);
  }
  auto f= _e->getCurForLoop();
  if (f.isSome())
    RAISE(d::SemanticError,
          "Unmatched for-loop near line %d.\n", (int) f->begin);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Program::pr_str() const {
  stdstr buf;
  for (auto& i : vlines) {
    auto c= s__cast(Compound,i.ptr());
    buf += std::to_string(c->line());
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
  for (auto& s : stmts) {
    AST(s)->line(ln);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Compound::eval(d::IEvaluator* e) {
  d::DslValue last;
  for (auto& i : stmts) {
    last= i->eval(e);
  }
  return last;
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
  _e->xrefForNext(n, line());
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
  auto f= _e->getForLoop(P);
  if (f->init.isNull()) {
    // first invoke
    f->init = this->init->eval(e);
    cast_number(f->init,1);
    e->setValue(f->var, f->init);
    f->step = this->step->eval(e);
    auto i= cast_number(f->init,1);
    auto s= cast_number(f->step,1);
    auto t= cast_number(term->eval(e),1);
    auto quit=true;
    if (s->isPos()) {
      quit = (i->getFloat() > t->getFloat());
    } else if (s->isNeg()) {
      quit = (i->getFloat() < t->getFloat());
    } else {
      RAISE(d::SemanticError, "Bad for-loop step.%s", "\n");
    }
    if (quit) {
      _e->endFor(f);
    } else {
      // into for loop
    }
  } else {
    auto v= cast_number(e->getValue(f->var),1);
    auto s= cast_number(f->step,1);
    auto t= cast_number(term->eval(e),1);
    auto quit=true;
    auto z = 0.0;
    if (s->isPos()) {
      z = v->getFloat() + s->getFloat();
      quit = z > t->getFloat();
    } else if (s->isNeg()) {
      z = v->getFloat() - s->getFloat();
      quit = z < t->getFloat();
    }
    if (quit) {
      _e->endFor(f);
    } else {
      if (v->isInt()) {
        e->setValue(f->var, NUMBER_VAL((llong)z));
      } else {
        e->setValue(f->var, NUMBER_VAL(z));
      }
    }
  }

  return NULL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ForLoop::visit(d::IAnalyzer* a) {
  auto vn= AST(var)->token()->getLiteralAsStr();
  auto info = DslForLoop(new ForLoopInfo(vn, line()));
  auto _e = s__cast(Basic,a);
  var->visit(a);
  init->visit(a);
  term->visit(a);
  step->visit(a);
  _e->addForLoop(info);
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
IfThen::IfThen(d::DslAst c, d::DslAst t, d::DslAst z) {
  cond=c;
  then=t;
  elze=z;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IfThen::IfThen(d::DslAst c, d::DslAst t) {
  cond=c;
  then=t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue IfThen::eval(d::IEvaluator* e) {
  auto c= cond->eval(e);
  auto n= cast_number(c,1);
  return !n->isZero()
      ? then->eval(e) : (elze.isSome() ? elze->eval(e) : NULL);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void IfThen::visit(d::IAnalyzer* a) {
  cond->visit(a);
  then->visit(a);
  if (elze.isSome()) elze->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr IfThen::pr_str() const {
  stdstr buf { "IF" };

  buf += " ";
  buf += AST(cond)->pr_str();
  buf += " THEN ";
  buf += AST(then)->pr_str();
  if (elze.isSome()) {
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
  return NULL;
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
  auto _e = s__cast(Basic,e);
  _e->halt();
  return NULL;
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
  auto _e = s__cast(Basic,e);
  _e->retSub();
  return NULL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void GoSubReturn::visit(d::IAnalyzer* a) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr GoSubReturn::pr_str() const {
  return token()->getLiteralAsStr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
GoSub::GoSub(d::DslAst e) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue GoSub::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto res= expr->eval(e);
  auto line= cast_number(res,1)->getInt();
  //std::cout << "Jumping to subroutine@line: " << line << "\n";
  return NUMBER_VAL(_e->jumpSub(line));
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
Goto::Goto(d::DslAst e) {
  expr=e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Goto::eval(d::IEvaluator* e) {
  auto _e = s__cast(Basic,e);
  auto res= expr->eval(e);
  auto line= cast_number(res,1)->getInt();
  //std::cout << "Jumping to line: " << line << "\n";
  return NUMBER_VAL(_e->jump(line));
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
  if (f.isNull())
    RAISE(d::NoSuchVar, "Unknown function: %s", n.c_str());
  auto fv = cast_native(f,1);
  d::ValVec pms;
  for (auto& a : args) {
    s__conj(pms, a->eval(e));
  }

  return pms.empty() ? fv->invoke(e) : fv->invoke(e,d::VSlice(pms));
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
  d::DslValue res;
  for (auto & i : terms) {
    res= i->eval(e);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolTerm::pr_str() const {
  stdstr buf;
  for (auto& i : terms) {
    if (!buf.empty()) buf += " , ";
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
  ASSERT1(ts.size() == (ops.size()+1));
  s__ccat(terms, ts);
  s__ccat(this->ops, ops);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BoolExpr::pr_str() const {
  if (terms.size()==0) { return ""; }
  stdstr buf { AST(terms[0])->pr_str() };
  for (int i=0,pz=ops.size();i<pz; ++i) {
    buf += " ";
    buf += TKN(ops[i])->pr_str();
    buf += " ";
    buf += AST(terms[i+1])->pr_str();
  }
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue BoolExpr::eval(d::IEvaluator* e) {
  d::DslValue res;
  for (auto& i : terms) {
    res= i->eval(e);
  }
  return res;
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
  auto x = cast_number(lhs->eval(e),1);
  auto y = cast_number(rhs->eval(e),1);
  auto ints = x->isInt() && y->isInt();
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
           ? x->getInt() >= y->getInt()
           : x->getFloat() >= y->getFloat();
    break;
    case T_LTEQ:
      b= ints
           ? x->getInt() <= y->getInt()
           : x->getFloat() <= y->getFloat();
    break;
    case d::T_GT:
      b= ints
           ? x->getInt() > y->getInt()
           : x->getFloat() > y->getFloat();
    break;
    case d::T_LT:
    b= ints
         ? x->getInt() < y->getInt()
         : x->getFloat() < y->getFloat();
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
  auto lf= cast_number(lhs->eval(e));
  auto rt= cast_number(rhs->eval(e));
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
stdstr Var::pr_str() const {
  return TKN(token())->pr_str();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Var::eval(d::IEvaluator* e) {
  auto n= token()->getLiteralAsStr();
  return e->getValue(n);
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
    if (auto res= i->eval(e); res.isSome()) {
      _e->writeString(res->pr_str(1));
    }
  }

  if (! lastSemi) {
    _e->writeln();
  }

  return NULL;
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
  auto n= CAST(Var,lhs)->name();
  return e->setValue(n, rhs->eval(e));
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
  lhs->visit(a);
  rhs->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ArrayDecl::ArrayDecl(d::DslAst v, const std::vector<llong>& sizes) {
  auto n= CAST(Var,v)->token()->getLiteralAsStr();
  if (n[n.length()-1] == '$') {
    stringType=true;
  } else {
    stringType=false;
  }
  var=v;
  s__ccat(dims,sizes);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr ArrayDecl::pr_str() const {
  stdstr buf;
  stdstr b;
  buf += AST(var)->pr_str();
  buf += "(";
  for (auto& x : dims) {
    if (!b.empty()) b += ",";
    b += std::to_string(x);
  }
  buf += b;
  buf += ")";
  return buf;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue ArrayDecl::eval(d::IEvaluator*) {
  return NULL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ArrayDecl::visit(d::IAnalyzer* a) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Comment::Comment(const std::vector<d::DslToken>& ts) {
  s__ccat(tkns,ts);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Comment::eval(d::IEvaluator*) {
  return NULL;
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
  return NULL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Input::visit(d::IAnalyzer* a) {
  var->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Input::pr_str() const {
  stdstr buf { "INPUT" };
  if (prompt.isSome()) {
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
BasicParser::~BasicParser() {
  DEL_PTR(lex);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken BasicParser::getEthereal() {
  return new Token(d::T_ETHEREAL, "?",
                   s__pair(int,int,lex->ctx().line, lex->ctx().col));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken BasicParser::eat(int wanted) {
  auto t= lex->ctx().cur;
  if (t->type() != wanted) {
    RAISE(d::SyntaxError,
          "Expected token %s, got token %s near line %d(%d).\n",
          C_STR(Token::typeToString(wanted)),
          C_STR(t->pr_str()),
          t->srcInfo().first, t->srcInfo().second);
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
d::DslAst assignment(BasicParser* bp) {
  auto name= bp->eat(d::T_IDENT);
  auto t= bp->eat(d::T_EQ);
  auto val= expr(bp);
  return new Assignment(new Var(name),t, val);
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

  return (bp->eat(d::T_RPAREN), new FuncCall(new Var(name), pms));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst skipComment(BasicParser* bp) {
  std::vector<d::DslToken> tkns;
  bp->eat(T_REM);
  while (1) {
    if (bp->isEof() || bp->isCur(T_EOL))
    break;
    s__conj(tkns, bp->eat());
  }
  return new Comment(tkns);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst input(BasicParser* bp) {
  d::DslAst prompt;
  bp->eat(T_INPUT);
  if (bp->isCur(d::T_STRING)) {
    prompt=new String(bp->eat());
  }
  bp->eat(d::T_SEMI);
  return new Input(new Var(bp->eat(d::T_IDENT)),prompt);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst ifThen(BasicParser* bp) {
  auto c= (bp->eat(T_IF), b_expr(bp));
  auto t= (bp->eat(T_THEN), statement(bp));
  if (!bp->isCur(T_ELSE)) {
    return new IfThen(c,t);
  } else {
    bp->eat();
    return new IfThen(c,t, statement(bp));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst forNext(BasicParser* bp) {
  auto v = (bp->eat(T_NEXT),bp->eat(d::T_IDENT));
  return new ForNext(new Var(v));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst forLoop(BasicParser* bp) {
  auto v= (bp->eat(T_FOR),bp->eat(d::T_IDENT));
  auto b= (bp->eat(d::T_EQ), expr(bp));
  auto e= (bp->eat(T_TO), expr(bp));
  // force a step = 1 as default
  auto t= new Token(d::T_INTEGER, "1", s__pair(llong,llong,0,0));
  t->impl().num.setInt(1);
  auto s= d::DslAst(new Num(t));
  if (bp->isCur(T_STEP)) {
    s = (bp->eat(), expr(bp));
  }
  return new ForLoop(new Var(v), b, e, s);
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
      s__conj(out, new PrintSep(bp->eat()));
    }
    else
    if (auto e= b_expr(bp); e.isSome()) {
      s__conj(out,e);
    }
  }
  return new Print(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst gotoLine(BasicParser* bp) {
  bp->eat(T_GOTO);
  return new Goto(expr(bp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst gosub(BasicParser* bp) {
  bp->eat(T_GOSUB);
  return new GoSub(expr(bp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst returnSub(BasicParser* bp) {
  return new GoSubReturn(bp->eat(T_RETURN));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst runProg(BasicParser* bp) {
  return new Run(bp->eat(T_RUN));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst endProg(BasicParser* bp) {
  return new End( bp->eat(T_END));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define LEFT -1
#define RIGHT 1

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int assoc_rank(d::DslToken t) {
  switch (t->type()) {
    case d::T_LPAREN:
    case d::T_LBRACKET:  return LEFT;

    case d::T_HAT: return RIGHT;
    case d::T_PLUS:
    case d::T_MINUS: return RIGHT;

    case d::T_MULT:
    case d::T_DIV:
    case T_INT_DIV:
    case T_MOD:  return LEFT;

    //case d::T_PLUS:
    //case d::T_MINUS: return LEFT;

    case d::T_EQ:
    case T_NOTEQ:
    case T_LTEQ:
    case T_GTEQ: return LEFT;

    case T_NOT: return RIGHT;
    case T_AND: return LEFT;
    case T_OR: return LEFT;
    case T_XOR:  return LEFT;
  }
  return LEFT;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define HIGH_PREC 9999
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int prec_rank(d::DslToken t) {
  switch (t->type()) {
    case d::T_LPAREN:
    case d::T_LBRACKET: return 1000;

    case d::T_HAT: return 800;
    case d::T_PLUS: return 700;
    case d::T_MINUS: return 700;

    case d::T_MULT:  return 600;
    case d::T_DIV:  return 600;
    case T_INT_DIV:  return 600;
    case T_MOD:  return 600;

    //case d::T_PLUS: return 500;
    //case d::T_MINUS: return 500;
    //
    case d::T_EQ:
    case T_NOTEQ:
    case T_LTEQ:
    case T_GTEQ: return 400;

    case T_NOT: return 300;
    case T_AND: return 200;
    case T_OR:
    case T_XOR:  return 100;
  }
  return -1;
}

/*
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst expr(BasicParser* bp, int minPrec) {
  auto result = computeAtom(bp);
  auto cur = bp->token();
  auto prec= prec_rank(cur);
  while (prec >= minPrec) {
    auto assoc= assoc_rank(cur);
    auto nextMinPrec = (assoc == LEFT) ? (prec + 1) : prec;
    auto rhs =expr(bp, nextMinPrec);
    result = computeOp(result, rhs)
  }
  return result;
}
*/

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst relation(BasicParser* bp) {
  static std::set<int> ops1 { d::T_GT, d::T_LT, T_GTEQ };
  static std::set<int> ops2 { T_LTEQ, d::T_EQ, T_NOTEQ };
  auto res = expr(bp);
  while (s__contains(ops1, bp->cur()) ||
         s__contains(ops2, bp->cur()) ) {
    res= d::DslAst(new RelationOp(res, bp->eat(), expr(bp)));
  }
  return res;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst b_factor(BasicParser* bp) {
  return relation(bp);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst not_factor(BasicParser* bp) {
  if (bp->isCur(T_NOT)) {
    bp->eat();
    return d::DslAst(new NotFactor(b_factor(bp)));
  } else {
    return b_factor(bp);
  }
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst b_term(BasicParser* bp) {
  d::AstVec res { not_factor(bp) };
  while (bp->isCur(T_AND)) {
    s__conj(res, not_factor(bp));
  }
  return new BoolTerm(res);
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
  return new BoolExpr(res, ts);
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst factor(BasicParser* bp) {
  auto t= bp->token();
  d::DslAst res;
  switch (t->type()) {
    case d::T_PLUS:
    case d::T_MINUS:
      res= (bp->eat(), new UnaryOp(t, factor(bp)));
    break;
    case d::T_REAL:
    case d::T_INTEGER:
      res= (bp->eat(), new Num(t));
    break;
    case d::T_STRING:
      res= (bp->eat(), new String(t));
    break;
    case d::T_LPAREN:
      res= (bp->eat(), b_expr(bp));
      bp->eat(d::T_RPAREN);
    break;
    case d::T_IDENT:
      bp->eat();
      if (bp->isCur(d::T_LPAREN)) {
        // a func call
        res = funcall(bp,t);
      } else {
        res= new Var(t);
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
    res = d::DslAst(new BinOp(res, bp->eat(), term2(bp)));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst term(BasicParser* bp) {
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV, T_MOD};
  auto res= term2(bp);
  while (s__contains(ops,bp->cur())) {
    res = d::DslAst(new BinOp(res, bp->eat(), term2(bp)));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst expr(BasicParser* bp) {
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  auto res= term(bp);
  while (s__contains(ops,bp->cur())) {
    res= d::DslAst(new BinOp(res, bp->eat(), term(bp)));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst declArray(BasicParser* bp) {
  auto t= (bp->eat(T_DIM),bp->eat(d::T_IDENT));
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
  return new ArrayDecl(new Var(t),sizes);
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
      res=(bp->eat(),assignment(bp));
    break;

    case d::T_IDENT:
      res=assignment(bp);
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
        loop=false;
        bp->eat();
      break;

      default:
        s__conj(out, statement(bp));
      break;
    }
  }

  return new Compound(bp->line(), out);
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
    if (bp->isCur(T_EOL)) { bp->eat(); } else {
      bp->setLine(line);
      res= compound_statements(bp);
    }
  }

  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslAst program(BasicParser* bp) {
  std::map<llong,d::DslAst> lines;
  do {
    auto res= parse_line(bp);
    if (res.isSome())
      lines[bp->line()]= res;
  }
  while (!bp->isEof());

  return new Program(lines);
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



