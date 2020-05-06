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
#define TO_VAL(x) DCAST(SValue,x)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Scheme::evalAst(d::DValue ast, d::DFrame env) {
  return TO_VAL(ast)->eval(this,env);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Scheme::PRINT(d::DValue v) {
  return TO_VAL(v)->pr_str(1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Scheme::EVAL(d::DValue ast, d::DFrame env) {
  return DVAL_NIL;
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
  case 1: out = lisp.PRINT(_2(ret)); break;
      //lisp.EVAL(_2(ret), env)); break;
  default:
    auto s= vcast<SVec>(_2(ret));
    for (auto i=0, e=s->count(); i<e; ++i) {
      out = lisp.PRINT(//lisp.EVAL(s->nth(i), env));
          s->nth(i));
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


