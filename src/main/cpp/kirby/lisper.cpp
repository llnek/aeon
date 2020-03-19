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
namespace czlab::kirby {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static bool truthy(d::DslValue v) {
  return s__cast(LValue,v.ptr())->truthy();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame lambda_env(LLambda* fn, VSlice args) {
  auto fm= new d::Frame("lambda", fn->env);
  auto z=fn->params.size();
  auto len= args.size();
  auto i=0, j=0;
  for (; i < z; ++i) {
    auto k= fn->params[i];
    if (k == "&") {
      ASSERT1((i+1 == (z-1)));
      VVec x;
      for (auto a= args.begin+j; a != args.end; ++a) {
        s__conj(x, *a);
      }
      fm->set(fn->params[i+1], list_value(VSlice(x)),true);
      j=len;
      i= z;
      break;
    }
    ASSERT1(j < len);
    fm->set(k, *(args.begin + j), true);
    ++j;
  }
  ASSERT1(j==len && i== z);
  return d::DslFrame(fm);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeq* is_pair(d::DslValue v, int panic) {
  LSeq* s = P_NIL;
  if (auto x = cast_list(v); X_NIL(x)) {
    s = s__cast(LSeq,x);
    if (s->count()==0) { S_NIL(s); }
  } else if (panic) {
    expected("list", v);
  }
  return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Lisper::PRINT(const d::DslValue& v) {
  return s__cast(LValue,v.ptr())->toString(true);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue quasiQuote(d::DslValue ast, d::DslFrame env) {
  auto seq = is_pair(ast,0);
  VVec out;
  if (E_NIL(seq)) {
    s__conj(out, new LSymbol("quote"));
    s__conj(out, ast);
    return list_value(VSlice(out));
  }
  auto f1= seq->first();
  if (auto s = cast_symbol(f1); s->impl() == "unquote") {
    // (qq (uq form)) -> form
    ASSERT1(2==seq->count());
    return seq->nth(1);
  }
  auto s2 = is_pair(f1, 1);
  if (auto b = cast_symbol(s2->first());
      X_NIL(b) && b->impl() == "splice-unquote") {
    ASSERT1(2==s2->count());
    // (qq (sq '(a b c))) -> a b c
    s__conj(out, symbol_value("concat"));
    s__conj(out, s2->nth(1));
    s__conj(out, quasiQuote(seq->rest(), env));
  } else {
    // (qq (a b c)) -> (list (qq a) (qq b) (qq c))
    // (qq xs     ) -> (cons (qq (car xs)) (qq (cdr xs)))
    s__conj(out, symbol_value("cons"));
    s__conj(out, quasiQuote(seq->first(), env));
    s__conj(out, quasiQuote(seq->rest(), env));
  }
  return list_value(VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro* maybeMacro(Lisper* p, d::DslValue ast, d::DslFrame env) {
  if (auto s = is_pair(ast,0); X_NIL(s)) {
    if (auto sym = cast_symbol(s->first())) {
      if (auto f = env->find(sym->impl()); f.isSome()) {
        return cast_macro(sym->eval(p, f));
      }
    }
  }
  return P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
StrVec cast_params(d::DslValue v) {
  auto pms = cast_vec(v, 1);
  StrVec vs;
  for (auto i = 0, e=pms->count(); i < e; ++i) {
    auto s= cast_symbol(pms->nth(i),1);
    auto p= s->impl();
    ASSERT1(p.length() > 0);
    if (p.length() > 1 && p[0]=='&') {
      s__conj(vs, stdstr("&"));
      p= stdstr(p.c_str() + 1);
    }
    s__conj(vs, p);
  }
  return vs;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue macroExpand(Lisper* p, d::DslValue ast, d::DslFrame env) {
  for (auto m = maybeMacro(p, ast, env); X_NIL(m);) {
    auto seq= is_pair(ast,0);
    VVec args;
    for (auto i = 1, e=seq->count(); i < e; ++i) {
      s__conj(args, seq->nth(i));
    }
    ast = args.size() == 0
      ? m->invoke()
      : m->invoke(VSlice(args.begin(), args.end()));
    m = maybeMacro(p, ast, env);
  }
  return ast;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Lisper::EVAL(d::DslValue ast, d::DslFrame env) {
  while (1) {
    auto list = cast_list(ast);

    if (E_NIL(list) ||
        (list->count() == 0)) {
      return s__cast(LValue,ast.ptr())->eval(this, env);
    }

    ast = macroExpand(this, ast, env);
    list = cast_list(ast);

    if (E_NIL(list) ||
        (list->count() == 0)) {
      return s__cast(LValue,ast.ptr())->eval(this, env);
    }

    if (auto op = cast_symbol(list->nth(0))) {
      auto len = list->count() - 1;
      auto code = op->impl();

      if (code == "def") {
        preEqual(2, len, "def");
        auto var= cast_symbol(list->nth(1),1);
        return env->set(var->impl(), EVAL(list->nth(2), env), true);
      }

      if (code == "defmacro") {
        auto var = cast_symbol(list->nth(1),1)->impl();
        auto pms= cast_params(list->nth(2));
        return env->set(var,
            macro_value(var, pms, list->nth(3), env), true);
      }

      if (code == "defn") {
        auto var = cast_symbol(list->nth(1),1)->impl();
        auto pms= cast_params(list->nth(2));
        return env->set(var,
            lambda_value(var, pms, list->nth(3), env), true);
      }

      if (code == "do") {
        for (auto i = 1; i < len; ++i) {
          EVAL(list->nth(i), env);
        }
        ast = list->nth(len);
        continue;
      }

      if (code == "fn") {
        preEqual(2, len, "fn");
        auto pms= cast_params(list->nth(1));
        return lambda_value("", pms, list->nth(2), env);
      }

      if (code == "if") {
        if (auto c= EVAL(list->nth(1), env); truthy(c)) {
          ast = list->nth(2);
        } else if (len > 2) {
          ast = list->nth(3);
        } else {
          return nil_value();
        }
        continue;
      }

      if (code == "let") {
        preEqual(2, len, "let");
        auto args= cast_vec(list->nth(1),1);
        auto cnt=args->count();
        preEven(cnt, "let bindings");
        auto f= d::DslFrame(new d::Frame("let", env));
        for (auto i = 0; i < cnt; i += 2) {
          auto s=cast_symbol(args->nth(i),1)->impl();
          f->set(s, EVAL(args->nth(i+1), f), true);
        }
        ast = list->nth(2);
        continue;
      }

      if (code == "macroexpand") {
        return macroExpand(this, list->nth(1), env);
      }

      if (code == "quasiquote") {
        ast = quasiQuote(list->nth(1), env);
        continue;
      }

      if (code == "quote") {
        return list->nth(1);
      }

      if (code == "try*") {
        auto body = list->nth(1);
        if (len == 1) {
          ast = EVAL(body, env);
          continue;
        }
        auto catche = cast_list(list->nth(2));
        auto c= cast_symbol(catche->nth(0),1)->impl();
        ASSERT(c=="catch*",
            "Expected keyword catch*, got %s.\n", c.c_str());
        auto e= cast_symbol(catche->nth(1),1)->impl();
        d::DslValue error;
        try {
          ast = EVAL(body, env);
        }
        catch(stdstr& exp) {
          error = string_value(exp);
        }
        /*
        catch (EmptyInput& exp) {
          ast = nil_value();
        }
        */
        catch(d::DslValue& exp) {
          error = exp;
        };
        if (error.isSome()) {
          env = d::DslFrame(new d::Frame("catch", env));
          env->set(e, error, true);
          ast = catche->nth(2);
        }
        continue;
      }
    }

    auto vs = list->evalEach(this, env);
    auto op = vs[0];
    VVec args;
    args.insert(args.end(), vs.begin()+1, vs.end());
    if (auto lambda= cast_lambda(op); X_NIL(lambda)) {
      ast = lambda->body;
      env = lambda_env(lambda, args);
      continue;
    }
    if (auto native= cast_native(op); X_NIL(native)) {
      return native->invoke(VSlice(args));
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Lisper::READ(const stdstr& s) {
  return SExprParser(s.c_str()).parse();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr repl(const stdstr& s, d::DslFrame p) {
  Lisper lisp;
  return lisp.PRINT(lisp.EVAL(lisp.READ(s), p));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr repl(const stdstr& s) {
  return repl(s, root_env());
}
















//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


