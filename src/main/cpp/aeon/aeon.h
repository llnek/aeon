#pragma once
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

//////////////////////////////////////////////////////////////////////////////
#include <functional>
#include <algorithm>
#include <queue>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "macros.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon {

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FileNotFound : public std::logic_error {
  explicit FileNotFound(const std::string&);
  explicit FileNotFound(const char* s);
};
struct FileError : public std::logic_error {
  explicit FileError(const std::string&);
  explicit FileError(const char* s);
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct CString {

  void copy(const char* src);
  const char* get();

  CString(size_t z);
  CString();
  ~CString();

  private:

  size_t z;
  char* s;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<std::string> tokenize(const std::string &src, TChar delim);
std::string toupper(const std::string&);
std::string tolower(const std::string&);
int modulo(int, int);
template <typename T> bool isEven(T x) {
  return (x % 2) == 0;
}
void randSeed();
std::string readFile(const char* filepath);
}

#include "smptr.h"
#include "dlist.h"
#include "pool.h"
#include "array.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


