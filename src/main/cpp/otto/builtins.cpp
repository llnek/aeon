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
//#define TO_FLOAT(x) x.isInt() ? (double) x.getInt() : x.getFloat()
//#define TO_INT(x) x.isInt() ? x.getInt() : (llong) x.getFloat()
#define CHKSZ(x, e) ((x) >= 0 && (x) < (e))

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::otto {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame root_env();
d::DslValue op_num(double n) { return LNumber::make(n); }
d::DslValue op_num(llong n) { return LNumber::make(n); }
double not_zero(double d) { ASSERT1(d != 0.0); return d; }
llong not_zero(llong n) { ASSERT1(n != 0); return n; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
d::DslValue op_math(int op, T res, d::VSlice args) {
  for (int i= 0, e= args.size(); i < e; ++i) {
    auto s= cast_number(*(args.begin + i),1);
    switch (op) {
      case d::T_PLUS:
        if (s->isInt())
          res += s->getInt(); else res += s->getFloat();
      break;
      case d::T_MINUS:
        if (i==0 && e > 1) {
          if (s->isInt())
            res = s->getInt(); else res = s->getFloat();
        } else {
          if (s->isInt())
            res -= s->getInt(); else res -= s->getFloat();
        }
      break;
      case d::T_MULT:
        if (s->isInt())
          res *= s->getInt(); else res *= s->getFloat();
      break;
      case d::T_DIV:
        if (i==0 && e > 1) {
          if (s->isInt())
            res = s->getInt(); else res = s->getFloat();
        } else {
          if (s->isInt())
            res /= not_zero(s->getInt());
          else
            res /= not_zero(s->getFloat());
        }
      break;
    }
  }
  return op_num(res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_mul(Lisper* lisp, d::VSlice args) {
  // (* 1 2 3.3 4) (*)
  return scan_numbers(args)
    ? op_math<double>(d::T_MULT, 1.0, args)
    : op_math<llong>(d::T_MULT, 1, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_div(Lisper* lisp, d::VSlice args) {
  // (/ 1 2 3) or (/ 2)
  d::preMin(1, args.size(), "/");
  return scan_numbers(args)
    ? op_math<double>(d::T_DIV, 1.0, args)
    : op_math<llong>(d::T_DIV, 1, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_add(Lisper* lisp, d::VSlice args) {
  // (+ 1 2 3 3) (+)
  return scan_numbers(args)
    ? op_math<double>(d::T_PLUS, 0.0, args)
    : op_math<llong>(d::T_PLUS, 0, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_sub(Lisper* lisp, d::VSlice args) {
  // (- 1 2 3 3) (- 1)
  d::preMin(1, args.size(), "-");
  return scan_numbers(args)
    ? op_math<double>(d::T_MINUS, 0.0, args)
    : op_math<llong>(d::T_MINUS, 0, args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_lteq(Lisper* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (<= 1 2 3 4 4 5)
  d::preMin(1, args.size(), "<=");
  int i=1,e=args.size();
  scan_numbers(args);
  auto lhs = cast_number(*args.begin,1)->getFloat();
  for (; i < e; ++i) {
    auto a=*(args.begin+i);
    if (auto rhs = cast_number(a,1)->getFloat(); lhs <= rhs) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_gteq(Lisper* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (>= 5 5 4 3 2 1)
  d::preMin(1, args.size(), ">=");
  int i=1,e=args.size();
  scan_numbers(args);
  auto lhs = cast_number(*args.begin,1)->getFloat();
  for (; i < e; ++i) {
    auto a=*(args.begin+i);
    if (auto rhs = cast_number(a,1)->getFloat(); lhs >= rhs) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_lt(Lisper* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (< 1 2 3)
  d::preMin(1, args.size(), "<");
  int i=1, e= args.size();
  scan_numbers(args);
  auto lhs = cast_number(*args.begin,1)->getFloat();
  for (; i < e; ++i) {
    auto a=*(args.begin+i);
    if (auto rhs = cast_number(a,1)->getFloat(); lhs < rhs) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_gt(Lisper* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (> 3 2 1)
  d::preMin(1, args.size(), ">");
  int i=1,e=args.size();
  scan_numbers(args);
  auto lhs = cast_number(*args.begin,1)->getFloat();
  for (; i < e; ++i) {
    auto a=*(args.begin+i);
    if (auto rhs = cast_number(a,1)->getFloat(); lhs > rhs) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_eqeq(Lisper* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (== 3 3.0)
  d::preMin(1, args.size(), "==");
  int i=1,e=args.size();
  scan_numbers(args);
  auto lhs = cast_number(*args.begin,1)->getFloat();
  for (; i < e; ++i) {
    auto a= *(args.begin+i);
    if (auto rhs = cast_number(a,1)->getFloat(); a::fuzzy_equals(lhs, rhs)) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_eq(Lisper* lisp, d::VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (= 3 3.0) => false (= 3 3) => true
  d::preMin(1, args.size(), "=");
  auto lhs = *args.begin;
  auto j=1;
  for (; (args.begin+j) != args.end; ++j) {
    if (auto rhs= *(args.begin+j); lhs->equals(rhs)) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL((args.begin+j) == args.end);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_apply(Lisper* lisp, d::VSlice args) {
  // (apply + 1 2 3 [4 5])
  auto len = d::preMin(2, args.size(), "apply");
  auto op = cast_function(*args.begin);
  auto last= args.begin + (len - 1);
  ASSERT1(last != args.end);
  auto s= cast_seqable(*last, 1);
  d::ValVec pms;
  appendAll(args,1,len-1, pms);
  appendAll(s,pms);
  return op->invoke(lisp, d::VSlice(pms));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_assoc(Lisper* lisp, d::VSlice args) {
  // (assoc m :a 1)
  // (assoc m :a 1 :b 2)
  d::preMin(1, args.size(), "assoc");
  return cast_map(*args.begin, 1)->assoc(d::VSlice(args.begin+1,args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_atom(Lisper* lisp, d::VSlice args) {
  // (atom nil)
  d::preEqual(1, args.size(), "atom");
  return ATOM_VAL(*args.begin);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_concat(Lisper* lisp, d::VSlice args) {
  // (concat [] [1 2])
  auto len= args.size();
  if (len == 0 ||
      (len == 1 && cast_nil(*args.begin))) {
    return EMPTY_LIST();
  }
  d::ValVec out;
  for (auto i=0; (args.begin+i) != args.end; ++i) {
    auto x= *(args.begin+i);
    if (cast_nil(x)) { continue; }
    appendAll(cast_seqable(x,1), out);
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_setQ(Lisper* lisp, d::VSlice args) {
  // (set? x)
  d::preEqual(1, args.size(), "set?");
  return BOOL_VAL(cast_set(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_disj(Lisper* lisp, d::VSlice args) {
  // (disj #{1 2} 1)
  d::preMin(2, args.size(), "disj");
  auto s= cast_set(*args.begin,1);
  return s->disj(d::VSlice(args.begin+1,args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_conj(Lisper* lisp, d::VSlice args) {
  // (conj [1 2] 3 4 5)
  d::preMin(2, args.size(), "conj");
  if (auto s= cast_set(*args.begin); X_NIL(s)) {
    return s->conj(d::VSlice(args.begin+1,args.end));
  }
  else
  return cast_sequential(*args.begin, 1)->conj(d::VSlice(args.begin+1,args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_cons(Lisper* lisp, d::VSlice args) {
  // (cons 1 [2 3])
  d::preEqual(2, args.size(), "cons");
  d::ValVec out { *args.begin };
  appendAll(cast_seqable(*(args.begin+1), 1),out);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_containsQ(Lisper* lisp, d::VSlice args) {
  // (contains? {:a 1} :a)
  // (contains? [9 8 7] 1)
  d::preEqual(2, args.size(), "contains?");
  return X_NIL(cast_nil(*args.begin))
    ? *args.begin
    : BOOL_VAL(cast_seqable(*args.begin,1)->contains(*(args.begin+1)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_count(Lisper* lisp, d::VSlice args) {
  // (count [2 3])
  d::preEqual(1, args.size(), "count");
  return X_NIL(cast_nil(*args.begin))
    ? NUMBER_VAL(0)
    : NUMBER_VAL(cast_seqable(*args.begin,1)->count());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_deref(Lisper* lisp, d::VSlice args) {
  // (deref x)
  d::preEqual(1, args.size(), "deref");
  return cast_atom(*args.begin,1)->deref();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_dissoc(Lisper* lisp, d::VSlice args) {
  // (dissoc {:a 1} :a)
  auto len= d::preMin(1, args.size(), "dissoc");
  auto m = cast_map(*args.begin, 1);
  return (len == 1)
    ? *args.begin
    : m->dissoc(d::VSlice(args.begin+1, args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_emptyQ(Lisper* lisp, d::VSlice args) {
  // (empty? "") (empty? []) (empty? {:a 1})
  d::preEqual(1, args.size(), "empty?");
  if (cast_nil(*args.begin)) { return TRUE_VAL(); }
  auto m= cast_seqable(*args.begin);
  if(E_NIL(m))
    expected("Countable", *args.begin);
  return BOOL_VAL(m->count()==0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_eval(Lisper* lisp, d::VSlice args) {
  // (eval '(+ 1 2))
  d::preEqual(1, args.size(), "eval");
  return Lisper().EVAL(*args.begin, root_env());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_first(Lisper* lisp, d::VSlice args) {
  // (first [1 2])
  d::preEqual(1, args.size(), "first");
  if (cast_nil(*args.begin)) { return *args.begin; }
  auto s=cast_seqable(*args.begin);
  if (E_NIL(s)) expected("Seq'able", *args.begin);
  return s->first();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_fnQ(Lisper* lisp, d::VSlice args) {
  // (fn? "aa")
  d::preEqual(1, args.size(), "fn?");
  return BOOL_VAL(X_NIL(cast_function(*args.begin)) &&
                  E_NIL(cast_macro(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_get(Lisper* lisp, d::VSlice args) {
  // (get {:a 1} :a)
  auto len= d::preMin(2, args.size(), "get");
  auto m= cast_map(*args.begin, 1);
  auto s= s__cast(LSeqable, m);
  auto k= *(args.begin+1);
  if (s->contains(k)) { return m->get(k); }
  // not found provided
  return len > 2 ? *(args.begin+2) : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_hash_map(Lisper* lisp, d::VSlice args) {
  // (hash-map :a 1 :b 2)
  d::preEven(args.size(), "hash-map");
  auto m = LHash::make();
  return cast_map(m,1)->assoc(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_keys(Lisper* lisp, d::VSlice args) {
  // (keys {:a 1 :b 2})
  d::preEqual(1, args.size(), "keys");
  return cast_map(*args.begin, 1)->keys();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_keyword(Lisper* lisp, d::VSlice args) {
  // (keyword "aaa")
  d::preEqual(1, args.size(), "keyword");
  return X_NIL(cast_nil(*args.begin))
    ? *args.begin
    : KEYWORD_VAL(cast_string(*args.begin, 1)->impl());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_list(Lisper* lisp, d::VSlice args) {
  // (list 1 2 3)
  return LIST_VAL(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_macroQ(Lisper* lisp, d::VSlice args) {
  // (macro? x)
  d::preEqual(1, args.size(), "macro?");
  return BOOL_VAL(cast_macro(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_atomQ(Lisper* lisp, d::VSlice args) {
  // (atom? a)
  d::preEqual(1, args.size(), "atom?");
  return BOOL_VAL(cast_atom(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_keywordQ(Lisper* lisp, d::VSlice args) {
  // (keyword? a)
  d::preEqual(1, args.size(), "keyword?");
  return BOOL_VAL(cast_keyword(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_listQ(Lisper* lisp, d::VSlice args) {
  // (list? a)
  d::preEqual(1, args.size(), "list?");
  return BOOL_VAL(cast_list(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_vecQ(Lisper* lisp, d::VSlice args) {
  // (vector? a)
  d::preEqual(1, args.size(), "vector?");
  return BOOL_VAL(cast_vec(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_sequentialQ(Lisper* lisp, d::VSlice args) {
  // (sequential? a)
  d::preEqual(1, args.size(), "sequential?");
  return BOOL_VAL((cast_list(*args.begin) != nullptr ||
                  (cast_vec(*args.begin) != nullptr)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_seqQ(Lisper* lisp, d::VSlice args) {
  // (seq? a)
  d::preEqual(1, args.size(), "seq?");
  return BOOL_VAL(cast_seqable(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_mapQ(Lisper* lisp, d::VSlice args) {
  // (map? a)
  d::preEqual(1, args.size(), "map?");
  return BOOL_VAL(cast_map(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_map(Lisper* lisp, d::VSlice args) {
  // (map f [1 2 3])
  d::preEqual(2, args.size(), "map");
  auto pp = cast_function(*args.begin, 1);
  auto e= *(args.begin+1);
  if (cast_nil(e)) { return EMPTY_LIST(); }
  auto s= cast_seqable(e, 1);
  if (s->isEmpty()) { return EMPTY_LIST(); }
  d::ValVec out;
  for (auto i=0, e=s->count(); i < e; ++i) {
    d::ValVec p { s->nth(i) };
    s__conj(out, pp->invoke(lisp, d::VSlice(p)));
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_meta(Lisper* lisp, d::VSlice args) {
  // (meta x)
  d::preEqual(1, args.size(), "meta");
  return s__cast(LValue, (*args.begin).get())->meta();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_nth(Lisper* lisp, d::VSlice args) {
  // (nth x 2)
  auto len= d::preMin(2, args.size(), "nth");
  if (cast_nil(*args.begin)) { return NIL_VAL(); }
  auto pos= cast_number(*(args.begin+1), 1)->getInt();
  auto s= cast_seqable(*args.begin,1);
  return (!CHKSZ(pos,s->count()) && len > 2) ? *(args.begin+2) : s->nth(pos);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static stdstr print(d::VSlice args, bool pretty, const stdstr& sep) {
  stdstr out;
  for (auto i=0; (args.begin+i) != args.end; ++i) {
    if (!out.empty()) { out += sep; }
    out += s__cast(LValue, (*(args.begin+i)).get())->pr_str(pretty);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_pr_str(Lisper* lisp, d::VSlice args) {
  // (pr-str "a" 1 "b")
  return STRING_VAL(print(args, true, " "));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_println(Lisper* lisp, d::VSlice args) {
  // (println "a" 1 "b")
  ::printf("%s\n", C_STR(print(args,false, " ")));
  return NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_prn(Lisper* lisp, d::VSlice args) {
  // (prn "a" 1 "b")
  ::printf("%s\n", C_STR(print(args,true, " ")));
  return NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_read_string(Lisper* lisp, d::VSlice args) {
  // (read-string "(+ 1 2)")
  d::preEqual(1, args.size(), "read-string");
  auto s= cast_string(*args.begin, 1)->impl();
  auto ret= SExprParser(s.c_str()).parse();
  //::printf("ret count = %d\n", ret.first);
  return ret.second;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_resetBang(Lisper* lisp, d::VSlice args) {
  // (reset! a nil)
  d::preEqual(2, args.size(), "reset!");
  return cast_atom(*args.begin, 1)->reset(*(args.begin+1));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_rest(Lisper* lisp, d::VSlice args) {
  // (rest [1 2 3])
  d::preEqual(1, args.size(), "rest");
  if (cast_nil(*args.begin)) { return EMPTY_LIST(); }
  auto s = cast_seqable(*args.begin,1);
  d::ValVec out;
  appendAll(s,1,out);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_seq(Lisper* lisp, d::VSlice args) {
  // (seq [1 2 3])
  d::preEqual(1, args.size(), "seq");
  return X_NIL(cast_nil(*args.begin))
    ? *args.begin
    : cast_seqable(*args.begin,1)->seq();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_gensym(Lisper* lisp, d::VSlice args) {
  // (gensym "G_")
  auto len=d::preMax(1, args.size(), "gensym");
  stdstr pfx {"G__"};
  if (len > 0) {
    pfx= cast_string(*args.begin,1)->impl();
  }
  return STRING_VAL(gensym(pfx));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_slurp(Lisper* lisp, d::VSlice args) {
  // (slurp "some file")
  d::preEqual(1, args.size(), "slurp");
  return STRING_VAL(a::read_file(cast_string(*args.begin, 1)->impl().c_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_str(Lisper* lisp, d::VSlice args) {
  // (str 1 2 3)
  return STRING_VAL(print(args, false, ""));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_swapBang(Lisper* lisp, d::VSlice args) {
  // (swap! a f) (swap! a f 1 2 3)
  auto len = d::preMin(2, args.size(), "swap!");
  auto a= cast_atom(*args.begin, 1);
  auto op= cast_function(*(args.begin+1), 1);
  d::ValVec out {a->deref()};
  for (auto i= 2; i < len; ++i) {
    s__conj(out, *(args.begin+i));
  }
  return a->reset(op->invoke(lisp, d::VSlice(out)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_symbol(Lisper* lisp, d::VSlice args) {
  // (symbol "s")
  d::preEqual(1, args.size(), "symbol");
  return SYMBOL_VAL(cast_string(*args.begin, 1)->impl());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_throw(Lisper* lisp, d::VSlice args) {
  // (throw "aaa")
  d::preEqual(1, args.size(), "throw");
  throw *args.begin;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace cc= std::chrono;
static d::DslValue native_time_ms(Lisper* lisp, d::VSlice args) {
  // (time-ms)
  d::preEqual(0, args.size(), "time-ms");
  return NUMBER_VAL(cc::duration_cast<cc::milliseconds>(
                 cc::system_clock::now().time_since_epoch()).count());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_vals(Lisper* lisp, d::VSlice args) {
  d::preEqual(1, args.size(), "vals");
  return X_NIL(cast_nil(*args.begin))
         ? *args.begin
         : cast_map(*args.begin, 1)->vals();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_vector(Lisper* lisp, d::VSlice args) {
  // (vector 1 2 )
  return VEC_VAL(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_with_meta(Lisper* lisp, d::VSlice args) {
  // (with-meta obj m)
  d::preEqual(2, args.size(), "with-meta");
  return s__cast(LValue, (*args.begin).get())->withMeta(*(args.begin+1));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame init_natives() {
  auto env = d::Frame::make("root");
  env->set("*", FN_VAL("*", &native_mul));
  env->set("+", FN_VAL("+",&native_add));
  env->set("-", FN_VAL("-",&native_sub));
  env->set("/", FN_VAL("/",&native_div));
  env->set("<=", FN_VAL("<=",&native_lteq));
  env->set(">=", FN_VAL("<=",&native_gteq));
  env->set("==", FN_VAL("==",&native_eqeq));
  env->set("<", FN_VAL("<",&native_lt));
  env->set(">", FN_VAL(">",&native_gt));
  env->set("=", FN_VAL("=",&native_eq));
  env->set("apply", FN_VAL("apply",&native_apply));
  env->set("assoc", FN_VAL("assoc",&native_assoc));
  env->set("dissoc", FN_VAL("dissoc",&native_dissoc));
  env->set("atom", FN_VAL("atom",&native_atom));
  env->set("concat", FN_VAL("concat",&native_concat));
  env->set("conj", FN_VAL("conj",&native_conj));
  env->set("disj", FN_VAL("disj",&native_disj));
  env->set("gensym", FN_VAL("gensym",&native_gensym));
  env->set("contains?", FN_VAL("contains?",&native_containsQ));
  env->set("count", FN_VAL("count",&native_count));
  env->set("deref", FN_VAL("deref",&native_deref));
  env->set("empty?", FN_VAL("empty?",&native_emptyQ));
  env->set("eval", FN_VAL("eval",&native_eval));
  env->set("first", FN_VAL("first",&native_first));
  env->set("fn?", FN_VAL("fn?",&native_fnQ));
  env->set("cons", FN_VAL("cons",&native_cons));
  env->set("get", FN_VAL("get",&native_get));
  env->set("hash-map",FN_VAL("hash-map",&native_hash_map));
  env->set("keys", FN_VAL("keys",&native_keys));
  env->set("keyword", FN_VAL("keyword",&native_keyword));
  env->set("keyword?", FN_VAL("keyword?",&native_keywordQ));
  env->set("sequential?", FN_VAL("sequential?",&native_sequentialQ));
  env->set("list", FN_VAL("list",&native_list));
  env->set("macro?", FN_VAL("macro?",&native_macroQ));
  env->set("list?", FN_VAL("list?",&native_listQ));
  env->set("vector?", FN_VAL("vector?",&native_vecQ));
  env->set("atom?", FN_VAL("atom?",&native_atomQ));
  env->set("set?", FN_VAL("set?",&native_setQ));
  env->set("map?", FN_VAL("map?",&native_mapQ));
  env->set("seq?", FN_VAL("seq?",&native_seqQ));
  env->set("map", FN_VAL("map",&native_map));
  env->set("meta", FN_VAL("meta",&native_meta));
  env->set("nth", FN_VAL("nth",&native_nth));
  env->set("pr-str", FN_VAL("pr-str",&native_pr_str));
  env->set("println", FN_VAL("println",&native_println));
  env->set("prn", FN_VAL("prn",&native_prn));
  env->set("read-string", FN_VAL("read-string",&native_read_string));
  env->set("reset!", FN_VAL("reset!",&native_resetBang));
  env->set("rest", FN_VAL("rest",&native_rest));
  env->set("seq", FN_VAL("seq",&native_seq));
  env->set("slurp", FN_VAL("slurp",&native_slurp));
  env->set("str", FN_VAL("str",&native_str));
  env->set("swap!", FN_VAL("swap!",&native_swapBang));
  env->set("symbol", FN_VAL("symbol",&native_symbol));
  env->set("throw", FN_VAL("throw",&native_throw));
  env->set("time-ms", FN_VAL("time-ms",&native_time_ms));
  env->set("vals", FN_VAL("vals",&native_vals));
  env->set("vector", FN_VAL("vector",&native_vector));
  env->set("with-meta", FN_VAL("with-meta",&native_with_meta));
  return env;
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


