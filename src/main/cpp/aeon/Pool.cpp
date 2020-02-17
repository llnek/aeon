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

#include "Pool.h"
namespace
fusii
{
//////////////////////////////////////////////////////////////////////////////
// Pre-populate a bunch of objects in the pool
void Pool::preset(std::function<Poolable* ()> f, int count) {
  for (auto n=0; n < count; ++n) {
    auto rc= f();
    if (rc) s__conj(_objs,rc);
  }
  _batch=count;
  _ctor=f;
}

//////////////////////////////////////////////////////////////////////////
// Find an object by applying this filter
Poolable* Pool::select(std::function<bool (Poolable*)> f) {
  F__LOOP(it, _objs) {
    auto e = *it;
    if (f(e)) return e;
  }
  return P_NIL;
}

//////////////////////////////////////////////////////////////////////////
// Get a free object from the pool and set it's status to true
Poolable* Pool::take(bool create) {
  auto rc= get(create);
  if (rc) rc->take();
  return rc;
}

//////////////////////////////////////////////////////////////////////////
Poolable* Pool::getAt(int pos) {
  return _objs.at(pos);
}

//////////////////////////////////////////////////////////////////////////
// Get a free object from the pool.  More like a peek
Poolable* Pool::get(bool create) {

  F__LOOP(it, _objs) {
    auto e= *it;
    if (! e->status()) {
      return e;
    }
  }

  if (create &&  _ctor) {
    preset(_ctor, _batch);
    return get();
  }

  return P_NIL;
}

//////////////////////////////////////////////////////////////////////////
void Pool::checkin(Poolable* c) {
  assert(c != nullptr), s__conj(_objs,c);
}

//////////////////////////////////////////////////////////////////////////
void Pool::clearAll() {
  if (_ownObjects) {
    F__LOOP(it, _objs) { delete *it; }
  }
  _objs.clear();
}

//////////////////////////////////////////////////////////////////////////////
const std::vector<Poolable*> Pool::actives() {
  std::vector<Poolable*> out;
  F__LOOP(it, _objs) {
    auto z= *it;
    if (z->status()) {
      s__conj(out,z);
    }
  }
  return out;
}

//////////////////////////////////////////////////////////////////////////
// Get the count of active objects
int Pool::countActives() {
  auto c=0;
  F__LOOP(it, _objs) {
    auto z= *it;
    if (z->status()) {
      ++c;
    }
  }
  return c;
}

//////////////////////////////////////////////////////////////////////////
void Pool::foreach(std::function<void (Poolable*)> f) {
  F__LOOP(it, _objs) {
    f(*it);
  }
}

//////////////////////////////////////////////////////////////////////////
bool Pool::some(std::function<bool (Poolable*)> f) {
  F__LOOP(it, _objs) { if (f(*it)) { return true;} }
  return false;
}

//////////////////////////////////////////////////////////////////////////
// Hibernate (status off) all objects in the pool
void Pool::reset() {
  F__LOOP(it, _objs) {
    auto z= *it;
    z->yield();
  }
}



}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





