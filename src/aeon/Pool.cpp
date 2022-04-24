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
 * Copyright © 2013-2022, Kenneth Leung. All rights reserved. */

#include <functional>
#include <map>
#include "Pool.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MemPool::MemPool(std::function<void* ()> f, size_t batch) {
  this->batch= batch;
  this->size= batch;
  this->next= 0;
  this->ctor=f;
  this->slots= (void**) ::malloc(size * sizeof(void*));
  init();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int MemPool::capacity() {
  return this->size;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int MemPool::count() {
  return this->next;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void* MemPool::nth(int pos) {
  return (pos >= 0 && pos < next) ? slots[pos] : nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void MemPool::each(std::function<void (void*)> f) {
  for (auto i = 0; i < next; ++i) {
    f(slots[i]);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void* MemPool::take() {
  if (next < size) {
    auto p=slots[next];
    rego[p]=next;
    ++next;
    return p;
  } else {
    grow();
    return take();
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void MemPool::grow() {
  size += batch;
  slots = (void**) ::realloc(slots, size * sizeof(void*));
  for (auto i= next; i < size; ++i) {
    slots[i] = ctor();
  }
  init();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void MemPool::init() {
  for (auto i=next; i < size; ++i) {
    slots[i] = ctor();
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void MemPool::drop(void* obj) {
  auto it= rego.find(obj);
  int pos;
  if (it != rego.end()) {
    pos=it->second;
    rego.erase(it);
  } else {
    return;
  }
  auto n1 = next-1;
  auto tail = slots[n1];
  // move tail
  slots[pos]=tail;
  rego[tail]=pos;
  // slot in obj for reuse
  slots[n1]=obj;
  --next;
}









//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





