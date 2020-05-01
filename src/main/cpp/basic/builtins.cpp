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
#include <cmath>
#include <random>
#include "builtins.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static double PI= 3.141592653589793;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double deg_rad(double deg) {
  return (deg * 2 * PI) / 360;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double rad_deg(double rad) {
  return (360 * rad) / (2 * PI);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static double to_dbl(d::DValue arg) {
  return vcast<d::Number>(arg,DMARK_00)->getFloat();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_pi(d::IEvaluator*, d::VSlice args) {
  d::preEqual(0, args.size(), "pi");
  return NUMBER_VAL(PI);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_cos(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "cos");
  return NUMBER_VAL(::cos(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_sin(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "sin");
  return NUMBER_VAL(::sin(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_tan(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "tan");
  return NUMBER_VAL(::tan(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_acs(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "acs");
  return NUMBER_VAL(::acos(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_asn(d::IEvaluator* e, d::VSlice args) {
  d::preEqual(1, args.size(), "asn");
  return NUMBER_VAL(::asin(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_atn(d::IEvaluator* e, d::VSlice args) {
  d::preEqual(1, args.size(), "atn");
  return NUMBER_VAL(::atan(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_sinh(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "sinh");
  return NUMBER_VAL(::sinh(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_cosh(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "cosh");
  return NUMBER_VAL(::cosh(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_tanh(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "tanh");
  return NUMBER_VAL(::tanh(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_asinh(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "asinh");
  return NUMBER_VAL(::asinh(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_acosh(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "acosh");
  return NUMBER_VAL(::acosh(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_atanh(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "atanh");
  return NUMBER_VAL(::atanh(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_exp(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "exp");
  return NUMBER_VAL(::exp(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_log(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "log");
  return NUMBER_VAL(::log(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/*
static d::DValue native_ln(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "ln");
  return NUMBER_VAL(::log10(to_dbl(*args.begin)));
}
*/
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_abs(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "abs");
  return NUMBER_VAL(::abs(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_sqrt(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "sqr");
  return NUMBER_VAL(::sqrt(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_cbrt(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "cur");
  return NUMBER_VAL(::cbrt(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_sign(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "sgn");
  auto d = to_dbl(*args.begin);
  return NUMBER_VAL(d > 0 ? 1 : (d < 0 ? -1 : 0));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_int(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "int");
  auto d = ::floor(to_dbl(*args.begin));
  return NUMBER_VAL((int)d);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_round(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "round");
  return NUMBER_VAL(::round(to_dbl(*args.begin)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_frac(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "frac");
  auto d= to_dbl(*args.begin);
  auto i=0.0;
  return NUMBER_VAL(::modf(d, &i));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_fix(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "fix");
  auto d= to_dbl(*args.begin);
  double i;
 ::modf(d, &i);
  return NUMBER_VAL((int) i);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_rand(d::IEvaluator*, d::VSlice args) {
  //d::preEqual(0, args.size(), "rnd");
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0, 1);
  return NUMBER_VAL(dis(gen));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_chr(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "chr$");
  int v= vcast<d::Number>(*(args.begin),DMARK_00)->getInt();
  ASSERT(v>=0&&v<=255, "Bad arg value: %d.", v);
  stdstr s {(char)v};
  return STRING_VAL(s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_asc(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "asc");
  auto s= vcast<d::String>(*(args.begin),DMARK_00)->impl();
  ASSERT(s.size() > 0, "Bad string: %s.", C_STR(s));
  return NUMBER_VAL((int) s[0]);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_val(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "val");
  auto s= vcast<d::String>(*(args.begin),DMARK_00)->impl().c_str();
  if (::strchr(s, '.')) {
    return NUMBER_VAL(::atof(s));
  } else {
    return NUMBER_VAL(::atoi(s));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_right(d::IEvaluator*, d::VSlice args) {
  d::preEqual(2, args.size(), "right$");
  auto s= vcast<d::String>(*(args.begin),DMARK_00)->impl();
  auto z= s.size();
  auto w= vcast<d::Number>(*(args.begin+1),DMARK_00)->getInt();

  if (w <= 0) {
    return STRING_VAL(""); }

  if (w >= z) {
    return STRING_VAL(s); }

  Tchar buf[z+1];
  int cz= s.copy(buf,w, z-w);
  buf[cz]='\0';
  return STRING_VAL(buf);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_left(d::IEvaluator*, d::VSlice args) {
  d::preEqual(2, args.size(), "left$");
  auto s= vcast<d::String>(*(args.begin),DMARK_00)->impl();
  auto z= s.size();
  auto w= vcast<d::Number>(*(args.begin+1),DMARK_00)->getInt();

  if (w <= 0) {
    return STRING_VAL(""); }

  if (w >= z) {
    return STRING_VAL(s); }

  Tchar buf[z+1];
  int cz= s.copy(buf,w, 0);
  buf[cz]='\0';
  return STRING_VAL(buf);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_mid(d::IEvaluator*, d::VSlice args) {
  auto len=d::preMin(2, args.size(), "mid$");
  auto s= vcast<d::String>(*(args.begin),DMARK_00)->impl();
  auto z= s.size();
  auto w=z;
  auto pos= vcast<d::Number>(*(args.begin+1),DMARK_00)->getInt();
  if (pos >=0 && pos < z) {} else {
    return STRING_VAL("");
  }
  if (len > 2) {
    w= vcast<d::Number>(*(args.begin+2),DMARK_00)->getInt(); }
  ASSERT1(w >=0);
  Tchar buf[z+1];
  int cz= s.copy(buf,w,pos);
  buf[cz]='\0';
  return STRING_VAL(buf);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_len(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "len");
  auto s= vcast<d::String>(*(args.begin),DMARK_00)->impl();
  return NUMBER_VAL((int)s.size());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_str(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "str$");
  auto n= vcast<d::Number>(*(args.begin),DMARK_00);
  stdstr s;
  if (n->isInt())
    s= N_STR(n->getInt()); else s=N_STR(n->getFloat());
  return STRING_VAL(s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DValue native_spc(d::IEvaluator*, d::VSlice args) {
  d::preEqual(1, args.size(), "spc");
  auto n= vcast<d::Number>(*(args.begin),DMARK_00);
  stdstr s;
  if (n->isInt() && n->getInt() > 0) {
    s= stdstr((int)n->getInt(), ' ');
  }
  return STRING_VAL(s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define REG(env,fn,arg) env->set(fn, FN_VAL(fn, &arg))

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DFrame init_natives(d::DFrame env) {

  //trig funcs
  REG(env,"SIN", native_sin);
  REG(env, "COS", native_cos);
  REG(env, "TAN", native_tan);
  REG(env, "ASN", native_asn);
  REG(env, "ACS", native_acs);
  REG(env, "ATN", native_atn);
  REG(env, "PI", native_pi);
  REG(env, "HYPSIN", native_sinh);
  REG(env, "HYPCOS", native_cosh);
  REG(env, "HYPTAN", native_tanh);
  REG(env, "HYPASN", native_asinh);
  REG(env, "HYPACS", native_acosh);
  REG(env, "HYPATN", native_atanh);

  REG(env, "EXP", native_exp);
  //REG(env, "LN", native_ln);
  REG(env, "LOG", native_log);
  REG(env, "ABS", native_abs);
  REG(env, "INT", native_int);
  REG(env, "SQR", native_sqrt);
  REG(env, "CUR", native_cbrt);
  REG(env, "SGN", native_sign);

  REG(env, "ROUND", native_round);
  REG(env, "FRAC", native_frac);
  REG(env, "FIX", native_fix);

  REG(env, "RAN#", native_rand);
  REG(env, "RND", native_rand);

  // string funcs
  REG(env, "RIGHT$", native_right);
  REG(env, "LEFT$", native_left);
  REG(env, "CHR$", native_chr);
  REG(env, "STR$", native_str);
  REG(env, "MID$", native_mid);
  REG(env, "ASC", native_asc);
  REG(env, "VAL", native_val);
  REG(env, "LEN", native_len);
  REG(env, "SPC", native_spc);








  return env;
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


