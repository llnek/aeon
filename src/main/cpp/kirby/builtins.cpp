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
#define TO_FLOAT(x) x.isInt() ? (double) x.u.n : x.u.r
#define TO_INT(x) x.isInt() ? x.u.n : (llong) x.u.r
#define CHKSZ(x, e) ((x) >= 0 && (x) < (e))
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::kirby {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame root_env();
LFloat* op_num(double n) { return new LFloat(n); }
LInt* op_num(llong n) { return new LInt(n); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
d::DslValue op_math(int op, T res, NumberSlots& slots) {
  for (int i= 0, e= slots.size(); i < e; ++i) {
    auto s= slots[i];
    switch (op) {
      case d::T_PLUS:
        if (s.isInt()) { res += s.u.n; }
        else { res += s.u.r; }
      break;
      case d::T_MINUS:
        if (i==0 && e > 1) {
          if (s.isInt()) { res = s.u.n; }
          else { res = s.u.r; }
        } else {
          if (s.isInt()) { res -= s.u.n; }
          else { res -= s.u.r; }
        }
      break;
      case d::T_MULT:
        if (s.isInt()) { res *= s.u.n; }
        else { res *= s.u.r; }
      break;
      case d::T_DIV:
        if (i==0 && e > 1) {
          if (s.isInt()) { res = s.u.n; }
          else { res = s.u.r; }
        } else {
          if (s.isInt()) { ASSERT1(s.u.n != 0); res /= s.u.n; }
          else { ASSERT1(s.u.r != 0); res /= s.u.r; }
        }
      break;
    }
  }
  return d::DslValue(op_num(res));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_mul(Lisper* lisp, VSlice args) {
  // (* 1 2 3.3 4) (*)
  NumberSlots slots;
  return cast_numeric(args,slots)
    ? op_math<double>(d::T_MULT, 1.0, slots)
    : op_math<llong>(d::T_MULT, 1, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_div(Lisper* lisp, VSlice args) {
  // (/ 1 2 3) or (/ 2)
  preMin(1, args.size(), "/");
  NumberSlots slots;
  return cast_numeric(args,slots)
    ? op_math<double>(d::T_DIV, 1.0, slots)
    : op_math<llong>(d::T_DIV, 1, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_add(Lisper* lisp, VSlice args) {
  // (+ 1 2 3 3) (+)
  NumberSlots slots;
  return cast_numeric(args,slots)
    ? op_math<double>(d::T_PLUS, 0.0, slots)
    : op_math<llong>(d::T_PLUS, 0, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_sub(Lisper* lisp, VSlice args) {
  // (- 1 2 3 3) (- 1)
  preMin(1, args.size(), "-");
  NumberSlots slots;
  return cast_numeric(args,slots)
    ? op_math<double>(d::T_MINUS, 0.0, slots)
    : op_math<llong>(d::T_MINUS, 0, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_lteq(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (<= 1 2 3 4 4 5)
  preMin(1,args.size(), "<=");
  NumberSlots slots;
  cast_numeric(args,slots);
  auto a= slots[0];
  auto lhs = TO_FLOAT(a);
  for (int i=1,e=slots.size(); i < e; ++i) {
    a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs <= rhs) {
      lhs=rhs;
    } else { return false_value(); }
  }
  return true_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_gteq(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (>= 5 5 4 3 2 1)
  preMin(1, args.size(), ">=");
  NumberSlots slots;
  cast_numeric(args,slots);
  auto a= slots[0];
  auto lhs = TO_FLOAT(a);
  for (int i=1,e=slots.size(); i < e; ++i) {
    a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs >= rhs) {
      lhs=rhs;
    } else { return false_value(); }
  }
  return true_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_lt(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (< 1 2 3)
  preMin(1, args.size(), "<");
  NumberSlots slots;
  cast_numeric(args,slots);
  auto a= slots[0];
  auto lhs = TO_FLOAT(a);
  for (auto i=1; i < slots.size(); ++i) {
    a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs < rhs) {
      lhs=rhs;
    } else { return false_value(); }
  }
  return true_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_gt(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (> 3 2 1)
  preMin(1, args.size(), ">");
  NumberSlots slots;
  cast_numeric(args,slots);
  auto a= slots[0];
  auto lhs = TO_FLOAT(a);
  for (auto i=1; i < slots.size(); ++i) {
    a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs > rhs) {
      lhs=rhs;
    } else { return false_value(); }
  }
  return true_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_eqeq(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (== 3 3.0)
  preMin(1, args.size(), "==");
  NumberSlots slots;
  cast_numeric(args,slots);
  auto a= slots[0];
  auto lhs = TO_FLOAT(a);
  for (auto i=1; i < slots.size(); ++i) {
    a= slots[i];
    if (auto rhs = TO_FLOAT(a); a::fuzzy_equals(lhs, rhs)) {
      lhs=rhs;
    } else { return false_value(); }
  }
  return true_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_eq(Lisper* lisp, VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (= 3 3.0) => false (= 3 3) => true
  preMin(1, args.size(), "=");
  auto lhs = *args.begin;
  auto j=1;
  for (auto i= args.begin+j; i != args.end; i= args.begin+j) {
    if (auto rhs= *i; lhs->equals(rhs.ptr())) {
      lhs=rhs;
    } else { false_value(); }
  }
  return true_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_apply(Lisper* lisp, VSlice args) {
  // (apply + 1 2 3 [4 5])
  auto len = preMin(2, args.size(), "apply");
  auto op = cast_function(*args.begin);
  auto last= args.begin + (len - 1);
  ASSERT1(last != args.end);
  auto s= cast_seq(*last, 1);
  VVec pms;
  appendAll(args,1,pms);
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
  return atom_value(*args.begin);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_concat(Lisper* lisp, VSlice args) {
  // (concat [] [1 2])
  auto len= args.size();
  if (len == 0 ||
      (len == 1 && cast_nil(*args.begin))) {
    return empty_list();
  }
  VVec out;
  for (auto i=0; (args.begin+i) != args.end; ++i) {
    auto x= *(args.begin+i);
    if (cast_nil(x)) { continue; }
    appendAll(cast_seq(x,1), out);
    ++i;
  }
  return list_value(VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_conj(Lisper* lisp, VSlice args) {
  // (conj [1 2] 3 4 5)
  preMin(2, args.size(), "conj");
  return cast_seq(*args.begin, 1)->conj(VSlice(args.begin+1,args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_cons(Lisper* lisp, VSlice args) {
  // (cons 1 [2 3])
  preEqual(2, args.size(), "cons");
  auto s = cast_seq(*(args.begin+1), 1);
  VVec out { *args.begin };
  appendAll(s,out);
  return list_value(VSlice(out));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_containsQ(Lisper* lisp, VSlice args) {
  // (contains? {:a 1} :a)
  // (contains? [9 8 7] 1)
  preEqual(2, args.size(), "contains?");
  return X_NIL(cast_nil(*args.begin))
    ? *args.begin
    : bool_value(cast_seqable(*args.begin,1)->contains(*(args.begin+1)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_count(Lisper* lisp, VSlice args) {
  // (count [2 3])
  preEqual(1, args.size(), "count");
  return X_NIL(cast_nil(*args.begin))
    ? int_value(0)
    : int_value(cast_seqable(*args.begin,1)->count());
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
  if (len == 1) { return *args.begin; }
  return m->dissoc(VSlice(args.begin+1, args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_emptyQ(Lisper* lisp, VSlice args) {
  // (empty? "") (empty? []) (empty? {:a 1})
  preEqual(1, args.size(), "empty?");
  if (auto m= cast_map(*args.begin); X_NIL(m)) {
    return bool_value(m->isEmpty());
  }
  if (auto s= cast_seq(*args.begin); X_NIL(s)) {
    return bool_value(s->isEmpty());
  }
  if (auto s= cast_string(*args.begin); X_NIL(s)) {
    return bool_value(s->isEmpty());
  }
  return expected("Countable", *args.begin);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_eval(Lisper* lisp, VSlice args) {
  // (eval '(+ 1 2))
  preEqual(1, args.size(), "eval");
  Lisper p;
  return p.EVAL(*args.begin, root_env());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_first(Lisper* lisp, VSlice args) {
  // (first [1 2])
  preEqual(1, args.size(), "first");
  if (auto n= cast_nil(*args.begin); X_NIL(n)) {
    return *args.begin;
  }
  if (auto s=cast_seqable(*args.begin); X_NIL(s)) {
    return s->first();
  }
  return expected("Seq'able", *args.begin);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_fnQ(Lisper* lisp, VSlice args) {
  // (fn? "aa")
  preEqual(1, args.size(), "fn?");
  return bool_value(X_NIL(cast_function(*args.begin)) &&
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
  return len > 2 ? *(args.begin+2) : nil_value();
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
    : keyword_value(cast_string(*args.begin, 1)->impl());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_list(Lisper* lisp, VSlice args) {
  // (list 1 2 3)
  return list_value(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_macroQ(Lisper* lisp, VSlice args) {
  // (macro? x)
  preEqual(1, args.size(), "macro?");
  return bool_value(cast_macro(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_atomQ(Lisper* lisp, VSlice args) {
  // (atom? a)
  preEqual(1, args.size(), "atom?");
  return bool_value(cast_atom(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_map(Lisper* lisp, VSlice args) {
  // (map f [1 2 3])
  preEqual(2, args.size(), "map");
  auto pp = cast_function(*args.begin, 1);
  auto e= *(args.begin+1);
  if (cast_nil(e)) {
    return empty_list();
  }
  auto s= cast_seqable(e, 1);
  if (s->isEmpty()) {
    return empty_list();
  }
  VVec out;
  for (auto i=0, e=s->count(); i < e; ++i) {
    VVec p { s->nth(i) };
    s__conj(out, pp->invoke(lisp, VSlice(p)));
  }
  return list_value(VSlice(out));
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
  if (cast_nil(*args.begin)) { return nil_value(); }
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
  return string_value(print(args, true, " "));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_println(Lisper* lisp, VSlice args) {
  // (println "a" 1 "b")
  ::printf("%s\n", print(args,false, " ").c_str());
  return nil_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_prn(Lisper* lisp, VSlice args) {
  // (prn "a" 1 "b")
  ::printf("%s\n", print(args,true, " ").c_str());
  return nil_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_read_string(Lisper* lisp, VSlice args) {
  // (read-string "(+ 1 2)")
  preEqual(1, args.size(), "read-string");
  auto s= cast_string(*args.begin, 1)->impl();
  auto ret= SExprParser(s.c_str()).parse();
  ::printf("ret count = %d\n", ret.first);
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
  if (cast_nil(*args.begin)) { return empty_list(); }
  auto s = cast_seqable(*args.begin,1);
  VVec out;
  appendAll(s,1,out);
  return list_value(VSlice(out));
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
static d::DslValue native_slurp(Lisper* lisp, VSlice args) {
  // (slurp "some file")
  preEqual(1, args.size(), "slurp");
  return string_value(a::read_file(cast_string(*args.begin, 1)->impl().c_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_str(Lisper* lisp, VSlice args) {
  // (str 1 2 3)
  return string_value(print(args, false, ""));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_swapBang(Lisper* lisp, VSlice args) {
  // (swap! a f) (swap! a f 1 2 3)
  auto len = preMin(2, args.size(), "swap!");
  auto a= cast_atom(*args.begin, 1);
  auto op= cast_function(*(args.begin+1), 1);
  d::DslValue r;
  return a->reset((len > 2)
                  ? op->invoke(lisp, VSlice(args.begin+2, args.end))
                  : op->invoke(lisp));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_symbol(Lisper* lisp, VSlice args) {
  // (symbol "s")
  preEqual(1, args.size(), "symbol");
  return symbol_value(cast_string(*args.begin, 1)->impl());
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
  return int_value(cc::duration_cast<cc::milliseconds>(
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
  return vector_value(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_with_meta(Lisper* lisp, VSlice args) {
  // (with-meta obj m)
  preEqual(2, args.size(), "with-meta");
  return s__cast(LValue, (*args.begin).ptr())->withMeta(*(args.begin+1));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::Frame* init_natives(d::Frame* env) {
  env->set("*", fn_value("*", &native_mul), true);
  env->set("+", fn_value("+",&native_add), true);
  env->set("-", fn_value("-",&native_sub), true);
  env->set("/", fn_value("/",&native_div), true);
  env->set("<=", fn_value("<=",&native_lteq), true);
  env->set(">=", fn_value("<=",&native_gteq), true);
  env->set("==", fn_value("==",&native_eqeq),true);
  env->set("<", fn_value("<",&native_lt),true);
  env->set(">", fn_value(">",&native_gt),true);
  env->set("=", fn_value("=",&native_eq),true);
  env->set("apply", fn_value("apply",&native_apply),true);
  env->set("assoc", fn_value("assoc",&native_assoc),true);
  env->set("dissoc", fn_value("dissoc",&native_dissoc),true);
  env->set("atom", fn_value("atom",&native_atom),true);
  env->set("concat", fn_value("concat",&native_concat),true);
  env->set("conj", fn_value("conj",&native_conj),true);
  env->set("contains", fn_value("contains",&native_containsQ),true);
  env->set("count", fn_value("count",&native_count),true);
  env->set("deref", fn_value("deref",&native_deref),true);
  env->set("empty?", fn_value("empty?",&native_emptyQ),true);
  env->set("eval", fn_value("eval",&native_eval),true);
  env->set("first", fn_value("first",&native_first),true);
  env->set("fn?", fn_value("fn?",&native_fnQ),true);
  env->set("cons", fn_value("cons",&native_cons),true);
  env->set("get", fn_value("get",&native_get),true);
  env->set("hash-map",fn_value("hash-map",&native_hash_map),true);
  env->set("keys", fn_value("keys",&native_keys),true);
  env->set("keyword", fn_value("keyword",&native_keyword),true);
  env->set("list", fn_value("list",&native_list),true);
  env->set("macro?", fn_value("macro?",&native_macroQ),true);
  env->set("atom?", fn_value("atom?",&native_atomQ),true);
  env->set("map", fn_value("map",&native_map),true);
  env->set("meta", fn_value("meta",&native_meta),true);
  env->set("nth", fn_value("nth",&native_nth),true);
  env->set("pr-str", fn_value("pr-str",&native_pr_str),true);
  env->set("println", fn_value("println",&native_println),true);
  env->set("prn", fn_value("prn",&native_prn),true);
  env->set("read-string", fn_value("read-string",&native_read_string),true);
  env->set("reset!", fn_value("reset!",&native_resetBang),true);
  env->set("rest", fn_value("rest",&native_rest),true);
  env->set("seq", fn_value("seq",&native_seq),true);
  env->set("slurp", fn_value("slurp",&native_slurp),true);
  env->set("str", fn_value("str",&native_str),true);
  env->set("swap!", fn_value("swap!",&native_swapBang),true);
  env->set("symbol", fn_value("symbol",&native_symbol),true);
  env->set("throw", fn_value("throw",&native_throw),true);
  env->set("time-ms", fn_value("time-ms",&native_time_ms),true);
  env->set("vals", fn_value("vals",&native_vals),true);
  env->set("vector", fn_value("vector",&native_vector),true);
  env->set("with-meta", fn_value("with-meta",&native_with_meta),true);
  return env;
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


