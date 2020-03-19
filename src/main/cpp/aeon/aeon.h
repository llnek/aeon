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
struct Exception {
  const stdstr what() const { return msg; }
  protected:
  Exception(const stdstr& s) { msg=s; }
  stdstr msg;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FileNotFound : public Exception {
  FileNotFound(const stdstr&);
};
struct FileError : public Exception {
  FileError(const stdstr&);
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
StrVec tokenize(const stdstr& src, Tchar delim);
Tchar unescape_char(Tchar c);
stdstr escape_char(Tchar c);
stdstr to_upper(const stdstr&);
stdstr to_lower(const stdstr&);
bool fuzzy_equals(double d1, double d2);
int modulo(int, int);
template <typename T> bool is_even(T x) {
  return (x % 2) == 0;
}
void rand_seed();
stdstr read_file(const char* filepath);


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/*
template <typename T>
size_t slice_size(typename std::vector<T>::iterator b, typename std::vector<T>::iterator e) {
  size_t i=0;
  while ((b + i) != e) {
    i++;
  }
  return i;
}
*/
template <typename T>
int slice_size(typename std::vector<T>::iterator b, typename std::vector<T>::iterator e) {
  return std::distance(b,e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
bool equals(const std::vector<T>& a, const std::vector<T>& b) {
  if (a.size() == b.size()) {
    auto i1=a.begin();
    auto i2=b.begin();
    for (; i1 != a.end(); ++i1, ++i2) {
      if (*i1 == *i2) {} else { return false; }
    }
  }
  return a.size() == b.size();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
std::vector<T> slice(std::vector<T>& src, int from, int end) {
  std::vector<T> out;
  out.insert(out.end(), src.begin()+from, src.begin() + end); return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
std::vector<T> slice(std::vector<T>& src, int from) {
  std::vector<T> out;
  out.insert(out.end(), src.begin()+from, src.end()); return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
std::vector<T> slice(std::vector<T>* src, int from, int end) {
  std::vector<T> out;
  out.insert(out.end(), src->begin()+from, src->begin() + end); return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
std::vector<T> slice(std::vector<T>* src, int from) {
  std::vector<T> out;
  out.insert(out.end(), src->begin()+from, src->end()); return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


