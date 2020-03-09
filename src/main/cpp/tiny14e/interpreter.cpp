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
#include "interpreter.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {
using namespace czlab::dsl;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Interpreter::Interpreter(const char* src) {
  source = src;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Interpreter::interpret() {
  CrenshawParser p(source);
  auto tree= p.parse();
  return check(tree), eval(tree);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Interpreter::eval(DslAst tree) {
  auto res= (push("root"), tree.ptr()->eval(this));
  //auto env= pop();
  //::printf("%s\n", env->toString().c_str());
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Interpreter::push(const stdstr& name) {
  stack = DslFrame(new d::Frame(name, stack));
  return stack;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Interpreter::pop() {
  d::DslFrame f;
  if (stack.isSome()) {
    f= stack;
    stack= stack.ptr()->getOuter();
  }
  //::printf("Frame pop'ed:=\n%s\n", f->toString().c_str());
  return f;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Interpreter::peek() {
  return stack;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Interpreter::setValue(const stdstr& name, d::DslValue v, bool localOnly) {
  auto x = peek();
  if (x.isSome()) x.ptr()->set(name, v, localOnly);
  return v;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Interpreter::getValue(const stdstr& name) {
  auto x = peek();
  if (x.isSome())
    return x.ptr()->get(name);
  else
    return d::DslValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<stdstr,DslSymbol> BITS {
  {"INTEGER", DslSymbol(new Symbol("INTEGER"))},
  {"REAL", DslSymbol(new Symbol("REAL"))},
  {"STRING", DslSymbol(new Symbol("STRING"))}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::check(DslAst tree) {
  symbols= DslSymbolTable(new SymbolTable("root", BITS));
  tree.ptr()->visit(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslSymbol Interpreter::lookup(const stdstr& n, bool traverse) {
  if (symbols.isSome())
    return symbols.ptr()->lookup(n, traverse);
  else
    return DslSymbol();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::define(DslSymbol s) {
  if (symbols.isSome()) symbols.ptr()->insert(s);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::pushScope(const stdstr& name) {
  symbols= DslSymbolTable(new SymbolTable(name, symbols));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslSymbolTable Interpreter::popScope() {
  if (symbols.isNull()) {
    return DslSymbolTable();
  }
  auto cur = symbols;
  symbols = cur.ptr()->enclosing;
  return cur;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Interpreter::readString() {
  stdstr s;
  std::cin >> s;
  return s;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Interpreter::readFloat() {
  double d;
  std::cin >> d;
  return d;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Interpreter::readInt() {
  llong n;
  std::cin >> n;
  return n;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::writeString(const stdstr& s) {
  ::printf("%s", s.c_str());
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::writeFloat(double d) {
  ::printf("%lf", d);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::writeInt(llong n) {
  std::cout << n;
  //::printf("%ld", n);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::writeln() {
  ::printf("%s", "\n");
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

