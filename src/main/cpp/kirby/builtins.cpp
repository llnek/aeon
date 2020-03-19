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
StrVec BUILT_INS {
  R"((defmacro cond
       [& xs]
       (if (> (count xs) 0)
         (list 'if
               (first xs)
               (if (> (count xs) 1)
                 (nth xs 1)
                 (throw "odd number of forms to cond"))
               (cons 'cond (rest (rest xs)))))))",
  R"((def not
       (fn [cond] (if cond false true))))",
  R"((def load-file
       (fn [filename]
         (eval (read-string (str "(do "
                                 (slurp filename) "\nnil)))))))",
  R"((def *host-language* "C++"))"
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame root_env();
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFloat* op_num(double n) { return new LFloat(n); }
LInt* op_num(llong n) { return new LInt(n); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
d::DslValue op_math(int op, T res, NumberSlots& slots) {
  for (auto& s :slots) {
    switch (op) {
      case d::T_MULT:
        if (s.isInt()) { res *= s.u.n; } else { res *= s.u.r; }
      break;
      case d::T_DIV:
        if (s.isInt()) {
          ASSERT1(s.u.n != 0);
          res /= s.u.n;
        } else {
          ASSERT1(s.u.r != 0);
          res /= s.u.r;
        }
      break;
      case d::T_PLUS:
        if (s.isInt()) { res += s.u.n; } else { res += s.u.r; }
      break;
      case d::T_MINUS:
        if (s.isInt()) { res -= s.u.n; } else { res -= s.u.r; }
      break;
    }
  }
  return d::DslValue(op_num(res));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_mul(VSlice args) {
  NumberSlots slots;
  auto r= cast_numeric(args,slots);
  return r
    ? op_math<double>(d::T_MULT, 1.0, slots)
    : op_math<llong>(d::T_MULT, 1, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_div(VSlice args) {
  if (args.begin == args.end)
    RAISE(BadArity, "Expecting args, got 0.%s", "\n");
  NumberSlots slots;
  auto r= cast_numeric(args,slots);
  return r
    ? op_math<double>(d::T_DIV, 1.0, slots)
    : op_math<llong>(d::T_DIV, 1, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_add(VSlice args) {
  NumberSlots slots;
  auto r= cast_numeric(args,slots);
  return r
    ? op_math<double>(d::T_PLUS, 0.0, slots)
    : op_math<llong>(d::T_PLUS, 0, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_sub(VSlice args) {
  if (args.begin==args.end)
    RAISE(BadArity, "Expecting args, got 0.%s", "\n");
  NumberSlots slots;
  auto r= cast_numeric(args,slots);
  return r
    ? op_math<double>(d::T_MINUS, 0.0, slots)
    : op_math<llong>(d::T_MINUS, 0, slots);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_lteq(VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (<= 1 2 3 4 4 5)
  if (args.begin==args.end)
    RAISE(BadArity, "Expecting args, got 0.%s", "\n");
  NumberSlots slots; cast_numeric(args,slots);
  auto a= slots[0];
  auto lhs = TO_FLOAT(a);
  for (auto i=1; i < slots.size(); ++i) {
    a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs <= rhs) {
      lhs=rhs;
    } else { return false_value(); }
  }
  return true_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_gteq(VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (>= 5 5 4 3 2 1)
  if (args.begin==args.end)
    RAISE(BadArity, "Expecting args, got 0.%s", "\n");
  NumberSlots slots; cast_numeric(args,slots);
  auto a= slots[0];
  auto lhs = TO_FLOAT(a);
  for (auto i=1; i < slots.size(); ++i) {
    a=slots[i];
    if (auto rhs = TO_FLOAT(a); lhs >= rhs) {
      lhs=rhs;
    } else { return false_value(); }
  }
  return true_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_lt(VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (< 1 2 3)
  if (args.begin==args.end)
    RAISE(BadArity, "Expecting args, got 0.%s", "\n");
  NumberSlots slots; cast_numeric(args,slots);
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
static d::DslValue native_gt(VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (> 3 2 1)
  if (args.begin==args.end)
    RAISE(BadArity, "Expecting args, got 0.%s", "\n");
  NumberSlots slots; cast_numeric(args,slots);
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
static d::DslValue native_eqeq(VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (== 3 3.0)
  if (args.begin==args.end)
    RAISE(BadArity, "Expecting args, got 0.%s", "\n");
  NumberSlots slots; cast_numeric(args,slots);
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
static d::DslValue native_eq(VSlice args) {
  //for this, always use real numbers, simpler logic
  // e.g. (= 3 3.0) => false (= 3 3) => true
   if (args.begin==args.end)
    RAISE(BadArity, "Expecting args, got 0.%s", "\n");
  auto lhs = *args.begin;
  args.begin++;
  for (auto i=args.begin; i != args.end; ++i) {
    if (auto rhs= *i; lhs->equals(rhs.ptr())) {
      lhs=rhs;
    } else { false_value(); }
  }
  return true_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_apply(VSlice args) {
  int len= a::slice_size<d::DslValue>(args.begin, args.end);
  if (len < 2)
    RAISE(BadArity, "Expecting > 2 args, got %d.\n", len);
  auto op = cast_function(*args.begin);
  auto last= args.begin + (len - 1);
  auto s= cast_seq(*last);
  if (E_NIL(s))
    expected("Sequence", *last);
  VVec pms;
  for (auto i=args.begin+1; i != last; ++i) {
    s__conj(pms, *i);
  }
  for (auto i=0, e= s->count(); i < e; ++i) {
    s__conj(pms, s->nth(i));
  }
  return op->invoke(VSlice(pms.begin(), pms.end()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_assoc(VSlice args) {
  preMin(4, args.size(), "assoc");
  return cast_map(*args.begin, true)->assoc(VSlice(args.begin+1,args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_atom(VSlice args) {
  preEqual(1, args.size(), "atom");
  return d::DslValue(new LAtom(*args.begin));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_concat(VSlice args) {
  auto len= args.size();
  if (len == 0 ||
      (len == 1 && cast_nil(*args.begin))) {
    return empty_list();
  }
  VVec out;
  for (auto i = args.begin; i != args.end; ++i) {
    if (cast_nil(*i)) { continue; }
    auto s= cast_seq(*i, true);
    for (auto i=0, e= s->count(); i < e; ++i) {
      s__conj(out, s->nth(i));
    }
  }
  return list_value(VSlice(out.begin(), out.end()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_conj(VSlice args) {
  preMin(2, args.size(), "conj");
  auto s= cast_seq(*args.begin, 1);
  return s->conj(VSlice(args.begin+1,args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_cons(VSlice args) {
  preEqual(2, args.size(), "cons");
  auto s = cast_seq(*(args.begin+1), 1);
  VVec out { *args.begin };
  for (auto i=0, e=s->count(); i < e; ++i) {
    s__conj(out, s->nth(i));
  }
  return list_value(VSlice(out.begin(),out.end()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_containsQ(VSlice args) {
  preEqual(2, args.size(), "contains?");
  return X_NIL(cast_nil(*args.begin))
    ? *args.begin
    : bool_value(cast_seqable(*args.begin,1)->contains(*(args.begin+1)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_count(VSlice args) {
  preEqual(1, args.size(), "count");
  return X_NIL(cast_nil(*args.begin))
    ? int_value(0)
    : int_value(cast_seqable(*args.begin,1)->count());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_deref(VSlice args) {
  preEqual(1, args.size(), "deref");
  return cast_atom(*args.begin,1)->deref();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_dissoc(VSlice args) {
  preMin(1, args.size(), "dissoc");
  auto m = cast_map(*args.begin, true);
  if (args.size() == 1) { return *args.begin; }
  return m->dissoc(VSlice(args.begin+1, args.end));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_emptyQ(VSlice args) {
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
static d::DslValue native_eval(VSlice args) {
  preEqual(1, args.size(), "eval");
  Lisper p;
  return p.EVAL(*args.begin, root_env());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_first(VSlice args) {
  preEqual(1, args.size(), "first");
  if (cast_nil(*args.begin)) { return nil_value(); }
  if (auto s=cast_string(*args.begin); X_NIL(s)) {
    return s->first();
  }
  if (auto s=cast_seq(*args.begin); X_NIL(s)) {
    return s->first();
  }
  if (auto m=cast_map(*args.begin); X_NIL(m)) {
    return m->first();
  }
  return expected("Seq'able", *args.begin);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_fnQ(VSlice args) {
  preEqual(1, args.size(), "fn?");
  if (cast_lambda(*args.begin)) {
    return true_value();
  }
  else
  if (cast_native(*args.begin)) {
    return true_value();
  }
  else {
    return false_value();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_get(VSlice args) {
  preMin(2, args.size(), "get");
  auto m= cast_map(*args.begin, 1);
  auto s= s__cast(LSeq, m);
  auto k= *(args.begin+1);
  if (s->contains(k)) { return m->get(k); }
  // not found provided
  return args.size() > 2 ? *(args.begin+2) : nil_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_hash_map(VSlice args) {
  preEven(args.size(), "hash-map");
  LHash m;
  return m.assoc(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_keys(VSlice args) {
  preEqual(1, args.size(), "keys");
  return cast_map(*args.begin, true)->keys();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_keyword(VSlice args) {
  preEqual(1, args.size(), "keyword");
  if (cast_nil(*args.begin)) {
    return *args.begin;
  }
  auto s= cast_string(*args.begin, 1);
  return keyword_value(s->impl());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_list(VSlice args) {
  return d::DslValue(new LList(args));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_macroQ(VSlice args) {
  preEqual(1, args.size(), "macro?");
  return bool_value(cast_macro(*args.begin) != nullptr);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_map(VSlice args) {
  preEqual(2, args.size(), "map");
  auto pp = cast_function(*args.begin, 1);
  auto e= *(args.begin+1);
  if (cast_nil(e)) { return empty_list(); }
  auto s= cast_seqable(e, 1);
  if (s->isEmpty()) { return empty_list(); }
  VVec out;
  for (auto i=0, e=s->count(); i < e; ++i) {
    VVec p { s->nth(i) };
    s__conj(out, pp->invoke(VSlice(p.begin(), p.end())));
  }
  return list_value(VSlice(out.begin(), out.end()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_meta(VSlice args) {
  preEqual(1, args.size(), "meta");
  auto x = *args.begin;
  return s__cast(LValue,x.ptr())->meta();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_nth(VSlice args) {
  auto len= args.size();
  preMin(2, len, "nth");
  if (cast_nil(*args.begin))
  { return nil_value(); }
  auto pos= cast_int(*(args.begin+1), 1)->impl();
  auto s= cast_seqable(*args.begin,1);
  return (!CHKSZ(pos,s->count()) && len > 2) ? *(args.begin+2) : s->nth(pos);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static stdstr print(VSlice args, bool pretty, const stdstr& sep) {
  stdstr out;
  for (auto i=args.begin; i != args.end; ++i) {
    if (!out.empty()) { out += sep; }
    out += s__cast(LValue,(*i).ptr())->toString(pretty);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_pr_str(VSlice args) {
  return string_value(print(args, true, " "));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_println(VSlice args) {
  ::printf("%s\n", print(args,false, " ").c_str());
  return nil_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_prn(VSlice args) {
  ::printf("%s\n", print(args,true, " ").c_str());
  return nil_value();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_read_string(VSlice args) {
  preEqual(1, args.size(), "read-string");
  auto s= cast_string(*args.begin, true);
  return SExprParser(s->impl().c_str()).parse();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_resetBang(VSlice args) {
  preEqual(2, args.size(), "reset!");
  return cast_atom(*args.begin, true)->reset(*(args.begin+1));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_rest(VSlice args) {
  preEqual(1, args.size(), "rest");
  if (cast_nil(*args.begin)) { return empty_list(); }
  auto s = cast_seqable(*args.begin,1);
  VVec out;
  for (auto i = 1, e=s->count(); i < e; ++i) {
    s__conj(out, s->nth(i));
  }
  return list_value(VSlice(out.begin(), out.end()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_seq(VSlice args) {
  preEqual(1, args.size(), "seq");
  if (cast_nil(*args.begin))
  { return *args.begin; }
  else
  if (auto m= cast_map(*args.begin); X_NIL(m)) {
    return m->seq();
  }
  else
  if (auto s= cast_seq(*args.begin); X_NIL(s)) {
    return s->seq();
  }
  else
  if (auto s= cast_string(*args.begin); X_NIL(s)) {
    return s->seq();
  }
  else {
    return expected("Seq'able", *args.begin);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_slurp(VSlice args) {
  preEqual(1, args.size(), "slurp");
  return string_value(a::read_file(cast_string(*args.begin, 1)->impl().c_str()));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_str(VSlice args) {
  return string_value(print(args, false, ""));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_swapBang(VSlice args) {
  auto len = args.size(); preMin(2, len, "swap!");
  auto a= cast_atom(*args.begin, 1);
  auto op= cast_function(*(args.begin+1), 1);
  d::DslValue r;
  if (len > 2) {
    r= op->invoke(VSlice(args.begin+2, args.end));
  } else {
    r= op->invoke();
  }
  return a->reset(r);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_symbol(VSlice args) {
  preEqual(1, args.size(), "symbol");
  return symbol_value(cast_string(*args.begin, 1)->impl());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_throw(VSlice args) {
  preEqual(1, args.size(), "throw");
  throw *args.begin;
}
namespace cc= std::chrono;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_time_ms(VSlice args) {
  auto ms = cc::duration_cast<cc::milliseconds>(
    cc::high_resolution_clock::now().time_since_epoch()
  );
  return int_value(ms.count());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_vals(VSlice args) {
  preEqual(1, args.size(), "vals");
  if (cast_nil(*args.begin)) { return *args.begin; }
  return cast_map(*args.begin, 1)->vals();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_vector(VSlice args) {
  return vector_value(args);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_with_meta(VSlice args) {
  preEqual(2, args.size(), "with-meta");
  auto a= *args.begin;
  auto m= *(args.begin+1);
  return s__cast(LValue, a.ptr())->withMeta(m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::Frame* init_natives(d::Frame* env) {
  env->set("*", fn_value("*",&native_mul), true);
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
d::DslFrame root_env() {
  auto r = new d::Frame("root");
  init_natives(r);
  return d::DslFrame(r);
}





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


