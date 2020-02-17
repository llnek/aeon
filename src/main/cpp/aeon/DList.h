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
namespace fusii {

  template<typename T> struct MS_DLL DListItem {
    T *_prev=nullptr;
    T *_next=nullptr;
  };

  template<typename T> struct MS_DLL DListAnchor {
    T *_head=nullptr;
    T *_tail=nullptr;
  };

  //////////////////////////////////////////////////////////////////////////////
  // owns all items in this list
  template <typename T>
  struct MS_DLL DList  {

    virtual bool isEmpty() { return _head==nullptr; }
    virtual void release(T*);
    virtual void purge(T*);
    virtual void add(T*);

    const std::vector<T*> list();
    void clear();
    int size();

    T *_head=nullptr;
    T *_tail=nullptr;

    virtual ~DList();
    DList() {}

    DList& operator=(const DList&) = delete;
    DList& operator=(DList&&) = delete;
    DList(const DList&) = delete;
    DList(DList&&) = delete;
  };

  template <typename T>
  void DList<T>::add(T* e) {
    if (E_NIL(_head)) {
      assert(E_NIL(_tail));
      s__nil(e->_prev);
      s__nil(e->_next);
      _head = e;
      _tail = e;
    } else {
      e->_prev = _tail;
      s__nil(e->_next);
      _tail->_next = e;
      _tail = e;
    }
  }

  template<typename T>
  int DList<T>::size() {
    auto n= _head;
    int c=0;
    while (X_NIL(n)) {
      n = n->_next;
      ++c;
    }
    return c;
  }

  template <typename T>
  void DList<T>::release(T* e) {
    if (_tail == e) { _tail = _tail->_prev; }
    if (_head == e) { _head = _head->_next; }
    if (X_NIL(e->_prev)) {
      e->_prev->_next = e->_next;
    }
    if (X_NIL(e->_next)) {
      e->_next->_prev = e->_prev;
    }
    s__nil(e->_prev);
    s__nil(e->_next);
  }

  template <typename T>
  void DList<T>::purge(T* e) {
    release(e);
    delete e.get();
  }

  template <typename T>
  void DList<T>::clear() {
    while (X_NIL(_head)) {
      auto e= _head;
      _head = _head->_next;
      delete e;
    }
    s__nil(_head);
    s__nil(_tail);
  }

  template <typename T>
  const std::vector<T*> DList<T>::list() {
    std::vector<T*> v;
    for (auto p= _head; X_NIL(p); p=p->_next) {
      s__conj(v,p);
    }
    return v;
  }

  template <typename T>
  DList<T>::~DList() {
    //printf("DList dtor\n");
    clear();
  }

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

