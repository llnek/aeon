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
        /*
        d::preMin(2, len, "let");
        auto args= vcast<LVec>(list->nth(1));
        auto cnt= d::preEven(args->count(), "let bindings");
        auto f= d::Frame::make("let", env);
        for (auto i = 0; i < cnt; i += 2) {
          f->set(vcast<LSymbol>(args->nth(i))->impl(), EVAL(args->nth(i+1), f));
        }
        ast = wrapAsDo(list,2);
        env = f;
        continue;
        */
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
        //d::preEqual(4, len, "define-macro");
        //auto var = vcast<LSymbol>(list->nth(1))->impl();
        //auto pms= cast_params(list->nth(2));
        //return env->set(var, MACRO_VAL(var, pms, list->nth(3), env));
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
        //d::preMin(2,len,"lambda");
        //auto pms= cast_params(list->nth(1));
        //auto var= "anon-fn#"+std::to_string(++seed);
        //return LAMBDA_VAL(var, pms, wrapAsDo(list,2), env);
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


