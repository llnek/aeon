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
struct MS_DLL Counted {

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
struct MS_DLL ManagedPtr {

  ManagedPtr(const ManagedPtr& rhs) : ManagedPtr() { retain(rhs.pObj); }

  ManagedPtr(T* obj) : ManagedPtr() { retain(obj); }

  ManagedPtr() { S_NIL(pObj); }

  const ManagedPtr& operator = (const ManagedPtr& rhs) {
    retain(rhs.pObj);
    return *this;
  }

  bool operator == (const ManagedPtr& rhs) {
    return pObj == rhs.pObj;
  }

  bool operator != (const ManagedPtr& rhs) {
    return pObj != rhs.pObj;
  }

  ~ManagedPtr() { release(); }

  T* operator -> () const { return get(); }

  T* get() const { return pObj; }

private:

  void retain(T* obj) {
    if (X_NIL(obj)) {
      obj->retain();
    }
    release();
    pObj = obj;
  }

  void release() {
    if (X_NIL(pObj) && pObj->release() == 0) {
      del_ptr(pObj);
    }
  }

  T* pObj;
};



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

