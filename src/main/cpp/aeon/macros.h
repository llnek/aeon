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
#if defined(WIN32) || defined(_WIN32)
#if defined(MS_STATIC)
#define MS_DLL
#else
#if defined(_USRDLL)
#define MS_DLL __declspec(dllexport)
#else /* use a DLL library */
#define MS_DLL __declspec(dllimport)
#endif
#endif
#else
#define MS_DLL
#endif

//////////////////////////////////////////////////////////////////////////////
#if !defined(TChar)
#if defined(_UNICODE)
#define TChar  wchar_t
#else
#define TChar  char
#endif
#endif

//////////////////////////////////////////////////////////////////////////////
typedef int64_t llong;

//////////////////////////////////////////////////////////////////////////////
#define ERROR_FILE    stderr
#define LOG_FILE    stdout
#define NO_OP  do {} while (0)
#define NO_LOG(...) NO_OP

//////////////////////////////////////////////////////////////////////////////
#define ERROR_TRACE   0
#define LOG_TRACE   0
#define DEBUG_TRACE   0

//////////////////////////////////////////////////////////////////////////////
#if DEBUG_TRACE
  #define DEBUG(...) ::fprintf(LOG_FILE, __VA_ARGS__)
#else
  #define DEBUG NO_LOG
#endif
//////////////////////////////////////////////////////////////////////////////
#if LOG_TRACE
  #define LOG(...) ::fprintf(LOG_FILE, __VA_ARGS__)
#else
  #define LOG NO_LOG
#endif
//////////////////////////////////////////////////////////////////////////////
#if ERROR_TRACE
  #define ERROR(...) ::fprintf(ERROR_FILE, __VA_ARGS__)
#else
  #define ERROR NO_LOG
#endif



//////////////////////////////////////////////////////////////////////////////
#define ____ASSERT(file, line, condition, ...) \
    if (!(condition)) { \
        ::printf("Assertion failed near %s(%d): ", file, line); \
        ::printf(__VA_ARGS__); \
        exit(1); \
    } else { }

#define ASSERT1(condition) \
    ____ASSERT(__FILE__, __LINE__, condition, "%s", "\n")

#define ASSERT(condition, ...) \
    ____ASSERT(__FILE__, __LINE__, condition, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////
#define RAISE(exp, fmt, ...) \
  do { char buf[1024]; \
  ::sprintf(buf, (const char*)fmt, __VA_ARGS__); \
    throw exp(buf);} while (0)


//////////////////////////////////////////////////////////////////////////////
// memory lifecycle stuff
#define FREE_FILE(x) do {if (x) ::fclose(x); x=nullptr;} while (0)
#define FREE_MEM(x) do {if (x) ::free(x); x=nullptr;} while (0)
#define DEL_ARRAY(a) do {delete[] a; a=nullptr;} while (0)
#define DEL_PTR(p) do {delete p; p=nullptr;} while (0)

// ctors
#define CTOR_5(T,p1,p2,p3,p4,p5) new(std::nothrow) T {p1,p2,p3,p4,p5}
#define CTOR_4(T,p1,p2,p3,p4) new(std::nothrow) T {p1,p2,p3,p4}
#define CTOR_3(T,p1,p2,p3) new(std::nothrow) T {p1,p2,p3}
#define CTOR_2(T,p1,p2) new(std::nothrow) T {p1,p2}
#define CTOR_1(T,p1) new(std::nothrow) T {p1}
#define CTOR_0(T) new(std::nothrow) T {}

// misc
#define BOOL_STR_U(b) ((b) ? "TRUE" : "FALSE")
#define BOOL_STR(b)  ((b) ? "true" : "false")

//////////////////////////////////////////////////////////////////////////////
// c++ std collection iteration
//#define F__POOP(x,c) for (auto x=c->begin(); x != c->end(); ++x)
//#define F__LOOP(x,c) for (auto x=c.begin(); x != c.end(); ++x)
#define s__ccat(x,y) ((x).insert((x).end(),(y).begin(),(y).end()))
#define s__pair(T1,T2,v1,v2) std::pair<T1,T2>(v1,v2)
#define s__conj(c,n) (c).push_back(n)
#define s__nil(x) (x = nullptr)
#define s__mix(c) std::random_shuffle(c.begin(),c.end())

//////////////////////////////////////////////////////////////////////////////
// c++ casting
#define d__cast(type,expr) dynamic_cast<type*>(expr)
#define s__cast(type,expr) static_cast<type*>(expr)
#define v__cast(type,expr) static_cast<type>(expr)

//#define PCAST(type,expr) ((type*)(expr))

//////////////////////////////////////////////////////////////////////////////
// c++ STL
#define s__contains(C,x) ((C).find((x)) != (C).end())
#define map__val(M,K) (M).find(K)->second
typedef std::string stdstr;
typedef std::vector<std::string> strvec;

//////////////////////////////////////////////////////////////////////////////
// pointer macros
#define X_NIL(p) (p) != nullptr
#define E_NIL(p) (p) == nullptr
#define P_NIL nullptr
#define S_NIL(x) (x = nullptr)

//////////////////////////////////////////////////////////////////////////////
// magic numbers
// |----a----|--b--|
// ++  (a+b)/a = a/b = GOLDEN_RATIO
#define GOLDEN_RATIO 1.6180339887

///////////////////////////////////////////////////////////////////////////////
// c++ short-cuts

///////////////////////////////////////////////////////////////////////////////
#define __declapi_gsms_is(v, funName) \
virtual bool is##funName(); \
virtual void set##funName(bool p);

#define __declapi_ismx(v, funName) \
virtual bool is##funName();

#define __decl_ismx(v, funName) \
virtual bool is##funName() { return v; }

#define __decl_gsms_is(v, funName) \
virtual bool is##funName() { return v; } \
virtual void set##funName(bool p) { v=p; }

/////////////////////////////////////////////////////////////////////////////
//
#define __declapi_gsms(varType, v, funName) \
virtual varType get##funName(); \
virtual void set##funName(varType p);

#define __decl_gsms(varType, v, funName) \
virtual varType get##funName() { return v; } \
virtual void set##funName(varType p) { v=p; }

#define __declapi_getr(varType, v, funName) \
virtual varType get##funName();

#define __decl_getr(varType, v, funName) \
virtual varType get##funName() { return v; }

#define __declapi_setr(varType, v, funName) \
virtual void set##funName(varType p);

#define __decl_setr(varType, v, funName) \
virtual void set##funName(varType p) { v=p; }

//////////////////////////////////////////////////////////////////////////////
// c++ constructor stuff
#define __decl_nocpyass(T) \
  T&operator =(const T&) = delete;  \
  T(const T&) = delete; \
  T(T&&) = delete;  \
  T&operator =(T&&) = delete;

#define __decl_nodft(T) T()=delete;

//////////////////////////////////////////////////////////////////////////
// member decl stuff
#define __decl_ptr(T,p) T *p=nullptr;
#define __decl_dz(d) double d=0;
#define __decl_fz(f) float f=0;
#define __decl_iz(i) int i=0;
#define __decl_bf(b) bool b=false;
#define __decl_bt(b) bool b=true;
#define __decl_md(T,m) T m;
#define __decl_mv(T,m,v) T m=v;
#define __decl_arr(T,e,v) std::array<T,(e)> v;
#define __decl_vec(T,v) std::vector<T> v;
#define __decl_map(K,V,m) std::map<K,V> m;
#define __decl_que(T,v) std::queue<T> v;

// find length of array
#define SIZEOFX(var,type) (sizeof(var)/sizeof(type))

// handy macros
#define FTOS(n) std::to_string(n)
// halve this value
#define HTV(v) ((v) * 0.5)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

