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
// memory lifecycle stuff
#define free_file(x) do {if (x) ::fclose(x); x=nullptr;} while (0)
#define free_mem(x) do {if (x) ::free(x); x=nullptr;} while (0)
#define del_array(a) do {delete[] a; a=nullptr;} while (0)
#define del_ptr(p) do {delete p; p=nullptr;} while (0)

// ctors
#define ctor5(T,p1,p2,p3,p4,p5) new(std::nothrow) T {p1,p2,p3,p4,p5}
#define ctor4(T,p1,p2,p3,p4) new(std::nothrow) T {p1,p2,p3,p4}
#define ctor3(T,p1,p2,p3) new(std::nothrow) T {p1,p2,p3}
#define ctor2(T,p1,p2) new(std::nothrow) T {p1,p2}
#define ctor1(T,p1) new(std::nothrow) T {p1}
#define ctor(T) new(std::nothrow) T {}

// misc
#define bool_str_u(b) ((b) ? "TRUE" : "FALSE")
#define bool_str(b)  ((b) ? "true" : "false")

//////////////////////////////////////////////////////////////////////////////
// c++ std collection iteration
//#define F__POOP(x,c) for (auto x=c->begin(); x != c->end(); ++x)
//#define F__LOOP(x,c) for (auto x=c.begin(); x != c.end(); ++x)
#define s__pair(T1,T2,v1,v2) std::pair<T1,T2>(v1,v2)
#define s__conj(c,n) c.push_back(n)
#define s__nil(x) (x = nullptr)
#define s__mix(c) std::random_shuffle(c.begin(),c.end())

//////////////////////////////////////////////////////////////////////////////
// c++ casting
#define DCAST(type,expr) dynamic_cast<type>(expr)
#define SCAST(type,expr) static_cast<type>(expr)
#define PCAST(type,expr) ((type*)(expr))

//////////////////////////////////////////////////////////////////////////////
// pointer macros
#define X_NIL(p) (p) != nullptr
#define E_NIL(p) (p) == nullptr
#define P_NIL nullptr

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
#define FTOS(n) s::to_string(n)
// halve this value
#define HTV(v) ((v) * 0.5)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

