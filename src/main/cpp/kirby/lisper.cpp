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

#include "lisper.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::kirby {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
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
d::DslValue Lisper::setValue(const stdstr& name, d::DslValue v, bool localOnly) {
  auto x = peekFrame();
  if (x.isSome()) x.ptr()->set(name, v, localOnly);
  return v;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Lisper::getValue(const stdstr& name) {
  auto x = peekFrame();
  if (x.isSome())
    return x.ptr()->get(name);
  else
    return nil_value();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lisper::containsSymbol(const stdstr& name) {
  auto x = peekFrame();
  return x.isSome() ? x.ptr()->find(name, false) : false;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame Lisper::pushFrame(const stdstr& name) {
  env = d::DslFrame(new d::Frame(name, env));
  return env;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame Lisper::popFrame() {
  d::DslFrame f;
  if (env.isSome()) {
    f= env;
    env= env.ptr()->getOuter();
  }
  //::printf("Frame pop'ed:=\n%s\n", f->toString().c_str());
  return f;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame Lisper::peekFrame() {
  return env;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lisper::Lisper() {
  auto f= new d::Frame("root");
  f->set("+", d::DslValue(new LNativeFn("+", &addition)), true);
  env= d::DslFrame(f);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue eval_ast(d::DslValue ast) {
  return d::DslValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue Lisper::eval(d::DslValue ast) {
  return s__cast(LValue,ast.ptr())->eval(this);
}











//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


