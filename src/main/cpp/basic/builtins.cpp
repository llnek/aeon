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
#include "builtins.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define TO_FLOAT(x) x.isInt() ? (double) x.getInt() : x.getFloat()
#define TO_INT(x) x.isInt() ? x.getInt() : (llong) x.getFloat()
#define CHKSZ(x, e) ((x) >= 0 && (x) < (e))

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static double PI= 3.141592653589793;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static double deg_rad(double deg) {
  return (deg * 2 * PI) / 360;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double rad_deg(double rad) {
  return (360 * rad) / (2 * PI);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static double to_dbl(d::DslValue arg) {
  if (auto f = cast_float(arg,0); f) {
    return f->impl();
  }
  if (auto n = cast_int(arg,0); n) {
    return n->impl();
  }
  RAISE(d::BadArg, "Expected numeric, got %s", C_STR(arg->pr_str(1)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_cos(d::IEvaluator* e, d::VSlice args) {
  // cos(x)
  d::preEqual(1, args.size(), "cos");
  auto deg = to_dbl(*args.begin);
  auto r= deg_rad(deg);
  return FLOAT_VAL(::cos(r));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
static d::DslValue native_sin(d::IEvaluator* e, d::VSlice args) {
  // sin(x)
  d::preEqual(1, args.size(), "sin");
  auto deg = to_dbl(*args.begin);
  auto r= deg_rad(deg);
  return FLOAT_VAL(::sin(r));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslFrame init_natives(d::DslFrame env) {
  env->set("SIN", FN_VAL("SIN",&native_sin));
  env->set("COS", FN_VAL("COS",&native_cos));
  return env;
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


