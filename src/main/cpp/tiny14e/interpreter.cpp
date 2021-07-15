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

#include <iostream>
#include "interpreter.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e{
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Interpreter::Interpreter(const Tchar* src){
  source = src;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Interpreter::interpret(){
  CrenshawParser p(source);
  auto tree= p.parse();
  return check(tree), eval(tree);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Interpreter::eval(d::DAst tree){
  return (pushFrame("root"), tree.get()->eval(this));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame Interpreter::pushFrame(cstdstr& name){
  stack = d::Frame::make(name, stack);
  return stack;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame Interpreter::popFrame(){
  if(stack){
    auto f= stack;
    //::printf("Frame pop'ed:=\n%s\n", f->pr_str().c_str());
    stack= stack->getOuter();
    return f;
  }else{
    return P_NIL;
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame Interpreter::peekFrame() const{
  return stack;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Interpreter::setValueEx(cstdstr& name, d::DValue v){
  auto x = peekFrame();
  return x ? x->setEx(name, v) : P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Interpreter::setValue(cstdstr& name, d::DValue v){
  auto x = peekFrame();
  return x ? x->set(name, v) : P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Interpreter::getValue(cstdstr& name) const{
  auto x = peekFrame();
  return x ? x->get(name) : P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<stdstr,d::DSymbol> BITS {
  {"INTEGER", d::Symbol::make("INTEGER")},
  {"REAL", d::Symbol::make("REAL")},
  {"STRING", d::Symbol::make("STRING")}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::check(d::DAst tree){
  symbols= d::Table::make("root", BITS);
  tree->visit(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DSymbol Interpreter::search(cstdstr& n) const{
  return symbols ? symbols->search(n) : P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DSymbol Interpreter::find(cstdstr& n) const{
  return symbols ? symbols->find(n) : P_NIL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DSymbol Interpreter::define(d::DSymbol s){
  if (symbols) symbols->insert(s);
  return s;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DTable Interpreter::pushScope(cstdstr& name){
  symbols= d::Table::make(name, symbols);
  return symbols;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DTable Interpreter::popScope(){
  if(!symbols)
    return P_NIL;
  auto cur = symbols;
  symbols = cur.get()->outer();
  return cur;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Interpreter::readString(){
  stdstr s;
  std::cin >> s;
  return s;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Interpreter::readFloat(){
  double d;
  std::cin >> d;
  return d;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Interpreter::readInt(){
  llong n;
  std::cin >> n;
  return n;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::writeString(cstdstr& s){
  ::printf("%s", s.c_str());
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::writeFloat(double d){
  ::printf("%lf", d);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::writeInt(llong n){
  std::cout << n;
  //::printf("%ld", n);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::writeln(){
  ::printf("%s", "\n");
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

