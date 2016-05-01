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

#include "FPool.h"
NS_BEGIN(fusii)

//////////////////////////////////////////////////////////////////////////////
// Pre-populate a bunch of objects in the pool
void FPool::preset(s::function<Poolable* ()> f, int count) {
  for (auto n=0; n < count; ++n) {
    auto rc= f();
    if (rc) {
      _objs.push_back(rc);
    }
  }
  _batch=count;
  _ctor=f;
}

//////////////////////////////////////////////////////////////////////////
// Find an object by applying this filter
Poolable* FPool::select(s::function<bool (Poolable*)> f) {
  F__LOOP(it, _objs) {
    auto e = *it;
    if (f(e)) {
      return e;
    }
  }
  return P_NIL;
}

//////////////////////////////////////////////////////////////////////////
// Get a free object from the pool and set it's status to true
Poolable* FPool::take(bool create) {
  auto rc= get(create);
  if (rc) {
    rc->take();
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////
//
Poolable* FPool::getAt(int pos) {
  return _objs.at(pos);
}

//////////////////////////////////////////////////////////////////////////
// Get a free object from the pool.  More like a peek
Poolable* FPool::get(bool create) {
  F__LOOP(it, _objs) {
    auto e= *it;
    if (! e->status()) { return e; }
  }
  if (create &&  _ctor) {
    preset(_ctor, _batch);
    return get();
  }
  return P_NIL;
}

//////////////////////////////////////////////////////////////////////////
//
void FPool::checkin(not_null<Poolable*> c) {
  _objs.push_back(c);
}

//////////////////////////////////////////////////////////////////////////
//
void FPool::clearAll() {
  if (_ownObjects) {
    F__LOOP(it, _objs) { delete *it; }
  }
  _objs.clear();
}

//////////////////////////////////////////////////////////////////////////////
//
const s_vec<Poolable*> FPool::actives() {
  s_vec<Poolable*> out;
  F__LOOP(it, _objs) {
    auto z= *it;
    if (z->status()) {
      out.push_back(z);
    }
  }
  return out;
}

//////////////////////////////////////////////////////////////////////////
// Get the count of active objects
int FPool::countActives() {
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
//
void FPool::foreach(s::function<void (Poolable*)> f) {
  F__LOOP(it, _objs) {
    f(*it);
  }
}

//////////////////////////////////////////////////////////////////////////
//
bool FPool::some(s::function<bool (Poolable*)> f) {
  F__LOOP(it, _objs) { if (f(*it)) { return true;} }
  return false;
}

//////////////////////////////////////////////////////////////////////////
// Hibernate (status off) all objects in the pool
void FPool::reset() {
  F__LOOP(it, _objs) {
    auto z= *it;
    z->yield();
  }
}



NS_END




