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
#include "builtins.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::elle {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue evalAst(Scheme* s, d::DFrame env, d::DValue ast) {
  return DCAST(SValue,ast)->eval(s,env);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SMacro* maybeMacro(Scheme* s, d::DFrame env, d::DValue ast) {
  if (auto p= vcast<SPair>(ast); p)
    if (auto sym= vcast<SSymbol>(p->head()); sym)
      if (auto f= env->search(sym->impl(),env); f)
        return vcast<SMacro>(sym->eval(s, f));
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue macroExpand(Scheme* s, d::DFrame env, d::DValue ast) {
  for (auto m = maybeMacro(s, env,ast); m;) {
    auto seq= vcast<SPair>(ast);
    d::ValVec args;
    appendAll(seq,1,args);
    ast = args.empty()
          ? m->invoke(s)
          : m->invoke(s, d::VSlice(args));
    m = maybeMacro(s, env, ast);
  }
  return ast;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue syntaxQuote(Scheme* s, d::DFrame env, d::DValue ast) {
  auto _A= DCAST(SValue,ast)->addr();
  auto seq = vcast<SPair>(ast);

  if (!seq) {
    // `a => 'a
    d::ValVec out{SSymbol::make("quote"), ast};
    return makeList(_A, out);
  }

  // work on the first element AST[0]
  // `(ast0 a b)

  // deal with unquote => ~x or ~(a b c)
  auto ast0= seq->head();
  if (auto s = vcast<SSymbol>(ast0); s && s->impl() == "unquote") {
    // `~x or `~(a b c)
    ASSERT1(2== count(seq));
    return nth(seq,1);
  }

  auto s2 = vcast<SPair>(ast0);
  SSymbol* b = P_NIL;
  d::ValVec out;

  if (s2)
    b = vcast<SSymbol>(s2->head());

  if (b && b->impl() == "splice-unquote") {
    // ~@x  or ~@(a b c)
    ASSERT1(2== count(s2));
    s__conj(out, SSymbol::make("append"));
    s__conj(out, nth(s2,1));
    s__conj(out, syntaxQuote(s, env,seq->tail()));
  } else {
    // else just simple case `(a b c (d e f))
    // look again, once at a time
    s__conj(out, SSymbol::make("cons"));
    s__conj(out, syntaxQuote(s, env, ast0));
    s__conj(out, syntaxQuote(s, env, seq->tail()));
  }

  return makeList(_A,out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Scheme::PRINT(d::DValue v) {
  return DCAST(SValue,v)->pr_str(1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue evalCond(Scheme* S, d::DFrame env, d::DValue code) {
  // (cond (a b) (c d) (else r))
  auto p=vcast<SPair>(code);
  while (p) {
    auto c = nth(code,0);
    auto k= car(c);
    auto res= DVAL_NIL;
    c= rest(c);
    if (auto s = vcast<SSymbol>(k); s && s->impl()=="else") {
      res= STrue::make();
    }
    else
    if (auto t = vcast<STrue>(k); t) {
      res=k;
    } else {
      res= S->EVAL(k,env);
    }
    if (truthy(res)) {
      d::ValVec out;
      //(a)
      if (isNil(c)) {
        s__conj(out, SSymbol::make("quote"));
        s__conj(out,res);
        return makeList(out);
      }
      //(a => xxx)
      k=car(c);
      if (auto s=vcast<SSymbol>(k); s && s->impl()=="=>") {
        s__conj(out, SSymbol::make("quote"));
        s__conj(out,res);
        c=rest(c);
        d::ValVec args{ car(c), makeList(out)};
        return makeList(args);
      }
      //else
      s__conj(out, SSymbol::make("begin"));
      s__conj(out, car(c));
      return makeList(out);
    }
    code=rest(code);
    p=vcast<SPair>(code);
  }
  return code;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
StrVec to_params(d::DValue v) {
  auto pms= vcast<SPair>(v);
  StrVec vs;
  while (pms) {
    auto h= pms->head();
    auto t= pms->tail();
    auto s= vcast<SSymbol>(h);
    s__conj(vs, s->impl());
    s=vcast<SSymbol>(t);
    pms=P_NIL;
    if (s) {
      // a dotted end, varargs
      s__conj(vs, stdstr("."));
      s__conj(vs, s->impl());
    } else {
      pms= vcast<SPair>(t);
    }
  }
  //std::cout << "size of args = " << vs.size() << "\n";
  //for (auto& x : vs) std::cout << "pms = " << x << "\n";
  return vs;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue wrapAsDo(d::DValue body) {
  auto p= vcast<SPair>(body);
  auto len= count(p);
  return len < 2
    ? p->head() : SPair::make(p->addr(), SSymbol::make("begin"),body);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Scheme::EVAL(d::DValue ast, d::DFrame env) {
  while (true) {

    DEBUG("eval(ast)=%s", ast->pr_str(1).c_str());

    auto list = vcast<SPair>(ast);
    if (!list ||
        !listQ(list))
      return evalAst(this, env,ast);

    ast = macroExpand(this, env, ast);
    list = vcast<SPair>(ast);
    if (!list ||
        !listQ(list))
      return evalAst(this, env,ast);

    DEBUG("eval(2)=%s", list->pr_str(1).c_str());

    if (auto op = vcast<SSymbol>(list->head())) {
      auto len = count(list);
      auto code = op->impl();

      DEBUG("op= %s, len()= %d", op->impl().c_str(), len);

      if (code == "cond") {
        ast= evalCond(this, env, rest(list));
        continue;
      }

      if (code == "define") {
        d::preMin(3, len, "define");
        auto var= vcast<SSymbol>(nth(list,1));
        if (var)
          return env->set(var->impl(), EVAL(nth(list,2), env));
        //else (define (name p1 p2)
        //               (some body))
        auto func= nth(list,1);
        auto body= rest(rest(list));
        auto pms= to_params(rest(func));
        var= vcast<SSymbol>(car(func));
        auto fn= var->impl();
        return env->set(fn,
            SLambda::make(fn, pms, wrapAsDo(body), env));
      }

      if (code == "let" || code == "let*") {
        // (let ((a 1)(b2))
        //        (+ a b))
        d::preMin(2, len, "let");
        auto args= nth(list,1);
        auto pargs= vcast<SPair>(args);
        auto body= rest(rest(list));
        auto cnt= d::preEven(count(pargs), "let bindings");
        auto f= d::Frame::make("let", env);
        for (auto i= 0; i < cnt; ++i) {
          auto a=nth(args,i);
          auto n=vcast<SSymbol>(car(a));
          auto e=car(rest(a));
          if (code == "let*")
            f->set(n->impl(), EVAL(e, f));
          else // let
            f->set(n->impl(), EVAL(e, env));
        }
        ast = wrapAsDo(body);
        env = f;
        continue;
      }

      if (code == "quote") {
        d::preEqual(2, len, "quote");
        return vcast<SPair>(list->tail())->head();
      }

      if (code == "syntax-quote") {
        d::preEqual(2, len, "syntax-quote");
        ast = syntaxQuote(this, env, vcast<SPair>(list->tail())->head());
        continue;
      }

      if (code == "define-macro") {
        d::preMin(3, len, "define-macro");
        auto a2=nth(list,1);
        if (auto s = vcast<SSymbol>(a2); s) {
          if (len == 3) {
            // (define-macro name (lambda ...))
            auto func=nth(list,2);
            auto e2= nth(func,1);
            auto s2= vcast<SSymbol>(e2);
            auto pms= s2 ? StrVec{".", s2->impl()} : to_params(e2);
            auto body=rest(rest(func));
            auto pb=vcast<SPair>(body);
            return env->set(s->impl(),
                SMacro::make(s->impl(), pms, pb->head(), env));
          }
          //(define-macro name (p1 . p2) (some stuff))
          auto pms= to_params(nth(list,2));
          auto body=rest(rest(rest(list)));
          auto pb=vcast<SPair>(body);
          return env->set(s->impl(),
              SMacro::make(s->impl(), pms, pb->head(), env));
        }
        //(define-macro (name p1 p2)
        //               (some body))
        auto body= rest(rest(list));
        auto pb=vcast<SPair>(body);
        auto pms= to_params(rest(a2));
        auto var= vcast<SSymbol>(car(a2));
        auto fn= var->impl();
        return env->set(fn,
            SMacro::make(fn, pms, pb->head(),env));
      }

      if (code == "macroexpand") {
        d::preEqual(2, len, "macroexpand");
        return macroExpand(this, env, vcast<SPair>(list->tail())->head());
      }

      if (code == "begin") {
        for (auto i = 1; i < (len-1); ++i) {
          EVAL(nth(list,i), env);
        }
        ast = EVAL(nth(list,len-1),env);
        continue;
      }

      if (code == "if") {
        if (auto c= EVAL(nth(list,1), env); truthy(c)) {
          ast = nth(list,2);
        } else if (len == 4) {
          ast = nth(list,3);
        } else {
          return DVAL_NIL;
        }
        continue;
      }

      if (code == "lambda") {
        d::preMin(3,len,"lambda");
        // (lambda (x y) ...)
        // (lambda (x .y) ...)
        // (lambda args ...)
        auto e2= nth(list,1);
        auto s2= vcast<SSymbol>(e2);
        auto pms= s2 ? StrVec{".", s2->impl()} : to_params(e2);
        auto var= "anon-fn"+N_STR(++seed);
        auto body=rest(rest(list));
        return SLambda::make(var, pms, wrapAsDo(body), env);
      }
    }

    DEBUG("about to each each element in %s", list->pr_str(1).c_str());

    auto vs = evalEach(this, env, list);
    DEBUG("ret=%s", vs->pr_str(1).c_str());
    d::DValue op= nth(vs,0);
    d::ValVec args;
    appendAll(vs,1,args);
    if (auto lambda= vcast<SLambda>(op); X_NIL(lambda)) {
      ast = lambda->body;
      env = lambda->bindContext(d::VSlice(args));
      continue;
    }
    if (auto native= vcast<SNative>(op); X_NIL(native)) {
      return native->invoke(this, d::VSlice(args));
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<int,d::DValue> Scheme::READ(cstdstr& s) {
  return SExprParser(s.c_str()).parse();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr repl(cstdstr& s, d::DFrame env) {
  Scheme lisp;
  stdstr out;
  auto ret= lisp.READ(s);
  auto cnt= _1(ret);
  switch (cnt) {
  case 0: out="nil"; break;
  case 1: out = lisp.PRINT(lisp.EVAL(_2(ret), env)); break;
  default:
    auto s= vcast<SVec>(_2(ret));
    for (auto i=0, e=s->count(); i<e; ++i) {
      out = lisp.PRINT(lisp.EVAL(s->nth(i), env));
    }
    break;
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame root_env() {
  auto f= init_natives();
  //Scheme ls;
  //for (auto& s : CORE_LISP) { repl(s, f); }
  return f;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr repl(cstdstr& s) {
  return repl(s, root_env());
}
















//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


