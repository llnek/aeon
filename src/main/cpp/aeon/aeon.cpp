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

#include "aeon.h"
#include <math.h>
#include <sstream>

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon {
char MSGBUF[1024];

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
FileNotFound::FileNotFound(const stdstr& s) : Exception(s) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
FileError::FileError(const stdstr& s) : Exception(s) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CString::CString(size_t z) {
  s= (char*) ::malloc((z+1) * sizeof(char));
  s[z]='\0';
  this->z=z;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CString::CString() { z=0; S_NIL(s); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CString::~CString() { ::free(s); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
const char* CString::get() { return s; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void CString::copy(const char* src) {
  auto n = src ? ::strlen(src) : 0;

  if (n > z) {
    // grow buffer
    s = (char*) ::realloc(s, (n+1) * sizeof(char));
    ::strcpy(s, src);
    z=n;
  }
  else if (src) {
    ::strcpy(s, src);
  }
  else if (s) {
    s[0]='\0';
  }
}

//////////////////////////////////////////////////////////////////////////////
strvec tokenize(const stdstr& src, TChar delim) {
  std::stringstream ss(src);
  strvec out;
  stdstr tkn;
  while (std::getline(ss, tkn, delim)) {
    if (tkn.length() > 0) {
      s__conj(out, tkn);
    }
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr toupper(const stdstr& s) {
  stdstr t(s);
  for (auto& c : t) { c= ::toupper(c); }
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr tolower(const stdstr& s) {
  stdstr t(s);
  for (auto& c : t) { c= ::tolower(c); }
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void randSeed(long x=0) {
  std::srand (x > 0 ? x : std::time(nullptr));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//int mod(int x, int m) { return (x%m + m)%m; }
int modulo(int x, int m) {
  int r = x % m;
  return r < 0 ? r+m : r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool dbl_equals(double d1, double d2) {
  auto d = d1-d2;
  return ::fabs(d) < 0.000000000001;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr readFile(const char* fpath) {
  auto fp = ::fopen(fpath, "rb");

  if (! fp) {
    raise(FileNotFound, "Failed to open file: %s", fpath);
  }

  auto len = (::fseek(fp, 0L, SEEK_END), ::ftell(fp));
  auto buf = (char*) ::malloc((len+1) * sizeof(char));
  auto cnt = (::rewind(fp), ::fread(buf, sizeof(char), len, fp));

  ::fclose(fp);

  if (cnt != len) {
    ::free(buf);
    raise(FileError,"Failed to read file: %s", fpath);
  } else {
    buf[cnt]='\0';
  }

  auto s= stdstr(buf);
  ::free(buf);
  return s;
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


