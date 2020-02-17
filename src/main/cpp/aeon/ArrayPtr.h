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
#include "fusilli.h"
namespace
fusii
{
// Dynamic array of pointers to a type.
template<typename T>
class MS_DLL ArrayPtr {

  T **_data=nullptr;
  int _sz=0;

public:

  ArrayPtr<T>& operator=(const ArrayPtr<T>&);
  ArrayPtr<T>& operator=(ArrayPtr<T>&&);

  ArrayPtr(const ArrayPtr<T>&);
  ArrayPtr(ArrayPtr<T>&&);

  explicit ArrayPtr(int z); ArrayPtr();
  ArrayPtr() = delete;
  virtual ~ArrayPtr();

  ArrayPtr<T>* clone();

  T* operator[](int pos);
  T* get(int pos);
  T* first();
  T* last();

  void first(T *value);
  void last(T *value);

  void set(int pos, T *value);
  int size() { return _sz; }
  T* swap(int, T*);

  bool notAny(T *v);
  bool some(T *v);
  bool every(T *v);
  int find(T *v);
  void fill(T *v);

  void map(std::function<T* (T*)>);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
ArrayPtr<T>& ArrayPtr<T>::operator=(ArrayPtr<T> &&src) {
  del_arr(_data);
  _data=src._data;
  _sz=src._sz;
  s__nil(src._data);
  src._sz=0;
  return *this;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayPtr<T>::ArrayPtr(ArrayPtr<T> &&src) {
  _data=src._data;
  _sz=src._sz;
  s__nil(src._data);
  src._sz=0;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayPtr<T>& ArrayPtr<T>::operator=(const ArrayPtr<T> &src) {
  del_arr(_data);
  _sz=src._sz;
  if (_sz > 0) {
    _data= new T* [_sz];
    for (auto i=0; i < _sz; ++i) {
      _data[i] = src._data[i];
    }
  }
  return *this;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayPtr<T>::ArrayPtr(const ArrayPtr<T> &src) {
  _sz=src._sz;
  s__nil(_data)
  if (_sz > 0) {
    _data= new T* [_sz];
    for (auto i=0; i < _sz; ++i) {
      _data[i] = src._data[i];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayPtr<T>::ArrayPtr(int z) {
  _data = z > 0 ? new T* [z] : nullptr;
  _sz=z;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayPtr<T>::~ArrayPtr() {
  del_arr(_data);
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
T* ArrayPtr<T>::swap(int pos, T *np) {
  assert(_sz > 0);
  assert(pos >= 0 && pos < _sz);
  auto rc= _data[pos];
  _data[pos]= np;
  return rc;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
void ArrayPtr<T>::map(std::function<T* (T*)> m) {
  for (auto i = 0; i < _sz; ++i) {
    _data[i] = m(_data[i]);
  }
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
int ArrayPtr<T>::find(T *v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v == _data[i]) { return i; }
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
void ArrayPtr<T>::fill(T *v) {
  for (auto i = 0; i < _sz; ++i) {
    _data[i]=v;
  }
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
bool ArrayPtr<T>::some(T *v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v == _data[i]) { return true; }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
bool ArrayPtr<T>::notAny(T *v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v == _data[i]) { return false; }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
bool ArrayPtr<T>::every(T *v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v != _data[i]) { return false; }
  }
  return _sz > 0 ;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayPtr<T>* ArrayPtr<T>::clone() {
  auto rc= new ArrayPtr<T>(this->_sz);
  for (auto i=0; i < this->_sz; ++i) {
    rc->_data[i] = this->_data[i];
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
void ArrayPtr<T>::set(int pos, T *v) {
  swap(pos,v);
}

//////////////////////////////////////////////////////////////////////////
template<typename T>
T* ArrayPtr<T>::first() {
  assert(_sz > 0);
  return _data[0];
}

//////////////////////////////////////////////////////////////////////////
template<typename T>
T* ArrayPtr<T>::last() {
  assert(_sz > 0);
  return _data[_sz-1];
}

//////////////////////////////////////////////////////////////////////////
template<typename T>
void ArrayPtr<T>::setFirst(T *v) {
  assert(_sz > 0);
  _data[0]= v;
}

//////////////////////////////////////////////////////////////////////////
template<typename T>
void ArrayPtr<T>::setLast(T *v) {
  assert(_sz > 0);
  _data[_sz-1]=v;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
T* ArrayPtr<T>::get(int pos) {
  assert(pos >= 0 && pos < _sz);
  return _data[pos];
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
T* ArrayPtr<T>::operator[](int pos) {
  return get(pos);
}





}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

