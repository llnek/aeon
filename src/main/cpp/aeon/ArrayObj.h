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
//////////////////////////////////////////////////////////////////////////////
// Dynamic array of primitive types and
// objects.
template<typename T>
class MS_DLL ArrayObj {

  T *_data=nullptr;
  int _sz=0;

public:

  ArrayObj<T>& operator=(const ArrayObj<T>&);
  ArrayObj<T>& operator=(ArrayObj<T>&&);

  ArrayObj(const ArrayObj<T>&);
  ArrayObj(ArrayObj<T>&&);

  explicit ArrayObj(int z);
  ArrayObj() = delete;
  virtual ~ArrayObj();

  ArrayObj<T>* clone();

  void setFirst(const T &value);
  void setLast(const T &value);

  const T& first();
  const T& last();

  void set(int pos, const T &value);
  int size() { return _sz; }

  bool notAny(const T &v);
  bool some(const T &v);
  bool every(const T &v);
  int find(const T &v);
  void fill(const T &v);

  const T& operator[](int pos);
  const T& get(int pos);
  T& getRef(int pos);

};

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayObj<T>& ArrayObj<T>::operator=(ArrayObj<T> &&src) {
  del_arr(_data);
  _data=src._data;
  _sz=src._sz;
  s__nil(src._data)
  src._sz=0;
  return *this;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayObj<T>::ArrayObj(ArrayObj<T> &&src) {
  _data=src._data;
  _sz=src._sz;
  s__nil(src._data)
  src._sz=0;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayObj<T>& ArrayObj<T>::operator=(const ArrayObj<T> &src) {
  del_arr(_data);
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
template<typename T>
ArrayObj<T>::ArrayObj(const ArrayObj<T> &src) {
  s__nil(_data)
  _sz=src._sz;
  if (_sz > 0) {
    _data= new T[_sz];
    for (auto i=0; i < _sz; ++i) {
      _data[i] = src._data[i];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayObj<T>::ArrayObj(int z) {
  _data = z > 0 ? new T[z] : nullptr;
  _sz=z;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayObj<T>::~ArrayObj() {
  del_arr(_data);
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
int ArrayObj<T>::find(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v == _data[i]) { return i; }
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
void ArrayObj<T>::fill(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    _data[i]=v;
  }
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
bool ArrayObj<T>::some(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v == _data[i]) { return true; }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
bool ArrayObj<T>::notAny(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v == _data[i]) { return false; }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
bool ArrayObj<T>::all(const T &v) {
  for (auto i = 0; i < _sz; ++i) {
    if (v != _data[i]) { return false; }
  }
  return _sz > 0 ;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
ArrayObj<T>* ArrayObj<T>::clone() {
  auto rc= new ArrayObj<T>(this->_sz);
  for (auto i=0; i < this->_sz; ++i) {
    rc->_data[i] = this->_data[i];
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
void ArrayObj<T>::set(int pos, const T &v) {
  assert(pos >= 0 && pos < _sz);
  _data[pos] = v;
}

//////////////////////////////////////////////////////////////////////////
template<typename T>
const T& ArrayObj<T>::first() {
  assert(_sz > 0);
  return _data[0];
}

//////////////////////////////////////////////////////////////////////////
template<typename T>
const T& ArrayObj<T>::last() {
  assert(_sz > 0);
  return _data[_sz-1];
}

//////////////////////////////////////////////////////////////////////////
template<typename T>
void ArrayObj<T>::setFirst(const T &v) {
  assert(_sz > 0);
  _data[0]= v;
}

//////////////////////////////////////////////////////////////////////////
template<typename T>
void ArrayObj<T>::setLast(const T &v) {
  assert(_sz > 0);
  _data[_sz-1]=v;
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
T& ArrayObj<T>::getRef(int pos) {
  assert(pos >= 0 && pos < _sz);
  return _data[pos];
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
const T& ArrayObj<T>::get(int pos) {
  assert(pos >= 0 && pos < _sz);
  return _data[pos];
}

//////////////////////////////////////////////////////////////////////////////
template<typename T>
const T& ArrayObj<T>::operator[](int pos) {
  assert(pos >= 0 && pos < _sz);
  return _data[pos];
}

//////////////////////////////////////////////////////////////////////////
typedef ArrayObj<float> FloatArray;
typedef ArrayObj<int> IntArray;
typedef ArrayObj<bool> BoolArray;


}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


