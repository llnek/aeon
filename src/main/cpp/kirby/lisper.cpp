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
StrVec CORE_LISP {
  R"((defmacro cond
       [& xs]
       (if (> (count xs) 0)
         (list 'if
               (first xs)
               (if (> (count xs) 1)
                 (nth xs 1)
                 (throw "odd number of forms to cond"))
               (cons 'cond (rest (rest xs)))))))",
  R"((def not
       (fn [cond] (if cond false true))))",
  "(defn load-file \
       [filename] \
         (eval (read-string (str \"(do \" (slurp filename) \")\"))))",
  R"((def *host-language* "C++"))"
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Lisper::evalAst(d::DslValue ast, d::DslFrame env) {
  return s__cast(LValue,ast.ptr())->eval(this,env);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame lambda_env(LLambda* fn, VSlice args) {
  auto fm= new d::Frame("lambda", fn->env);
  auto z=fn->params.size();
  auto len= args.size();
  auto i=0, j=0;
  // run through parameters...
  for (; i < z; ++i) {
    auto k= fn->params[i];
    if (k == "&") {
      // var-args, next must be the last one
      // e.g. [a b c & x]
      ASSERT1((i+1 == (z-1)));
      VVec x;
      appendAll(args,j,x);
      fm->set(fn->params[i+1], LIST_VAL(x),true);
      j=len;
      i= z;
      break;
    }
    ASSERT1(j < len);
    fm->set(k, *(args.begin + j), true);
    ++j;
  }
  // make sure arg count matches param count
  ASSERT1(j==len && i== z);
  return d::DslFrame(fm);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeqable* is_pair(d::DslValue v, int panic) {
  LSeqable* s = P_NIL;
  if (auto x = cast_list(v); X_NIL(x)) {
    s = s__cast(LSeqable,x);
    if (s->count()==0) { S_NIL(s); }
  } else if (panic) {
    expected("list", v);
  }
  return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Lisper::PRINT(d::DslValue v) {
  return s__cast(LValue,v.ptr())->toString(true);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Lisper::syntaxQuote(d::DslValue ast, d::DslFrame env) {
  auto seq = is_pair(ast,0);
  if (E_NIL(seq)) {
    return LIST_VAL2(SYMBOL_VAL("quote"), ast);
  }
  auto f1= seq->first();
  if (auto s = cast_symbol(f1);
           X_NIL(s) && s->impl() == "unquote") {
    // `~x = x
    ASSERT1(2==seq->count());
    return seq->nth(1);
  }
  auto s2 = is_pair(f1, 1);
  VVec out;
  if (auto b = cast_symbol(s2->first());
      X_NIL(b) && b->impl() == "splice-unquote") {
    ASSERT1(2==s2->count());
    // (qq (sq '(a b c))) -> a b c
    s__conj(out, SYMBOL_VAL("concat"));
    s__conj(out, s2->nth(1));
    s__conj(out, syntaxQuote(seq->rest(), env));
  } else {
    // (qq (a b c)) -> (list (qq a) (qq b) (qq c))
    // (qq xs     ) -> (cons (qq (car xs)) (qq (cdr xs)))
    s__conj(out, SYMBOL_VAL("cons"));
    s__conj(out, syntaxQuote(seq->first(), env));
    s__conj(out, syntaxQuote(seq->rest(), env));
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro* maybeMacro(Lisper* p, d::DslValue ast, d::DslFrame env) {
  if (auto s = is_pair(ast,0); X_NIL(s)) {
    if (auto sym = cast_symbol(s->first()); X_NIL(sym)) {
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
d::DslValue Lisper::macroExpand(d::DslValue ast, d::DslFrame env) {
  for (auto m = maybeMacro(this, ast, env); X_NIL(m);) {
    auto seq= is_pair(ast,1);
    VVec args;
    appendAll(seq,1,args);
    ast = args.size() == 0
      ? m->invoke(this)
      : m->invoke(this, VSlice(args));
    m = maybeMacro(this, ast, env);
  }
  return ast;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue wrapAsDo(LList* form, int from) {
  auto end= form->count();
  auto diff=end-from;
  switch (diff) {
    case 0: return NIL_VAL();
    case 1: return form->nth(from);
    default:
      VVec out { SYMBOL_VAL("do") };
      for (; from < end; ++from) {
        s__conj(out, form->nth(from));
      }
      return LIST_VAL(out);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue wrapAsDo(VVec& v) {
  switch (v.size()) {
    case 0: return NIL_VAL();
    case 1: return v[0];
    default:
      VVec out { SYMBOL_VAL("do") };
      s__ccat(out,v);
      return LIST_VAL(out);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Lisper::EVAL(d::DslValue ast, d::DslFrame env) {
  while (true) {
    DEBUG("eval(ast)=%s.\n", C_STR(ast->toString(true)));
    auto list = cast_list(ast, 0);

    if (E_NIL(list)) {
      return evalAst(ast,env);
    } else {
      ast = macroExpand(ast, env);
      list = cast_list(ast, 0);
    }

    if (E_NIL(list)) {
      return evalAst(ast,env);
    }

    if (list->isEmpty()) {
      return ast;
    }

    DEBUG("eval(2)=%s.\n", C_STR(list->toString(true)));

    if (auto op = cast_symbol(list->nth(0), 0)) {
      DEBUG("op = %s.\n", C_STR(op->impl()));
      auto len = list->count();
      auto code = op->impl();

      if (code == "def") {
        preEqual(3, len, "def");
        auto var= cast_symbol(list->nth(1),1)->impl();
        return env->set(var, EVAL(list->nth(2), env), true);
      }

      if (code == "let") {
        preMin(2, len, "let");
        auto args= cast_vec(list->nth(1),1);
        auto cnt= preEven(args->count(), "let bindings");
        auto f= d::DslFrame(new d::Frame("let", env));
        for (auto i = 0; i < cnt; i += 2) {
          f->set(cast_symbol(args->nth(i),1)->impl(), EVAL(args->nth(i+1), f), true);
        }
        ast = wrapAsDo(list,2);
        env = f;
        continue;
      }

      if (code == "quote") {
        preEqual(2, len, "quote");
        return list->nth(1);
      }

      if (code == "syntax-quote") {
        preEqual(2, len, "syntax-quote");
        ast = syntaxQuote(list->nth(1), env);
        continue;
      }

      if (code == "defmacro") {
        preEqual(4, len, "defmacro");
        auto var = cast_symbol(list->nth(1),1)->impl();
        auto pms= cast_params(list->nth(2));
        return env->set(var,
            MACRO_VAL(var, pms, list->nth(3), env), true);
      }

      if (code == "macroexpand") {
        preEqual(2, len, "macroexpand");
        return macroExpand(list->nth(1), env);
      }

      if (code == "try") {
        // (try a b c (catch e 1 2 3))
        d::DslValue error;
        stdstr errorVar;
        d::DslValue cbody;
        VVec tbody;
        for (auto j=1; j < len; ++j) {
          auto n= list->nth(j);
          if (auto c= cast_list(n); X_NIL(c) && c->count() > 0) {
            if (auto s= cast_symbol(c->nth(0)); X_NIL(s) && s->impl() == "catch") {
              ASSERT(j==(len-1),
                  "catch must be last form: %s.\n", C_STR(list->toString(true)));
              preMin(2,c->count(), "catch");
              errorVar = cast_symbol(c->nth(1),1)->impl();
              cbody=wrapAsDo(c,2);
              break;
            }
          }
          s__conj(tbody, n);
        }
        if (len==1 || tbody.empty()) { return NIL_VAL(); }
        try {
          ast = EVAL(wrapAsDo(tbody),env);
        }
        catch(stdstr& exp) {
          error = STRING_VAL(exp);
        }
        catch(d::DslValue& exp) {
          error = exp;
        }
        if (cbody.isSome() && error.isSome()) {
          env = d::DslFrame(new d::Frame("catch", env));
          env->set(errorVar, error, true);
          ast = EVAL(cbody,env);
        }
        continue;
      }

      if (code == "do") {
        for (auto i = 1; i < (len-1); ++i) {
          EVAL(list->nth(i), env);
        }
        ast = EVAL(list->nth(len-1),env);
        continue;
      }

      if (code == "if") {
        if (auto c= EVAL(list->nth(1), env); truthy(c)) {
          ast = list->nth(2);
        } else if (len == 4) {
          ast = list->nth(3);
        } else {
          return NIL_VAL();
        }
        continue;
      }

      if (code == "defn") {
        preMin(3,len,"defn");
        auto var = cast_symbol(list->nth(1),1)->impl();
        auto pms= cast_params(list->nth(2));
        return env->set(var,
            LAMBDA_VAL(var, pms, wrapAsDo(list,3), env), true);
      }

      if (code == "fn") {
        preMin(2,len,"fn");
        auto pms= cast_params(list->nth(1));
        auto var= "anon-fn#"+std::to_string(++seed);
        return LAMBDA_VAL(var, pms, wrapAsDo(list,2), env);
      }
    }

    auto ret = list->eval(this, env);
    DEBUG("ret=%s.\n", C_STR(ret->toString(true)));
    auto vs = cast_seqable(ret,1);
    if (auto len=vs->count(); len==0) {
      return NIL_VAL();
    }
    d::DslValue op= vs->nth(0);
    VVec args;
    appendAll(vs,1,args);
//    auto func= cast_function(op,1);
//    DEBUG("casted function = %s.\n", C_STR(func->name()));
    if (auto lambda= cast_lambda(op); X_NIL(lambda)) {
      ast = lambda->body;
      env = lambda->bindContext(VSlice(args));
      continue;
    }
    if (auto native= cast_native(op); X_NIL(native)) {
      return native->invoke(this, VSlice(args));
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<int,d::DslValue> Lisper::READ(const stdstr& s) {
  return SExprParser(s.c_str()).parse();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr repl(const stdstr& s, d::DslFrame env) {
  Lisper lisp;
  stdstr out;
  auto ret= lisp.READ(s);
  auto cnt= ret.first;
  switch (cnt) {
    case 0: out="nil"; break;
    case 1: out = lisp.PRINT(lisp.EVAL(ret.second, env)); break;
    default:
      auto s= cast_list(ret.second,1);
      for (auto i = 0, e=s->count(); i < e; ++i) {
        out = lisp.PRINT(lisp.EVAL(s->nth(i), env));
      }
      break;
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame root_env() {
  auto f= init_natives(new d::Frame("root"));
  Lisper ls;
  for (auto& s : CORE_LISP) {
    repl(s, f);
  }
  return d::DslFrame(f);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr repl(const stdstr& s) {
  return repl(s, root_env());
}
















//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


