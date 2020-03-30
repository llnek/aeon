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

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#include "aeon.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon {

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct MSVC_DLL Counted {

  Counted() : count(0) {}
  virtual ~Counted() {}

  Counted* retain() {
    ++count;
    return this;
  }

  int release() {
    if (count > 0) {
      --count;
    }
    return count;
  }

  int refs() {
    return count;
  }

private:
  Counted& operator= (const Counted&&);
  Counted& operator= (const Counted&);
  Counted(const Counted&&) ;
  Counted(const Counted&) ;
  int count;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<class T>
struct MSVC_DLL RefPtr {

  RefPtr(const RefPtr& rhs) : RefPtr() { retain(rhs.pObj); }

  RefPtr(T* obj) : RefPtr() { retain(obj); }

  RefPtr() { S_NIL(pObj); }

  const RefPtr& operator = (const RefPtr& rhs) {
    retain(rhs.pObj);
    return *this;
  }

  bool operator == (const RefPtr& rhs) const {
    return pObj == rhs.pObj;
  }

  bool operator != (const RefPtr& rhs) const {
    return pObj != rhs.pObj;
  }

  bool isNull() const {
    return E_NIL(pObj);
  }

  bool isSome() const {
    return X_NIL(pObj);
  }

  ~RefPtr() { release(); }

  T* operator -> () const { return ptr(); }

  T* ptr() const { return pObj; }

private:

  // will fail to compile if not subclass of Counted

  void retain(T* obj) {
    if (X_NIL(obj)) {
      obj->retain();
    }
    release();
    pObj = obj;
  }

  void release() {
    if (X_NIL(pObj) && pObj->release() == 0) {
      DEL_PTR(pObj);
    }
  }

  T* pObj;
};



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

