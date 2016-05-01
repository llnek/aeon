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

#pragma once
//////////////////////////////////////////////////////////////////////////////
//
#include "GSL/gsl.h"
#include <functional>
#include <algorithm>
#include <queue>
#include <vector>
#include <array>
#include <map>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
//typedef std::string stdstr;
typedef std::string sstr;

//////////////////////////////////////////////////////////////////////////////
//
#if defined(WIN32) || defined(_WIN32)

#if defined(MS_STATIC)
#define MS_DLL
#else
#if defined(_USRDLL)
#define MS_DLL     __declspec(dllexport)
#else         /* use a DLL library */
#define MS_DLL    __declspec(dllimport)
#endif
#endif


#else
#define MS_DLL
#endif


//////////////////////////////////////////////////////////////////////////////
//
#if !defined(TChar)

#  if defined(_UNICODE)
#   define  TChar  wchar_t
#  else
#    define  TChar  char
#  endif

#endif


//////////////////////////////////////////////////////////////////////////////
// namespace stuff
#ifdef __cplusplus
#define NS_USING(nsp)  using namespace nsp;
#define NS_BEGIN(nsp)  namespace nsp {
#define NS_END         };
#define NS_ALIAS(x,y)  namespace x = y;
#define BEGIN_NS_UNAMED   namespace {
#define END_NS_UNAMED   };
#else
#define NS_USING(nsp)
#define NS_BEGIN(nsp)
#define NS_END
#define NS_ALIAS(x,y)
#define BEGIN_NS_UNAMED
#define END_NS_UNAMED
#endif


//////////////////////////////////////////////////////////////////////////////
// memory lifecycle stuff
#define mc_free_mem(mem)  do { if (mem) ::free(mem); mem = nullptr; } while (0)
#define mc_free_fp(fp)  do { if (fp) ::fclose(fp); fp = nullptr; } while (0)
#define mc_new5(T, p1, p2, p3,p4,p5) new(std::nothrow) T((p1), (p2), (p3),(p4),(p5))
#define mc_new4(T, p1, p2, p3,p4) new(std::nothrow) T((p1), (p2), (p3) , (p4))
#define mc_new3(T, p1, p2, p3) new(std::nothrow) T((p1), (p2), (p3))
#define mc_new2(T, p1, p2) new(std::nothrow) T((p1), (p2))
#define mc_new1(T, p1) new(std::nothrow) T((p1))
#define mc_new(T) new(std::nothrow) T()
#define mc_free_ptr(ptr)  mc_free_mem(ptr)
#define mc_free_str(pc)   mc_free_mem(pc)

#define mc_del_arr(arr)   do { delete[] arr; arr = nullptr; } while (0)
#define mc_del_ptr(ptr)   do { delete ptr; ptr = nullptr; } while (0)
#define mc_bool_str(b)    ((b) ? "true" : "false")
#define mc_bool_str_u(b)  ((b) ? "TRUE" : "FALSE")

//////////////////////////////////////////////////////////////////////////////
// c++ std collection iteration
#define F__POOP(x,c) for (auto x=c->begin(); x != c->end(); ++x)
#define F__LOOP(x,c) for (auto x=c.begin(); x != c.end(); ++x)
#define S__PAIR(T1,T2,v1,v2) std::pair<T1,T2>((v1),(v2))
#define S__COPY(f,t) s::copy(s::begin(f), s::end(f), s::begin(t))
#define S__ADD(c,n) c.push_back(n)
#define S__NIL(x) x = nullptr;
#define S__MIX(c) std::random_shuffle(c.begin(),c.end())

//////////////////////////////////////////////////////////////////////////////
// c++ casting
#define DCAST(type,expr) dynamic_cast<type>(expr)
#define SCAST(type,expr) static_cast<type>(expr)
#define PCAST(type,expr) ((type*)(expr))

//////////////////////////////////////////////////////////////////////////////
// pointer macros
#define N_NIL(p) (p) != nullptr
#define E_NIL(p) (p) == nullptr
#define P_NIL nullptr

//////////////////////////////////////////////////////////////////////////////
typedef std::function<void ()> VOIDFN;
// std collection aliases
#define s_vec std::vector
#define s_que std::queue
#define s_arr std::array
#define s_map std::map

// magic numbers
// |----a----|--b--|
// ++  (a+b)/a = a/b = GOLDEN_RATIO
#define GOLDEN_RATIO 1.6180339887

///////////////////////////////////////////////////////////////////////////////
//
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
#define __decl_arr(T,e,v) s_arr<T,(e)> v;
#define __decl_vec(T,v) s_vec<T> v;
#define __decl_map(K,V,m) s_map<K,V> m;
#define __decl_que(T,v) s_que<T> v;

// find length of array
#define SIZEOFX(var,type) (sizeof(var)/sizeof(type))

// handy macros
#define FTOS(n) s::to_string(n)
// halve this value
#define HTV(v) ((v) * 0.5)

//////////////////////////////////////////////////////////////////////////////
//
NS_BEGIN(fusii)

enum class Locale {
  ENGLISH = 0,
  CHINESE,
  FRENCH,
  ITALIAN,
  GERMAN,
  SPANISH,
  DUTCH,
  RUSSIAN,
  KOREAN,
  JAPANESE,
  HUNGARIAN,
  PORTUGUESE,
  ARABIC,
  NORWEGIAN,
  POLISH,
  TURKISH,
  UKRAINIAN,
  ROMANIAN,
  BULGARIAN
};

//////////////////////////////////////////////////////////////////////////////
//
s_vec<sstr> tokenize(const sstr &src, TChar delim);
int modulo(int, int);
void randSeed();





NS_END

#include "FArrayObj.h"
#include "FArrayPtr.h"
#include "SMPtr.h"
#include "FDList.h"
#include "FPool.h"
NS_ALIAS(f,fusii)
NS_ALIAS(s,std)
NS_USING(gsl)


