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
 * Copyright (c) 2013-2016, Kenneth Leung. All rights reserved. */

#include "fusilli.h"
#include <sstream>
NS_BEGIN(fusii)

//////////////////////////////////////////////////////////////////////////////
//
s_vec<sstr> tokenize(const sstr &src, TChar delim) {
  s::stringstream ss(src);
  s_vec<sstr> out;
  sstr tkn;
  while (s::getline(ss, tkn, delim)) {
    if (tkn.length() > 0) {
      out.push_back(tkn);
    }
  }
  return out;
}

//////////////////////////////////////////////////////////////////////////
//
void randSeed() {
  s::srand (s::time(nullptr));
}

//////////////////////////////////////////////////////////////////////////
//
int modulo(int x, int m) {
  int r = x%m;
  return r<0 ? r+m : r;
}
/*
int mod(int x, int m) {
    return (x%m + m)%m;
}
*/


NS_END



