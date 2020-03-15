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
namespace czlab::kirby {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
LLambdaFn A_LAMBDA;
LMacro A_MACRO;
LNativeFn A_NATIVE;
LList A_LIST;
LVec A_VEC;
LHash A_MAP;
LNil A_NIL;
LTrue A_TRUE;
LFalse A_FALSE;
LSymbol A_SYMB { "" };

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame root_env() {
  auto r = new d::Frame("root");

  return d::DslFrame(r);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame lambda_env(LLambdaFn* fn, ValueVec args) {
  auto fm= new d::Frame("lambda", fn->env);
  auto z= fn->params.size();
  auto len= args.size();
  auto j=0;
  for (auto i=0; i < z; ++i) {
    auto k= fn->params[i];
    if (k == "&") {
      ASSERT1((i+1 == (z-1)));
      ValueVec x;
      k= fn->params[i+1];
      for (; j < len; ++j) {
        s__conj(x, args[j]);
      }
      fm->set(k, d::DslValue(new LList(x)), true);
      break;
    }
    ASSERT1(j < len);
    fm->set(k, args[j], true);
    ++j;
  }
  return d::DslFrame(fm);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeq* is_pair(d::DslValue v) {
  LSeq* s= nullptr;
  auto x= v.ptr();
  if (typeid(A_LIST) == typeid(*x) ||
      typeid(A_VEC) == typeid(*x) ||
      typeid(A_MAP) == typeid(*x)) {
    s = s__cast(LSeq,x);
    if (s->count() == 0) {
      S_NIL(s);
    }
  }
  return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool falsey(d::DslValue v) {
  auto p = v.ptr();
  return typeid(*p) == typeid(A_NIL) ||
         typeid(*p) == typeid(A_FALSE);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool truthy(d::DslValue v) { return ! falsey(v); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol* cast_symbol(d::DslValue v) {
  return typeid(A_SYMB) == typeid(v.ptr())
         ? s__cast(LSymbol,v.ptr()) : nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList* cast_list(d::DslValue v) {
  return typeid(A_LIST) == typeid(v.ptr())
         ? s__cast(LList,v.ptr()) : nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec* cast_vec(d::DslValue v) {
  return typeid(A_VEC) == typeid(v.ptr())
         ? s__cast(LVec,v.ptr()) : nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash* cast_map(d::DslValue v) {
  return typeid(A_MAP) == typeid(v.ptr())
         ? s__cast(LHash,v.ptr()) : nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LLambdaFn* cast_lambda(d::DslValue v) {
  return typeid(A_LAMBDA) == typeid(v.ptr())
         ? s__cast(LLambdaFn,v.ptr()) : nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro* cast_macro(d::DslValue v) {
  return typeid(A_MACRO) == typeid(v.ptr())
         ? s__cast(LMacro,v.ptr()) : nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNativeFn* cast_native(d::DslValue v) {
  return typeid(A_NATIVE) == typeid(v.ptr())
         ? s__cast(LNativeFn,v.ptr()) : nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue addition(ValueVec& args) {
  std::vector<d::NumberSlot> v;
  auto real=false;
  for (auto& x : args) {
    auto n= s__cast(LNumber,x.ptr())->number();
    if (n.type == d::T_REAL) { real=true; }
    s__conj(v,n);
  }
  double R= 0.0;
  llong N= 0;
  for (auto& x : v) {
    if (x.type == d::T_REAL) {
      R += x.u.r;
    } else {
      if (real) R += x.u.n; else N += x.u.n;
    }
  }
  if (real) {
    return d::DslValue(new LFloat(R));
  } else {
    return d::DslValue(new LInt(N));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue eval_ast(d::DslValue ast) {
  return d::DslValue();
}










//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Lisper::PRINT(const d::DslValue& v) {
  return s__cast(LValue,v.ptr())->toString(true);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue quasiQuote(d::DslValue ast, d::DslFrame env) {
  auto seq = is_pair(ast);
  if (!seq) {
    ValueVec x { new LSymbol("quote"), ast };
    return d::DslValue(new LList(x));
  }
  if (auto s = cast_symbol(seq->first()); s->impl() == "unquote") {
    // (qq (uq form)) -> form
    //checkArgsIs("unquote", 1, seq->count() - 1);
    return seq->get(1);
  }
  auto s2 = is_pair(seq->first());
  ValueVec vec;
  if (auto b = cast_symbol(s2->first());
      X_NIL(b) && b->impl() == "splice-unquote") {
    //checkArgsIs("splice-unquote", 1, innerSeq->count() - 1);
    // (qq (sq '(a b c))) -> a b c
    s__conj(vec, new LSymbol("concat"));
    s__conj(vec, s2->get(1));
    s__conj(vec, quasiQuote(seq->rest(), env));
  } else {
    // (qq (a b c)) -> (list (qq a) (qq b) (qq c))
    // (qq xs     ) -> (cons (qq (car xs)) (qq (cdr xs)))
    s__conj(vec, new LSymbol("cons"));
    s__conj(vec, quasiQuote(seq->first(), env));
    s__conj(vec, quasiQuote(seq->rest(), env));
  }
  return d::DslValue(new LList(vec));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro* maybeMacro(Lisper* p, d::DslValue ast, d::DslFrame env) {
  if (auto s = is_pair(ast); X_NIL(s)) {
    if (auto sym = cast_symbol(s->first())) {
      if (auto f = env->find(sym->impl()); f.isSome()) {
        return cast_macro(sym->eval(p, f));
      }
    }
  }
  return nullptr;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
strvec cast_params(d::DslValue v) {
  auto pms = cast_vec(v);
  strvec vs;
  for (auto i = 0; i < pms->count(); ++i) {
    auto s= cast_symbol(pms->get(i));
    ASSERT1(s != nullptr);
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
  auto m = maybeMacro(p, ast,env);
  while (X_NIL(m)) {
    auto seq= is_pair(ast);
    auto len=seq->count();
    ValueVec args;
    for (auto i = 1; i < len; ++i) {
      s__conj(args, seq->get(i));
    }
    ast = m->apply(args);
  }
  return ast;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Lisper::EVAL(d::DslValue ast, d::DslFrame env) {
  while (true) {
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

    if (auto op = cast_symbol(list->get(0))) {
      auto len = list->count() - 1;
      auto code = op->impl();

      if (code == "def") {
        ASSERT(len == 2, "def expects 2, got %d arguments.", len);
        auto var= cast_symbol(list->get(1));
        ASSERT1(var != nullptr);
        return env->set(var->impl(), EVAL(list->get(2), env), true);
      }
      else
      if (code == "defmacro") {
        auto var = cast_symbol(list->get(1));
        ASSERT1(var != nullptr);
        auto pms= cast_params(list->get(2));
        return env->set(var->impl(),
            d::DslValue(new LMacro(pms, list->get(3), env)), true);
      }
      else
      if (code == "defn") {
        auto var = cast_symbol(list->get(1));
        ASSERT1(var != nullptr);
        auto pms= cast_params(list->get(2));
        return env->set(var->impl(),
            d::DslValue(new LLambdaFn(var->impl(), pms, list->get(3), env)), true);
      }
      else
      if (code == "do") {
        for (auto i = 1; i < len; ++i) {
          EVAL(list->get(i), env);
        }
        ast = list->get(len);
        continue;
      }
      else
      if (code == "fn") {
        ASSERT(len == 2, "fn expects 2, got %d arguments.", len);
        auto pms= cast_params(list->get(1));
        return d::DslValue(new LLambdaFn(pms, list->get(2), env));
      }
      else
      if (code == "if") {
        if (auto c= EVAL(list->get(1), env); truthy(c)) {
          ast = list->get(2);
        } else if (len > 2) {
          ast = list->get(3);
        } else {
          return nil_value();
        }
        continue;
      }
      else
      if (code == "let") {
        ASSERT(len == 2, "let expects 2 args, got %d.\n", len);
        auto args=cast_vec(list->get(1));
        auto cnt=args->count();
        ASSERT(a::is_even(cnt), "let bindings must be even, got %d.\n", cnt);
        d::DslFrame f= d::DslFrame(new d::Frame("let", env));
        for (auto i = 0; i < cnt; i += 2) {
          auto s=cast_symbol(args->get(i));
          ASSERT1(s != nullptr);
          f->set(s->impl(), EVAL(args->get(i+1), f), true);
        }
        ast = list->get(2); // body
        continue;
      }
      else
      if (code == "macroexpand") {
        return macroExpand(this, list->get(1), env);
      }
      else
      if (code == "quasiquote") {
        ast = quasiQuote(list->get(1), env);
        continue;
      }
      else
      if (code == "quote") {
        return list->get(1);
      }
      else
      if (code == "try*") {
        auto body = list->get(1);
        if (len == 1) {
          ast = EVAL(body, env);
          continue;
        }
        auto catche = cast_list(list->get(2));
        auto c= cast_symbol(catche->get(0));
        ASSERT(X_NIL(c) && c->impl()=="catch*",
            "Expected keyword catch*, got %s.\n", c->impl().c_str());
        auto e= cast_symbol(catche->get(1));
        ASSERT(X_NIL(e),
            "Expected exception variable, got %s.\n", e->impl().c_str());
        d::DslValue error;
        try {
          ast = EVAL(body, env);
        }
        catch(stdstr& exp) {
          error = d::DslValue(new LString(exp));
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
          env->set(e->impl(), error, true);
          ast = catche->get(2);
        }
        continue;
      }
    }

    auto vs = list->evalEach(this, env);
    auto op = vs[0];
    ValueVec args;
    args.insert(args.end(), vs.begin()+1, vs.end());
    if (auto lambda= cast_lambda(op); X_NIL(lambda)) {
      ast = lambda->body;
      env = lambda_env(lambda, args);
      continue;
    }
    else if (auto native= cast_native(op); X_NIL(native)) {
      return native->apply(args);
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


