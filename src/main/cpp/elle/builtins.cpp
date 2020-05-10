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

#include <chrono>
#include "parser.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::elle {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//d::DFrame root_env();
d::DValue op_num(double n) { return SNumber::make(n); }
d::DValue op_num(llong n) { return SNumber::make(n); }
double not_zero(double d) { ASSERT1(d != 0.0); return d; }
llong not_zero(llong n) { ASSERT1(n != 0); return n; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool scan_numbers(d::VSlice vs) {
  auto r=false;
  for (auto i= 0; (vs.begin+i) != vs.end; ++i) {
    auto x= *(vs.begin+i);
    auto n= vcast<SNumber>(x);
    if (E_NIL(n)) { expected("number", x); }
    if (!n->isInt()) { r= true; }
  }
  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
d::DValue op_math(int op, T res, d::VSlice args) {
  for (int i= 0, e= args.size(); i < e; ++i) {
    auto s= vcast<SNumber>(*(args.begin + i));
    ASSERT1(s);
    switch (op) {
    case d::T_PLUS:
      if (s->isInt())
        res += s->getInt(); else res += s->getFloat();
    break;
    case d::T_MINUS:
      if (i==0 && e > 1)
      { if (s->isInt())
          res = s->getInt(); else res = s->getFloat(); }
      else
      { if (s->isInt())
          res -= s->getInt(); else res -= s->getFloat(); }
    break;
    case d::T_MULT:
      if (s->isInt())
        res *= s->getInt(); else res *= s->getFloat();
    break;
    case d::T_DIV:
      if (i==0 && e > 1)
      { if (s->isInt())
          res = s->getInt(); else res = s->getFloat(); }
      else
      { if (s->isInt())
          res /= not_zero(s->getInt());
        else
          res /= not_zero(s->getFloat()); }
    break;
    }
  }
  return op_num(res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_mul(Scheme* lisp, d::VSlice args) {
  // (* 1 2 3.3 4) (*)
  return scan_numbers(args)
    ? op_math<double>(d::T_MULT, 1.0, args)
    : op_math<llong>(d::T_MULT, 1, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_div(Scheme* lisp, d::VSlice args) {
  // (/ 1 2 3) or (/ 2)
  d::preMin(1, args.size(), "/");
  return scan_numbers(args)
    ? op_math<double>(d::T_DIV, 1.0, args)
    : op_math<llong>(d::T_DIV, 1, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_add(Scheme* lisp, d::VSlice args) {
  // (+ 1 2 3 3) (+)
  return scan_numbers(args)
    ? op_math<double>(d::T_PLUS, 0.0, args)
    : op_math<llong>(d::T_PLUS, 0, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_sub(Scheme* lisp, d::VSlice args) {
  // (- 1 2 3 3) (- 1)
  d::preMin(1, args.size(), "-");
  return scan_numbers(args)
    ? op_math<double>(d::T_MINUS, 0.0, args)
    : op_math<llong>(d::T_MINUS, 0, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_lteq(Scheme* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (<= 1 2 3 4 4 5)
  d::preMin(1, args.size(), "<=");
  int i=1,e=args.size();
  scan_numbers(args);
  auto lhs = vcast<SNumber>(*args.begin)->getFloat();
  for (; i < e; ++i) {
    auto a=*(args.begin+i);
    if (auto rhs = vcast<SNumber>(a)->getFloat(); lhs <= rhs) {
      lhs=rhs;
    } else break;
  }
  return i>=e ? STrue::make() : SFalse::make();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_gteq(Scheme* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (>= 5 5 4 3 2 1)
  d::preMin(1, args.size(), ">=");
  int i=1,e=args.size();
  scan_numbers(args);
  auto lhs = vcast<SNumber>(*args.begin)->getFloat();
  for (; i < e; ++i) {
    auto a=*(args.begin+i);
    if (auto rhs = vcast<SNumber>(a)->getFloat(); lhs >= rhs) {
      lhs=rhs;
    } else break;
  }
  return i>=e ? STrue::make() : SFalse::make();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_lt(Scheme* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (< 1 2 3)
  d::preMin(1, args.size(), "<");
  int i=1, e= args.size();
  scan_numbers(args);
  auto lhs = vcast<SNumber>(*args.begin)->getFloat();
  for (; i < e; ++i) {
    auto a=*(args.begin+i);
    if (auto rhs = vcast<SNumber>(a)->getFloat(); lhs < rhs) {
      lhs=rhs;
    } else break;
  }
  return i>=e ? STrue::make() : SFalse::make();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_gt(Scheme* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (> 3 2 1)
  d::preMin(1, args.size(), ">");
  int i=1,e=args.size();
  scan_numbers(args);
  auto lhs = vcast<SNumber>(*args.begin)->getFloat();
  for (; i < e; ++i) {
    auto a=*(args.begin+i);
    if (auto rhs = vcast<SNumber>(a)->getFloat(); lhs > rhs) {
      lhs=rhs;
    } else break;
  }
  return i>=e ? STrue::make() : SFalse::make();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_concat(Scheme* lisp, d::VSlice args) {
  // (append [] [1 2])
  auto len= args.size();
  if (len == 0 ||
      (len == 1 && vcast<SNil>(*args.begin))) {
    return SNil::make();
  }
  d::ValVec out;
  for (auto i=0; (args.begin+i) != args.end; ++i) {
    auto x= *(args.begin+i);
    if (vcast<SNil>(x)) { continue; }
    appendAll(vcast<SPair>(x), out);
  }
  return makeList(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_cons(Scheme* lisp, d::VSlice args) {
  // (cons 1 [2 3])
  d::preEqual(2, args.size(), "cons");
  d::ValVec out { *args.begin };
  appendAll(vcast<SPair>(*(args.begin+1)),out);
  return makeList(out);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_list(Scheme* lisp, d::VSlice args) {
  // (list 1 2 3)
  return makeList(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_listQ(Scheme* lisp, d::VSlice args) {
  // (list? a)
  d::preEqual(1, args.size(), "list?");
  return listQ(*args.begin) ? STrue::make() : SFalse::make();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_atomQ(Scheme* lisp, d::VSlice args) {
  // (atom? a)
  d::preEqual(1, args.size(), "atom?");
  return atomQ(*args.begin) ? STrue::make() : SFalse::make();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_gensym(Scheme* lisp, d::VSlice args) {
  // (gensym "G_")
  auto len=d::preMax(1, args.size(), "gensym");
  stdstr pfx {"G__"};
  if (len > 0) {
    pfx= vcast<SString>(*args.begin)->impl();
  }
  return SString::make(gensym(pfx));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame init_natives() {
  auto env = d::Frame::make("root");

  env->set("*", SNative::make("*", &native_mul));
  env->set("+", SNative::make("+",&native_add));
  env->set("-", SNative::make("-",&native_sub));
  env->set("/", SNative::make("/",&native_div));
  env->set("<=", SNative::make("<=",&native_lteq));
  env->set(">=", SNative::make("<=",&native_gteq));
  env->set("<", SNative::make("<",&native_lt));
  env->set(">", SNative::make(">",&native_gt));
  env->set("append", SNative::make("append",&native_concat));
  env->set("gensym", SNative::make("gensym",&native_gensym));
  env->set("cons", SNative::make("cons",&native_cons));
  env->set("list", SNative::make("list",&native_list));
  env->set("list?", SNative::make("list?",&native_listQ));
  env->set("atom?", SNative::make("atom?",&native_atomQ));

  return env;
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


