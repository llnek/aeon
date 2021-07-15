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
#include "builtins.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::otto{
namespace a= czlab::aeon;
namespace d= czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
StrVec CORE_LISP{
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
#define TO_VAL(x) DCAST(LValue,x)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Lisper::evalAst(d::DValue ast, d::DFrame env){
  return TO_VAL(ast)->eval(this,env);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//set up a new frame for this function call
d::DFrame lambda_env(LLambda* fn, d::VSlice args){
  auto fm= d::Frame::make("lambda", fn->env);
  auto z=fn->params.size();
  auto len= args.size();
  auto i=0, j=0;
  //run through parameters...
  for(; i<z; ++i){
    auto k= fn->params[i];
    if(k == "&"){
      //var-args, next must be the last one
      //e.g. [a b c & x]
      ASSERT1((i+1 == (z-1)));
      d::ValVec x;
      appendAll(args,j,x);
      fm->set(fn->params[i+1], LIST_VAL(x));
      j=len;
      i= z;
      break;
    }
    ASSERT1(j < len);
    fm->set(k, *(args.begin + j));
    ++j;
  }
  //make sure arg count matches param count
  ASSERT1(j==len && i== z);
  return fm;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeqable* is_pair(d::DValue v, int panic){
  LSeqable* s=NULL;
  if(auto x= vcast<LList>(v); X_NIL(x)){
    s= s__cast(LSeqable,x);
    if(s->count()==0){ S_NIL(s); }
  }else if(panic){
    expected("list", v);
  }
  return s;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Lisper::PRINT(d::DValue v){
  return TO_VAL(v)->pr_str(1);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Lisper::syntaxQuote(d::DValue ast, d::DFrame env){
  auto seq= is_pair(ast,0);
  if(E_NIL(seq))
    //`a => 'a
    return LIST_VAL2(SYMBOL_VAL("quote"), ast);

  //work on the first element AST[0]
  //`(ast0 a b)
  //deal with unquote => ~x or ~(a b c)
  auto ast0= seq->first();
  if(auto s = vcast<LSymbol>(ast0);
     X_NIL(s) && s->impl() == "unquote"){
    //`~x or `~(a b c)
    ASSERT1(2==seq->count());
    return seq->nth(1);
  }
  auto s2 = is_pair(ast0,0);
  d::ValVec out;
  LSymbol* b = NULL;
  if(s2)
    b = vcast<LSymbol>(s2->first());

  if(b && b->impl() == "splice-unquote"){
    //~@x  or ~@(a b c)
    ASSERT1(2==s2->count());
    s__conj(out, SYMBOL_VAL("concat"));
    s__conj(out, s2->nth(1));
    s__conj(out, syntaxQuote(seq->rest(), env));
  }else{
    //else just simple case `(a b c (d e f))
    //look again, once at a time
    s__conj(out, SYMBOL_VAL("cons"));
    s__conj(out, syntaxQuote(ast0, env));
    s__conj(out, syntaxQuote(seq->rest(), env));
  }
  return LIST_VAL(out);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LMacro* maybeMacro(Lisper* p, d::DValue ast, d::DFrame env){
  auto s= is_pair(ast,0);
  auto sym = s ? vcast<LSymbol>(s->first()) : P_NIL;
  auto f= sym ? env->search(sym->impl(),env) : P_NIL;
  return f ? vcast<LMacro>(sym->eval(p, f)) : P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
StrVec cast_params(d::DValue v){
  auto pms = vcast<LVec>(v);
  StrVec vs;
  for(auto i=0, e=pms->count(); i<e; ++i){
    auto s= vcast<LSymbol>(pms->nth(i));
    auto p= s->impl();
    ASSERT1(p.length()>0);
    //handle case where param name is &xs,
    //if so, we make it as & xs.
    if(p.length() > 1 && p[0]=='&'){
      s__conj(vs, stdstr("&"));
      p= stdstr(p.c_str() + 1);
    }
    s__conj(vs, p);
  }
  return vs;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Lisper::macroExpand(d::DValue ast, d::DFrame env){
  for(auto m= maybeMacro(this, ast, env); X_NIL(m);){
    auto seq= is_pair(ast,1);
    d::ValVec args;
    appendAll(seq,1,args);
    ast= args.empty() ? m->invoke(this)
                      : m->invoke(this, d::VSlice(args));
    m = maybeMacro(this, ast, env);
  }
  return ast;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue wrapAsDo(LList* form, int from){
  auto end= form->count();
  auto diff= end-from;
  switch(diff){
    case 0: return NIL_VAL();
    case 1: return form->nth(from);
    default:
      d::ValVec out { SYMBOL_VAL("do") };
      for(; from<end; ++from)
        s__conj(out, form->nth(from));
      return LIST_VAL(out);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue wrapAsDo(d::ValVec& v){
  switch(v.size()){
    case 0: return NIL_VAL();
    case 1: return v[0];
    default:
      d::ValVec out { SYMBOL_VAL("do") };
      s__ccat(out,v);
      return LIST_VAL(out);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Lisper::EVAL(d::DValue ast, d::DFrame env){
  while(1){
    //DEBUG("eval(ast)=%s.\n", C_STR(ast->pr_str(1)));
    auto list = vcast<LList>(ast);
    if(E_NIL(list))
      return evalAst(ast, env);
    else{
      ast = macroExpand(ast, env);
      list = vcast<LList>(ast);
    }

    if(E_NIL(list))
      return evalAst(ast,env);

    if(list->isEmpty())
      return ast;

    //DEBUG("eval(2)=%s.\n", C_STR(list->pr_str(1)));

    if(auto op = vcast<LSymbol>(list->nth(0))){
      //DEBUG("op = %s.\n", C_STR(op->impl()));
      auto len = list->count();
      auto code = op->impl();

      if(code == "def"){
        d::preEqual(3, len, "def");
        auto var= vcast<LSymbol>(list->nth(1))->impl();
        return env->set(var, EVAL(list->nth(2), env));
      }

      if(code == "let"){
        d::preMin(2, len, "let");
        auto args= vcast<LVec>(list->nth(1));
        auto cnt= d::preEven(args->count(), "let bindings");
        auto f= d::Frame::make("let", env);
        for(auto i= 0; i<cnt; i += 2)
          f->set(vcast<LSymbol>(args->nth(i))->impl(), EVAL(args->nth(i+1), f));
        ast = wrapAsDo(list,2);
        env = f;
        continue;
      }

      if(code == "quote"){
        d::preEqual(2, len, "quote");
        return list->nth(1);
      }

      if(code == "syntax-quote"){
        d::preEqual(2, len, "syntax-quote");
        ast= syntaxQuote(list->nth(1), env);
        continue;
      }

      if(code == "defmacro"){
        d::preEqual(4, len, "defmacro");
        auto var = vcast<LSymbol>(list->nth(1))->impl();
        auto pms= cast_params(list->nth(2));
        return env->set(var, MACRO_VAL(var, pms, list->nth(3), env));
      }

      if(code == "macroexpand"){
        d::preEqual(2, len, "macroexpand");
        return macroExpand(list->nth(1), env);
      }

      if(code == "try"){
        //(try a b c (catch e 1 2 3))
        d::DValue error;
        stdstr errorVar;
        d::DValue cbody;
        d::ValVec tbody;
        for(auto j=1; j<len; ++j){
          auto n= list->nth(j);
          if(auto c= vcast<LList>(n); X_NIL(c) && c->count() > 0){
            if(auto s= vcast<LSymbol>(c->nth(0)); X_NIL(s) && s->impl() == "catch"){
              ASSERT(j==(len-1),
                     "catch must be last form: %s.\n", C_STR(list->pr_str(1)));
              d::preMin(2,c->count(), "catch");
              errorVar = vcast<LSymbol>(c->nth(1))->impl();
              cbody=wrapAsDo(c,2);
              break;
            }
          }
          s__conj(tbody, n);
        }
        if(len==1 || tbody.empty()) { return NIL_VAL(); }
        try{
          ast = EVAL(wrapAsDo(tbody),env);
        }
        catch(stdstr& exp){
          error = STRING_VAL(exp);
        }
        catch(d::DValue& exp){
          error = exp;
        }
        if(cbody && error){
          env= d::Frame::make("catch", env);
          env->set(errorVar, error);
          ast = EVAL(cbody,env);
        }
        continue;
      }

      if(code == "do"){
        for(auto i=1; i<(len-1); ++i){
          EVAL(list->nth(i), env);
        }
        ast = EVAL(list->nth(len-1),env);
        continue;
      }

      if(code == "if"){
        if(auto c= EVAL(list->nth(1), env); truthy(c)){
          ast= list->nth(2);
        }else if(len == 4){
          ast= list->nth(3);
        }else{
          return NIL_VAL();
        }
        continue;
      }

      if(code == "defn"){
        d::preMin(3,len,"defn");
        auto var = vcast<LSymbol>(list->nth(1))->impl();
        auto pms= cast_params(list->nth(2));
        return env->set(var, LAMBDA_VAL(var, pms, wrapAsDo(list,3), env));
      }

      if(code == "fn"){
        d::preMin(2,len,"fn");
        auto pms= cast_params(list->nth(1));
        auto var= "anon-fn#"+std::to_string(++seed);
        return LAMBDA_VAL(var, pms, wrapAsDo(list,2), env);
      }
    }

    auto ret = list->eval(this, env);
    //DEBUG("ret=%s.\n", C_STR(ret->pr_str(1)));
    auto vs = cast_seqable(ret);
    if(auto len=vs->count(); len==0){
      return NIL_VAL();
    }
    d::DValue op= vs->nth(0);
    d::ValVec args;
    appendAll(vs,1,args);
    if(auto lambda= vcast<LLambda>(op); X_NIL(lambda)){
      ast = lambda->body;
      env = lambda->bindContext(d::VSlice(args));
      continue;
    }
    if(auto native= vcast<LNative>(op); X_NIL(native)){
      return native->invoke(this, d::VSlice(args));
    }
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<int,d::DValue> Lisper::READ(const stdstr& s){
  return SExprParser(s.c_str()).parse();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr repl(const stdstr& s, d::DFrame env){
  Lisper lisp;
  stdstr out;
  auto ret= lisp.READ(s);
  auto cnt= ret.first;
  switch(cnt){
    case 0: out="nil"; break;
    case 1: out = lisp.PRINT(lisp.EVAL(ret.second, env)); break;
    default:
      auto s= vcast<LList>(ret.second);
      for(auto i=0, e=s->count(); i<e; ++i)
        out= lisp.PRINT(lisp.EVAL(s->nth(i), env));
      break;
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame root_env(){
  auto f= init_natives();
  Lisper ls;
  for(auto& s : CORE_LISP){
    repl(s, f);
  }
  return f;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr repl(const stdstr& s){
  return repl(s, root_env());
}





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


