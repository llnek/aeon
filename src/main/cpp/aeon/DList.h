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
namespace czlab::aeon {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//////////////////////////////////////////////////////////////////////////////
// owns all items in this list
template <typename T>
struct MSVC_DLL DList {

  struct DListItem {
    DListItem(T e) {
      item=e;
      _prev=_next=nullptr;
    }
    ~DListItem() {}
    DListItem* _prev;
    T item;
    DListItem* _next;
  };
  struct DListAnchor {
    DListAnchor() {
      _head=_tail=nullptr;
    }
    ~DListAnchor() {}
    DListItem* _head;
    DListItem* _tail;
  };

  struct Iterator;
  Iterator begin() { return Iterator(anchor._head); }
  Iterator end()   { return Iterator(anchor._tail); }
  struct Iterator {
    Iterator(DListItem* d) : node(d) {}
    Iterator& operator ++ () {
      node = node->_next;
      return *this;
    }
    T& operator * () { return node->item; }
    bool operator != (const Iterator& rhs) const {
      return node != rhs.node;
    }
    private:
    DListItem* node;
  };

  bool isEmpty() const { return anchor._head==nullptr; }
  virtual void remove(T);
  virtual void add(T);

  std::vector<T> list() const;
  void clear();
  int size() const;

  virtual ~DList();
  DList() {}

  DList& operator=(const DList&) = delete;
  DList& operator=(DList&&) = delete;
  DList(const DList&) = delete;
  DList(DList&&) = delete;

  protected:

  void purge(DListItem*);
  DListAnchor anchor;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
void DList<T>::add(T e) {
  auto i= new DListItem(e);
  if (! anchor._head) {
    anchor._head = i;
    anchor._tail = i;
  } else {
    i->_prev = anchor._tail;
    anchor._tail->_next = i;
    anchor._tail = i;
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
int DList<T>::size() const {
  auto n= anchor._head;
  int c=0;
  while (X_NIL(n)) {
    ++c;
    n = n->_next;
  }
  return c;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
void DList<T>::remove(T e) {
  auto p = anchor._head;
  while (p) {
    if (p->item == e) {
      purge(p);
      break;
    } else {
      p=p->_next;
    }
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
void DList<T>::purge(DListItem* e) {
  if (anchor._tail == e) { anchor._tail = anchor._tail->_prev; }
  if (anchor._head == e) { anchor._head = anchor._head->_next; }
  if (X_NIL(e->_prev)) {
    e->_prev->_next = e->_next;
  }
  if (X_NIL(e->_next)) {
    e->_next->_prev = e->_prev;
  }
  delete e;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
void DList<T>::clear() {
  auto p= anchor._head;
  while (p) {
    auto n= p;
    p=p->_next;
    delete n;
  }
  s__nil(anchor._head);
  s__nil(anchor._tail);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
std::vector<T> DList<T>::list() const {
  std::vector<T> v;
  for (auto p= anchor._head; X_NIL(p); p=p->_next) {
    s__conj(v,p->item);
  }
  return v;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template <typename T>
DList<T>::~DList() {
  //printf("DList dtor\n");
  clear();
}





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

