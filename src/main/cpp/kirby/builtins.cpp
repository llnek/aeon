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
#define TO_FLOAT(x) x.isInt() ? (double) x.getInt() : x.getFloat()
#define TO_INT(x) x.isInt() ? x.getInt() : (llong) x.getFloat()
#define CHKSZ(x, e) ((x) >= 0 && (x) < (e))

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::kirby {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame root_env();
LFloat* op_num(double n) { return new LFloat(n); }
LInt* op_num(llong n) { return new LInt(n); }
double not_zero(double d) { ASSERT1(d != 0.0); return d; }
llong not_zero(llong n) { ASSERT1(n != 0); return n; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
d::DslValue op_math(int op, T res, NumberVec& slots) {
  for (int i= 0, e= slots.size(); i < e; ++i) {
    auto s= slots[i];
    auto sn= s.getInt();
    auto sf= s.getFloat();
    switch (op) {
      case d::T_PLUS:
        if (s.isInt())
          res += sn; else res += sf;
      break;
      case d::T_MINUS:
        if (i==0 && e > 1) {
          if (s.isInt())
            res = sn; else res = sf;
        } else {
          if (s.isInt())
            res -= sn; else res -= sf;
        }
      break;
      case d::T_MULT:
        if (s.isInt())
          res *= sn; else res *= sf;
      break;
      case d::T_DIV:
        if (i==0 && e > 1) {
          if (s.isInt())
            res = sn; else res = sf;
        } else {
          if (s.isInt())
            res /= not_zero(sn);
          else
            res /= not_zero(sf);
        }
      break;
    }
  }
  return d::DslValue(op_num(res));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_mul(Lisper* lisp, VSlice args) {
  // (* 1 2 3.3 4) (*)
  NumberVec slots;
  return cast_numeric(args,slots)
    ? op_math<double>(d::T_MULT, 1.0, slots)
    : op_math<llong>(d::T_MULT, 1, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_div(Lisper* lisp, VSlice args) {
  // (/ 1 2 3) or (/ 2)
  preMin(1, args.size(), "/");
  NumberVec slots;
  return cast_numeric(args,slots)
    ? op_math<double>(d::T_DIV, 1.0, slots)
    : op_math<llong>(d::T_DIV, 1, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_add(Lisper* lisp, VSlice args) {
  // (+ 1 2 3 3) (+)
  NumberVec slots;
  return cast_numeric(args,slots)
    ? op_math<double>(d::T_PLUS, 0.0, slots)
    : op_math<llong>(d::T_PLUS, 0, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_sub(Lisper* lisp, VSlice args) {
  // (- 1 2 3 3) (- 1)
  preMin(1, args.size(), "-");
  NumberVec slots;
  return cast_numeric(args,slots)
    ? op_math<double>(d::T_MINUS, 0.0, slots)
    : op_math<llong>(d::T_MINUS, 0, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_lteq(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (<= 1 2 3 4 4 5)
  preMin(1, args.size(), "<=");
  NumberVec slots;
  cast_numeric(args,slots);
  int i=1,e=slots.size();
  auto lhs = TO_FLOAT(slots[0]);
  for (; i < e; ++i) {
    auto a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs <= rhs) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_gteq(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (>= 5 5 4 3 2 1)
  preMin(1, args.size(), ">=");
  NumberVec slots;
  cast_numeric(args,slots);
  int i=1,e=slots.size();
  auto lhs = TO_FLOAT(slots[0]);
  for (; i < e; ++i) {
    auto a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs >= rhs) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_lt(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (< 1 2 3)
  preMin(1, args.size(), "<");
  NumberVec slots;
  cast_numeric(args,slots);
  int i=1, e= slots.size();
  auto lhs = TO_FLOAT(slots[0]);
  for (; i < e; ++i) {
    auto a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs < rhs) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_gt(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (> 3 2 1)
  preMin(1, args.size(), ">");
  NumberVec slots;
  cast_numeric(args,slots);
  int i=1,e=slots.size();
  auto lhs = TO_FLOAT(slots[0]);
  for (; i < e; ++i) {
    auto a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs > rhs) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_eqeq(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (== 3 3.0)
  preMin(1, args.size(), "==");
  NumberVec slots;
  cast_numeric(args,slots);
  int i=1,e=slots.size();
  auto lhs = TO_FLOAT(slots[0]);
  for (; i < e; ++i) {
    auto a= slots[i];
    if (auto rhs = TO_FLOAT(a); a::fuzzy_equals(lhs, rhs)) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL(i>=e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_eq(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (= 3 3.0) => false (= 3 3) => true
  preMin(1, args.size(), "=");
  auto lhs = *args.begin;
  auto j=1;
  for (; (args.begin+j) != args.end; ++j) {
    if (auto rhs= *(args.begin+j); lhs->equals(rhs.ptr())) {
      lhs=rhs;
    } else { break; }
  }
  return BOOL_VAL((args.begin+j) == args.end);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_apply(Lisper* lisp, VSlice args) {
  // (apply + 1 2 3 [4 5])
  auto len = preMin(2, args.size(), "apply");
  auto op = cast_function(*args.begin);
  auto last= args.begin + (len - 1);
  ASSERT1(last != args.end);
  auto s= cast_seqable(*last, 1);
  VVec pms;
  appendAll(args,1,len-1, pms);
  appendAll(s,pms);
  return op->invoke(lisp, VSlice(pms));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_assoc(Lisper* lisp, VSlice args) {
  // (assoc m :a 1)
  // (assoc m :a 1 :b 2)
  preMin(1, args.size(), "assoc");
  return cast_map(*args.begin, 1)->assoc(VSlice(args.begin+1,args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_atom(Lisper* lisp, VSlice args) {
  // (atom nil)
  preEqual(1, args.size(), "atom");
  return ATOM_VAL(*args.begin);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_concat(Lisper* lisp, VSlice args) {
  // (concat [] [1 2])
  auto len= args.size();
  if (len == 0 ||
      (len == 1 && cast_nil(*args.begin))) {
    return EMPTY_LIST();
  }
  VVec out;
  for (auto i=0; (args.begin+i) != args.end; ++i) {
    auto x= *(args.begin+i);
    if (cast_nil(x)) { continue; }
    appendAll(cast_seqable(x,1), out);
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_setQ(Lisper* lisp, VSlice args) {
  // (set? x)
  preEqual(1, args.size(), "set?");
  return BOOL_VAL(cast_set(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_disj(Lisper* lisp, VSlice args) {
  // (disj #{1 2} 1)
  preMin(2, args.size(), "disj");
  auto s= cast_set(*args.begin,1);
  return s->disj(VSlice(args.begin+1,args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_conj(Lisper* lisp, VSlice args) {
  // (conj [1 2] 3 4 5)
  preMin(2, args.size(), "conj");
  if (auto s= cast_set(*args.begin); X_NIL(s)) {
    return s->conj(VSlice(args.begin+1,args.end));
  }
  else
  return cast_seq(*args.begin, 1)->conj(VSlice(args.begin+1,args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_cons(Lisper* lisp, VSlice args) {
  // (cons 1 [2 3])
  preEqual(2, args.size(), "cons");
  VVec out { *args.begin };
  appendAll(cast_seqable(*(args.begin+1), 1),out);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_containsQ(Lisper* lisp, VSlice args) {
  // (contains? {:a 1} :a)
  // (contains? [9 8 7] 1)
  preEqual(2, args.size(), "contains?");
  return X_NIL(cast_nil(*args.begin))
    ? *args.begin
    : BOOL_VAL(cast_seqable(*args.begin,1)->contains(*(args.begin+1)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_count(Lisper* lisp, VSlice args) {
  // (count [2 3])
  preEqual(1, args.size(), "count");
  return X_NIL(cast_nil(*args.begin))
    ? INT_VAL(0)
    : INT_VAL(cast_seqable(*args.begin,1)->count());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_deref(Lisper* lisp, VSlice args) {
  // (deref x)
  preEqual(1, args.size(), "deref");
  return cast_atom(*args.begin,1)->deref();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_dissoc(Lisper* lisp, VSlice args) {
  // (dissoc {:a 1} :a)
  auto len= preMin(1, args.size(), "dissoc");
  auto m = cast_map(*args.begin, 1);
  return (len == 1)
    ? *args.begin
    : m->dissoc(VSlice(args.begin+1, args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_emptyQ(Lisper* lisp, VSlice args) {
  // (empty? "") (empty? []) (empty? {:a 1})
  preEqual(1, args.size(), "empty?");
  if (cast_nil(*args.begin)) { return TRUE_VAL(); }
  auto m= cast_seqable(*args.begin);
  if(E_NIL(m))
    expected("Countable", *args.begin);
  return BOOL_VAL(m->count()==0);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_eval(Lisper* lisp, VSlice args) {
  // (eval '(+ 1 2))
  preEqual(1, args.size(), "eval");
  return Lisper().EVAL(*args.begin, root_env());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_first(Lisper* lisp, VSlice args) {
  // (first [1 2])
  preEqual(1, args.size(), "first");
  if (cast_nil(*args.begin)) { return *args.begin; }
  auto s=cast_seqable(*args.begin);
  if (E_NIL(s)) expected("Seq'able", *args.begin);
  return s->first();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_fnQ(Lisper* lisp, VSlice args) {
  // (fn? "aa")
  preEqual(1, args.size(), "fn?");
  return BOOL_VAL(X_NIL(cast_function(*args.begin)) &&
                  E_NIL(cast_macro(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_get(Lisper* lisp, VSlice args) {
  // (get {:a 1} :a)
  auto len= preMin(2, args.size(), "get");
  auto m= cast_map(*args.begin, 1);
  auto s= s__cast(LSeqable, m);
  auto k= *(args.begin+1);
  if (s->contains(k)) { return m->get(k); }
  // not found provided
  return len > 2 ? *(args.begin+2) : NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_hash_map(Lisper* lisp, VSlice args) {
  // (hash-map :a 1 :b 2)
  preEven(args.size(), "hash-map");
  LHash m;
  return m.assoc(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_keys(Lisper* lisp, VSlice args) {
  // (keys {:a 1 :b 2})
  preEqual(1, args.size(), "keys");
  return cast_map(*args.begin, 1)->keys();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_keyword(Lisper* lisp, VSlice args) {
  // (keyword "aaa")
  preEqual(1, args.size(), "keyword");
  return X_NIL(cast_nil(*args.begin))
    ? *args.begin
    : KEYWORD_VAL(cast_string(*args.begin, 1)->impl());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_list(Lisper* lisp, VSlice args) {
  // (list 1 2 3)
  return LIST_VAL(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_macroQ(Lisper* lisp, VSlice args) {
  // (macro? x)
  preEqual(1, args.size(), "macro?");
  return BOOL_VAL(cast_macro(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_atomQ(Lisper* lisp, VSlice args) {
  // (atom? a)
  preEqual(1, args.size(), "atom?");
  return BOOL_VAL(cast_atom(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_keywordQ(Lisper* lisp, VSlice args) {
  // (keyword? a)
  preEqual(1, args.size(), "keyword?");
  return BOOL_VAL(cast_keyword(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_listQ(Lisper* lisp, VSlice args) {
  // (list? a)
  preEqual(1, args.size(), "list?");
  return BOOL_VAL(cast_list(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_vecQ(Lisper* lisp, VSlice args) {
  // (vector? a)
  preEqual(1, args.size(), "vector?");
  return BOOL_VAL(cast_vec(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_sequentialQ(Lisper* lisp, VSlice args) {
  // (sequential? a)
  preEqual(1, args.size(), "sequential?");
  return BOOL_VAL((cast_list(*args.begin) != nullptr ||
                  (cast_vec(*args.begin) != nullptr)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_seqQ(Lisper* lisp, VSlice args) {
  // (seq? a)
  preEqual(1, args.size(), "seq?");
  return BOOL_VAL(cast_seqable(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_mapQ(Lisper* lisp, VSlice args) {
  // (map? a)
  preEqual(1, args.size(), "map?");
  return BOOL_VAL(cast_map(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_map(Lisper* lisp, VSlice args) {
  // (map f [1 2 3])
  preEqual(2, args.size(), "map");
  auto pp = cast_function(*args.begin, 1);
  auto e= *(args.begin+1);
  if (cast_nil(e)) { return EMPTY_LIST(); }
  auto s= cast_seqable(e, 1);
  if (s->isEmpty()) { return EMPTY_LIST(); }
  VVec out;
  for (auto i=0, e=s->count(); i < e; ++i) {
    VVec p { s->nth(i) };
    s__conj(out, pp->invoke(lisp, VSlice(p)));
  }
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_meta(Lisper* lisp, VSlice args) {
  // (meta x)
  preEqual(1, args.size(), "meta");
  return s__cast(LValue, (*args.begin).ptr())->meta();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_nth(Lisper* lisp, VSlice args) {
  // (nth x 2)
  auto len= preMin(2, args.size(), "nth");
  if (cast_nil(*args.begin)) { return NIL_VAL(); }
  auto pos= cast_int(*(args.begin+1), 1)->impl();
  auto s= cast_seqable(*args.begin,1);
  return (!CHKSZ(pos,s->count()) && len > 2) ? *(args.begin+2) : s->nth(pos);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static stdstr print(VSlice args, bool pretty, const stdstr& sep) {
  stdstr out;
  for (auto i=0; (args.begin+i) != args.end; ++i) {
    if (!out.empty()) { out += sep; }
    out += s__cast(LValue, (*(args.begin+i)).ptr())->toString(pretty);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_pr_str(Lisper* lisp, VSlice args) {
  // (pr-str "a" 1 "b")
  return STRING_VAL(print(args, true, " "));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_println(Lisper* lisp, VSlice args) {
  // (println "a" 1 "b")
  ::printf("%s\n", C_STR(print(args,false, " ")));
  return NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_prn(Lisper* lisp, VSlice args) {
  // (prn "a" 1 "b")
  ::printf("%s\n", C_STR(print(args,true, " ")));
  return NIL_VAL();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_read_string(Lisper* lisp, VSlice args) {
  // (read-string "(+ 1 2)")
  preEqual(1, args.size(), "read-string");
  auto s= cast_string(*args.begin, 1)->impl();
  auto ret= SExprParser(s.c_str()).parse();
  //::printf("ret count = %d\n", ret.first);
  return ret.second;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_resetBang(Lisper* lisp, VSlice args) {
  // (reset! a nil)
  preEqual(2, args.size(), "reset!");
  return cast_atom(*args.begin, 1)->reset(*(args.begin+1));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_rest(Lisper* lisp, VSlice args) {
  // (rest [1 2 3])
  preEqual(1, args.size(), "rest");
  if (cast_nil(*args.begin)) { return EMPTY_LIST(); }
  auto s = cast_seqable(*args.begin,1);
  VVec out;
  appendAll(s,1,out);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_seq(Lisper* lisp, VSlice args) {
  // (seq [1 2 3])
  preEqual(1, args.size(), "seq");
  return X_NIL(cast_nil(*args.begin))
    ? *args.begin
    : cast_seqable(*args.begin,1)->seq();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_gensym(Lisper* lisp, VSlice args) {
  // (gensym "G_")
  auto len=preMax(1, args.size(), "gensym");
  stdstr pfx {"G__"};
  if (len > 0) {
    pfx= cast_string(*args.begin,1)->impl();
  }
  return STRING_VAL(gensym(pfx));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_slurp(Lisper* lisp, VSlice args) {
  // (slurp "some file")
  preEqual(1, args.size(), "slurp");
  return STRING_VAL(a::read_file(cast_string(*args.begin, 1)->impl().c_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_str(Lisper* lisp, VSlice args) {
  // (str 1 2 3)
  return STRING_VAL(print(args, false, ""));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_swapBang(Lisper* lisp, VSlice args) {
  // (swap! a f) (swap! a f 1 2 3)
  auto len = preMin(2, args.size(), "swap!");
  auto a= cast_atom(*args.begin, 1);
  auto op= cast_function(*(args.begin+1), 1);
  VVec out {a->deref()};
  for (auto i= 2; i < len; ++i) {
    s__conj(out, *(args.begin+i));
  }
  return a->reset(op->invoke(lisp, VSlice(out)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_symbol(Lisper* lisp, VSlice args) {
  // (symbol "s")
  preEqual(1, args.size(), "symbol");
  return SYMBOL_VAL(cast_string(*args.begin, 1)->impl());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_throw(Lisper* lisp, VSlice args) {
  // (throw "aaa")
  preEqual(1, args.size(), "throw");
  throw *args.begin;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace cc= std::chrono;
static d::DslValue native_time_ms(Lisper* lisp, VSlice args) {
  // (time-ms)
  preEqual(0, args.size(), "time-ms");
  return INT_VAL(cc::duration_cast<cc::milliseconds>(
                 cc::high_resolution_clock::now().time_since_epoch()).count());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_vals(Lisper* lisp, VSlice args) {
  preEqual(1, args.size(), "vals");
  return X_NIL(cast_nil(*args.begin))
    ? *args.begin
    : cast_map(*args.begin, 1)->vals();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_vector(Lisper* lisp, VSlice args) {
  // (vector 1 2 )
  return VEC_VAL(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_with_meta(Lisper* lisp, VSlice args) {
  // (with-meta obj m)
  preEqual(2, args.size(), "with-meta");
  return s__cast(LValue, (*args.begin).ptr())->withMeta(*(args.begin+1));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::Frame* init_natives(d::Frame* env) {
  env->set("*", FN_VAL("*", &native_mul), true);
  env->set("+", FN_VAL("+",&native_add), true);
  env->set("-", FN_VAL("-",&native_sub), true);
  env->set("/", FN_VAL("/",&native_div), true);
  env->set("<=", FN_VAL("<=",&native_lteq), true);
  env->set(">=", FN_VAL("<=",&native_gteq), true);
  env->set("==", FN_VAL("==",&native_eqeq),true);
  env->set("<", FN_VAL("<",&native_lt),true);
  env->set(">", FN_VAL(">",&native_gt),true);
  env->set("=", FN_VAL("=",&native_eq),true);
  env->set("apply", FN_VAL("apply",&native_apply),true);
  env->set("assoc", FN_VAL("assoc",&native_assoc),true);
  env->set("dissoc", FN_VAL("dissoc",&native_dissoc),true);
  env->set("atom", FN_VAL("atom",&native_atom),true);
  env->set("concat", FN_VAL("concat",&native_concat),true);
  env->set("conj", FN_VAL("conj",&native_conj),true);
  env->set("disj", FN_VAL("disj",&native_disj),true);
  env->set("gensym", FN_VAL("gensym",&native_gensym),true);
  env->set("contains?", FN_VAL("contains?",&native_containsQ),true);
  env->set("count", FN_VAL("count",&native_count),true);
  env->set("deref", FN_VAL("deref",&native_deref),true);
  env->set("empty?", FN_VAL("empty?",&native_emptyQ),true);
  env->set("eval", FN_VAL("eval",&native_eval),true);
  env->set("first", FN_VAL("first",&native_first),true);
  env->set("fn?", FN_VAL("fn?",&native_fnQ),true);
  env->set("cons", FN_VAL("cons",&native_cons),true);
  env->set("get", FN_VAL("get",&native_get),true);
  env->set("hash-map",FN_VAL("hash-map",&native_hash_map),true);
  env->set("keys", FN_VAL("keys",&native_keys),true);
  env->set("keyword", FN_VAL("keyword",&native_keyword),true);
  env->set("keyword?", FN_VAL("keyword?",&native_keywordQ),true);
  env->set("sequential?", FN_VAL("sequential?",&native_sequentialQ),true);
  env->set("list", FN_VAL("list",&native_list),true);
  env->set("macro?", FN_VAL("macro?",&native_macroQ),true);
  env->set("list?", FN_VAL("list?",&native_listQ),true);
  env->set("vector?", FN_VAL("vector?",&native_vecQ),true);
  env->set("atom?", FN_VAL("atom?",&native_atomQ),true);
  env->set("set?", FN_VAL("set?",&native_setQ),true);
  env->set("map?", FN_VAL("map?",&native_mapQ),true);
  env->set("seq?", FN_VAL("seq?",&native_seqQ),true);
  env->set("map", FN_VAL("map",&native_map),true);
  env->set("meta", FN_VAL("meta",&native_meta),true);
  env->set("nth", FN_VAL("nth",&native_nth),true);
  env->set("pr-str", FN_VAL("pr-str",&native_pr_str),true);
  env->set("println", FN_VAL("println",&native_println),true);
  env->set("prn", FN_VAL("prn",&native_prn),true);
  env->set("read-string", FN_VAL("read-string",&native_read_string),true);
  env->set("reset!", FN_VAL("reset!",&native_resetBang),true);
  env->set("rest", FN_VAL("rest",&native_rest),true);
  env->set("seq", FN_VAL("seq",&native_seq),true);
  env->set("slurp", FN_VAL("slurp",&native_slurp),true);
  env->set("str", FN_VAL("str",&native_str),true);
  env->set("swap!", FN_VAL("swap!",&native_swapBang),true);
  env->set("symbol", FN_VAL("symbol",&native_symbol),true);
  env->set("throw", FN_VAL("throw",&native_throw),true);
  env->set("time-ms", FN_VAL("time-ms",&native_time_ms),true);
  env->set("vals", FN_VAL("vals",&native_vals),true);
  env->set("vector", FN_VAL("vector",&native_vector),true);
  env->set("with-meta", FN_VAL("with-meta",&native_with_meta),true);
  return env;
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


