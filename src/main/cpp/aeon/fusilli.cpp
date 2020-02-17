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

#include "fusilli.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
namespace fusii {

  std::vector<std::string> tokenize(const std::string &src, TChar delim) {
    std::vector<std::string> out;
    std::stringstream ss(src);
    std::string tkn;
    while (std::getline(ss, tkn, delim)) {
      if (tkn.length() > 0) {
        s__conj(out, tkn);
      }
    }
    return out;
  }

  void randSeed(long x=0) {
    std::srand ( x > 0 ? x : std::time(nullptr));
  }

  int modulo(int x, int m) {
    int r = x % m;
    return r < 0 ? r+m : r;
  }

  /*
  int mod(int x, int m) {
      return (x%m + m)%m;
  }
  */

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


