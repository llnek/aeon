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

#include "fusilli.h"
NS_BEGIN(fusii)

//////////////////////////////////////////////////////////////////////////////
// Dynamic array of primitive types and
// objects.
template<typename T>
class MS_DLL FArray {

  __decl_ptr(T,_data)
  __decl_iz(_sz)

public:

  FArray<T>& operator=(const FArray<T>&);
  FArray<T>& operator=(FArray<T>&&);

  FArray(const FArray<T>&);
  FArray(FArray<T>&&);

  FArray<T>* clone();

  void setFirst(const T &value);
  void setLast(const T &value);

  const T& getFirst();
  const T& getLast();

  void set(int pos, const T &value);
  int size() { return _sz; }

  bool notAny(const T &v);
  bool some(const T &v);
  bool all(const T &v);
  int find(const T &v);
  void fill(const T &v);

  const T& operator[](int pos);
  const T& get(int pos);
  T& getRef(int pos);

  explicit FArray(int z);
  FArray();

  virtual ~FArray();
};

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
FArray<T>& FArray<T>::operator=(FArray<T> &&src) {
  mc_del_arr(_data);
  _data=src._data;
  _sz=src._sz;
  S__NIL(src._data)
  src._sz=0;
  return *this;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
FArray<T>::FArray(FArray<T> &&src) {
  _data=src._data;
  _sz=src._sz;
  S__NIL(src._data)
  src._sz=0;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
FArray<T>& FArray<T>::operator=(const FArray<T> &src) {
  mc_del_arr(_data);
  _sz=src._sz;
  if (_sz > 0) {
    _data= new T[_sz];
    for (auto i=0; i < _sz; ++i) {
      _data[i] = src._data[i];
    }
  }
  return *this;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
FArray<T>::FArray(const FArray<T> &src) {
  S__NIL(_data)
  _sz=src._sz;
  if (_sz > 0) {
    _data= new T[_sz];
    for (auto i=0; i < _sz; ++i) {
      _data[i] = src._data[i];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
FArray<T>::FArray(int z) {
  _data = z > 0 ? new T[z] : nullptr;
  _sz=z;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
FArray<T>::FArray() {
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
FArray<T>::~FArray() {
  mc_del_arr(_data);
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
int FArray<T>::find(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v == _data[i]) { return i; }
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
void FArray<T>::fill(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    _data[i]=v;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
bool FArray<T>::some(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v == _data[i]) { return true; }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
bool FArray<T>::notAny(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v == _data[i]) { return false; }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
bool FArray<T>::all(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v != _data[i]) { return false; }
  }
  return _sz > 0 ;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
FArray<T>* FArray<T>::clone() {
  auto rc= new FArray<T>(this->_sz);
  for (auto i=0; i < this->_sz; ++i) {
    rc->_data[i] = this->_data[i];
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
void FArray<T>::set(int pos, const T &v) {
  assert(pos >= 0 && pos < _sz);
  _data[pos] = v;
}

//////////////////////////////////////////////////////////////////////////
//
template<typename T>
const T& FArray<T>::getFirst() {
  assert(_sz > 0);
  return _data[0];
}

//////////////////////////////////////////////////////////////////////////
//
template<typename T>
const T& FArray<T>::getLast() {
  assert(_sz > 0);
  return _data[_sz-1];
}

//////////////////////////////////////////////////////////////////////////
//
template<typename T>
void FArray<T>::setFirst(const T &v) {
  assert(_sz > 0);
  _data[0]= v;
}

//////////////////////////////////////////////////////////////////////////
//
template<typename T>
void FArray<T>::setLast(const T &v) {
  assert(_sz > 0);
  _data[_sz-1]=v;
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
T& FArray<T>::getRef(int pos) {
  assert(pos >= 0 && pos < _sz);
  return _data[pos];
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
const T& FArray<T>::get(int pos) {
  assert(pos >= 0 && pos < _sz);
  return _data[pos];
}

//////////////////////////////////////////////////////////////////////////////
//
template<typename T>
const T& FArray<T>::operator[](int pos) {
  assert(pos >= 0 && pos < _sz);
  return _data[pos];
}


//////////////////////////////////////////////////////////////////////////
//
typedef FArray<float> FArrFloat;
typedef FArray<int> FArrInt;
typedef FArray<bool> FArrBool;


NS_END


