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
#include "parser.h"
#include "builtins.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame Basic::root_env() {
  return init_natives(pushFrame("root"));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Basic::interpret() {
  BasicParser p(source);
  root_env();
  dataSlots.clear();
  defs.clear();
  auto tree= p.parse();
  DEBUG("%s", PRN(tree));
  return check(tree), eval(tree);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::addData(d::DValue v) {
  DEBUG("addData(): %s", PRV(v,0));
  s__conj(dataSlots,v);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Basic::readData() {
  return s__index(dataPtr,dataSlots) ? dataSlots[dataPtr++] : DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::restore() { dataPtr=0; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::addLambda(d::DValue f) {
  defs[PNAME(Lambda,f)]=f;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Basic::eval(d::DAst tree) {
  init_counters();
  auto res= tree->eval(this);
  return (finz_counters(), res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame Basic::pushFrame(cstdstr& name) {
  return (stack = d::Frame::make(name, stack));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame Basic::popFrame() {
  if (stack) {
    auto f= stack;
    DEBUG("Frame: %s", PSTR(f));
    stack= stack->getOuter();
    return f;
  } else {
    return DENV_NIL;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame Basic::peekFrame() const { return stack; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Basic::setValueEx(cstdstr& name, d::DValue v) {
  RAISE(d::Unsupported, "Can't call %s", "setValueEx");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Basic::setValue(cstdstr& name, d::DValue v) {
  auto x = peekFrame();
  ensure_data_type(name,v);
  return x ? x->set(name, v) : DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue Basic::getValue(cstdstr& name) const {
  auto x = peekFrame();
  return x ? x->get(name) : DVAL_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static StrVec TYPES {"INT", "REAL", "STRING"};
static std::map<stdstr,d::DSymbol> BITS {
  {TYPES[0], d::Symbol::make(TYPES[0])},
  {TYPES[1], d::Symbol::make(TYPES[1])},
  {TYPES[2], d::Symbol::make(TYPES[2])}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::check(d::DAst tree) {
  symbols= d::Table::make("root", BITS);
  tree->visit(this);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DSymbol Basic::search(cstdstr& n) const {
  return symbols ? symbols->search(n) : P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DSymbol Basic::find(cstdstr& n) const {
  return symbols ? symbols->find(n) : P_NIL;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DSymbol Basic::define(d::DSymbol s) {
  if (symbols) symbols->insert(s);
  return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DTable Basic::pushScope(cstdstr& name) {
  return (symbols= d::Table::make(name, symbols));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DTable Basic::popScope() {
  if (!symbols)
    return P_NIL;
  auto cur = symbols;
  return (symbols = cur.get()->outer(), cur);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Basic::readString() {
  // get the whole line
  stdstr s; std::getline(std::cin,s); return s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Basic::readFloat() {
  auto s= readString();
  return s.empty() ? 0 : ::atof(s.c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Basic::readInt() {
  auto s= readString();
  return s.empty() ? 0 : ::atol(s.c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::writeString(cstdstr& s) { std::cout << s; }

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::writeFloat(double d) { std::cout << d; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::writeInt(llong n) { std::cout << n; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::writeln() { std::cout << "\n"; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::install(const std::map<int,int>& m) {
  // install the entire program, maps code lines
  // to linear array positions.
  for (auto& x : m)
    lines[_1(x)] = _2(x);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::uninstall() {
  dataSlots.clear();
  defs.clear();
  lines.clear();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::init_lambdas() {
  // install all user defined functions.
  for (auto& x : defs) {
    auto v= _2(x);
    auto p= DCAST(Lambda,v);
    setValue(p->name(), v);
    DEBUG("installed lambda: %s", p->name().c_str()); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define CLEAR_STACK(s) \
  do { while (!s.empty()) s.pop(); } while (0)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::init_counters() {
  running=true;
  dataPtr=0;
  progOffset=0;
  progCounter= -1;
  init_lambdas();
  CLEAR_STACK(gosubReturns);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::finz_counters() {
  running=false;
  dataPtr=0;
  progOffset=0;
  progCounter= -1;
  CLEAR_STACK(gosubReturns);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Basic::retSub() {
  if (gosubReturns.empty())
    RAISE(d::BadArg, "Bad gosub-return: %s", "no sub called");
  auto r= gosubReturns.top();
  gosubReturns.pop();
  // return to the *next* offset
  progOffset = _2(r) + 1;
  // return to the line before since pc always increment.
  return (progCounter = _1(r) - 1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Basic::jumpSub(int target, int from, int off) {
  auto it= lines.find(target);
  if (it == lines.end())
    RAISE(d::BadArg, "Bad gosub<%d>", target);

  // must!
  ASSERT1(progCounter == lines[from]);

  gosubReturns.push(s__pair(int,int,progCounter,off));
  auto pc = _2_(it);
  progOffset=0;
  // go one less since pc always increments.
  return (progCounter = pc-1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Basic::jump(int line) {
  auto it= lines.find(line);
  if (it == lines.end())
    RAISE(d::BadArg, "Bad goto<%d>", line);
  auto pos = _2_(it);
  progOffset=0;
  // go one less since pc always increments.
  return (progCounter = pos-1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Basic::jumpFor(DslFLInfo f) {
  auto it= lines.find(f->begin);
  if (it == lines.end())
    RAISE(d::BadArg, "Bad for-loop<%d>",  f->begin);
  progOffset=f->beginOffset;
  // always one less since pc always increments.
  return (progCounter = _2_(it) - 1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Basic::endFor(DslFLInfo f) {
  auto it= lines.find(f->end);
  if (it == lines.end())
    RAISE(d::BadArg, "Bad end-for<%d>", f->end);
  f->init=P_NIL;
  // when done goto next offset.
  progOffset=f->endOffset+1;
  // one less since pc always increments.
  return (progCounter = _2_(it) - 1);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::addForLoop(DslFLInfo f) {
  auto x= this->forLoop; // current outer for loop
  auto vn= f->var;
  bool bad=0;

  while (x) {
    bad = (x->var == vn); x=x->outer; }

  if (bad)
    E_SEMANTIC("For counter-var: %s reused", vn.c_str());

  f->outer= forLoop, forLoop=f;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::xrefForNext(int n, int pos) {
  xrefForNext("", n, pos);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Basic::xrefForNext(cstdstr& v, int n, int pos) {
  // make sure the next statement matches the current for loop.
  auto c = this->forLoop;
  // must!
  ASSERT1(c);

  if (!v.empty())
    if (!(c->var == v))
      E_SEMANTIC("Wanted forloop var: %s, got %s.",
                 c->var.c_str(), v.c_str());

  c->endOffset=pos;
  c->end= n;

  // find the corresponding counters
  auto b= this->lines[c->begin];
  auto e= this->lines[c->end];

  // we need to handle multi next on same line
  auto bkey= N_STR(b)+","+N_STR(c->beginOffset);
  auto ekey= N_STR(e)+","+N_STR(pos);

  //std::cout << "bkey= " << bkey << "\n";
  //std::cout << "ekey= " << ekey << "\n";

  this->forBegins[bkey] = c;
  this->forEnds[ekey] = c;

  // pop it
  forLoop=forLoop->outer;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFLInfo Basic::getForLoop(int c, int offset) const {
  auto k= N_STR(c)+","+N_STR(offset);

  if (auto i = forBegins.find(k);
      i != forBegins.end()) { return _2_(i); }

  if (auto i = forEnds.find(k);
      i != forEnds.end()) { return _2_(i); }

  E_SEMANTIC("Unknown for-loop<%d>, offset[%d]", c, offset);
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


