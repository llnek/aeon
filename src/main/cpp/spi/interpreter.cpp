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

#include "interpreter.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::spi {
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Interpreter::Interpreter(const char* src) {
  source = src;
  S_NIL(symbols);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Interpreter::interpret() {
  SimplePascalParser p(source);
  auto tree= (Ast*) p.parse();
  return check(tree), eval(tree);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Interpreter::eval(Ast* tree) {
  auto res= (stack.push("root"), tree->eval(this));
  auto env= stack.pop();
  if (res.type == d::EXPR_INT)
    ::printf("result = %ld\n\n", res.value.u.n);
  else
    ::printf("result = %lf\n\n", res.value.u.r);
  auto m= env->keys();
  ::printf("slots cont = %d\n", (int)m.size());
  for (auto& k : m) {
    auto x = env->get(k);
    if (x.type == d::EXPR_INT)
      ::printf("key = %s, value = %ld\n", k.c_str(), x.value.u.n);
    if (x.type == d::EXPR_REAL)
      ::printf("key = %s, value = %lf\n", k.c_str(), x.value.u.r);
    if (x.type == d::EXPR_STR)
      ::printf("key = %s, value = %s\n", k.c_str(), x.value.cs.get()->get());
  }
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::Frame* Interpreter::push(const std::string& name) {
  return stack.push(name);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::Frame* Interpreter::pop() {
  return stack.pop();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::Frame* Interpreter::peek() {
  return stack.peek();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::setValue(const std::string& name, const d::ExprValue& v) {
  auto x = stack.peek();
  if (x) x->set(name, v);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Interpreter::getValue(const std::string& name) {
  auto x = stack.peek();
  if (x)
    return x->get(name);
  else
    return d::ExprValue();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::check(Ast* tree) {
  symbols= new SymTable("root");
  tree->visit(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::Symbol* Interpreter::lookup(const std::string& n, bool traverse) {
  return symbols->lookup(n, traverse);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::define(d::Symbol* s) {
  if (s) symbols->insert(s);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::pushScope(const std::string& name) {
  auto s= new SymTable(name, (SymTable*)symbols);
  symbols=s;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::SymbolTable* Interpreter::popScope() {
  if (! symbols->enclosing) {
    return nullptr;
  } else {
    auto cur = symbols;
    symbols = cur->enclosing;
    S_NIL(cur->enclosing);
    return cur;
  }
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

