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

#include "System.h"
NS_BEGIN(ecs)

//////////////////////////////////////////////////////////////////////////////
//
void SystemList::add(not_null<System*> sys) {
  auto s = sys.get();
  if (E_NIL(_head)) {
    s->_next = s->_prev = P_NIL;
    _head = _tail = s;
  } else {
    System *node= P_NIL;
    for (node = _tail; N_NIL(node); node = node->_prev) {
      if (node->priority() <= s->priority() ) {
        break;
      }
    }
    if (node == _tail) {
      _tail->_next = s;
      s->_prev = _tail;
      s->_next = P_NIL;
      _tail = s;
    }
    else
    if (E_NIL(node )) {
      s->_prev = P_NIL;
      s->_next = _head;
      _head->_prev = s;
      _head = s;
    }
    else {
      s->_next = node->_next;
      s->_prev = node;
      node->_next->_prev = s;
      node->_next = s;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
//
System* SystemList::get(const SystemType &type) {
  for (auto s = _head; N_NIL(s); s = s->_next) {
    if (s->isa(type) ) { return s; }
  }
  return P_NIL;
}


NS_END



