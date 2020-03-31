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

#include "types.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::ecs {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/*
A* myA = new A;
Will_Get_Function_Pointer(1.00,2.00, &myA->Minus);
void Will_Get_Function_Pointer(A* object, float a, float b,
float (A::*pt2Func)(float, float)) {
   (object->*pt2Func)(a, b);
}
A* myA = new A;
Will_Get_Function_Pointer(myA, 1.00, 2.00, &A::Minus);
*/
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool _compSystem(ESystem lhs, ESystem rhs) {
  // we want it to be descending
  return lhs->priority() > rhs->priority();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Engine::Engine() { _types= new Registry(); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Engine::~Engine() { DEL_PTR(_types); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
EntVec Engine::getEnts(const std::vector<Cid>& cs) const {
  std::vector<MapEidC*> ccs;
  EntVec out;
  MapEidC* pm;
  auto pmin= LONG_MAX;

  //find shortest cache, doing an intersection
  for (auto& cid : cs) {
    auto c= _types->getCache(cid);
    if (E_NIL(c)) {
      DEBUG("cache missed on %s", C_STR(cid));
      return out;
    }
    if (c->size() < pmin) {
      pmin= c->size();
      pm=c;
    }
    s__conj(ccs, c);
  }

  DEBUG("intesection on %ud caches", ccs.size());

  if (ccs.size() > 0) {
    //use the shortest cache as the baseline
    for (auto i= pm->begin(),e= pm->end();i != e;++i) {
      auto eid= i->first;
      auto sum=0;
      for (auto& c : ccs) {
        // look for intersection
        if (c == pm) { ++sum; continue; }
        if (s__contains(*c, eid)) { ++sum; }
      }
      // if found in all caches...matched!
      if (sum == ccs.size()) {
        if (auto it4= _ents.find(eid); it4 != _ents.end()) {
          s__conj(out,it4->second);
        }
      }
    }
  }

  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
EntVec Engine::getEnts() const {
  EntVec out;
  for (auto i=_ents.begin(),e= _ents.end();i != e; ++i) {
    s__conj(out, i->second);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
EEntity Engine::reifyEnt(const stdstr& n, bool take) {
  auto e= new Entity(this, n);
  _ents.insert(s__pair(EntityId,EEntity,e->id(),e));
  //if (take) {e->take();}
  return e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
EEntity Engine::reifyEnt(bool take) {
  auto e= new Entity(this);
  _ents.insert(s__pair(EntityId,EEntity,e->id(),e));
  //if (take) {e->take();}
  return e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::purgeEnt(EEntity e) {
  assert(e.isSome());
  e->die();
  s__conj(_garbo, e);

  if (auto i= _ents.find(e->id()); i != _ents.end()) {
    _ents.erase(i);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::purgeEnts() {
  _garbo.clear();
  _ents.clear();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ESystem Engine::addSystem(ESystem arg) {
  auto p= arg->priority();
  auto i= _systems.begin();
  auto e= _systems.end();
  for (; i != e; ++i) {
    auto s= *i;
    if (p > s->priority())
    break;
  }
  _systems.insert(i,arg);
  return arg;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::purgeSystem(ESystem s) {
  for (auto i= _systems.begin(), e= _systems.end(); i != e; ++i) {
    auto p= *i;
    if (p.ptr()== s.ptr()) {
      _systems.erase(i);
      s=NULL;
      break;
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::purgeSystems() {
  _systems.clear();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::update(float time) {
  _updating = true;
  for (auto i=_systems.begin(),e=_systems.end();i != e;++i) {
    auto s= *i;
    if (s->isActive()) {
      if (! s->update(time)) { break; }
    }
  }
  _garbo.clear();
  _updating = false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::ignite() {
  (initEnts(), initSystems());
  for (auto i= _systems.begin(),e= _systems.end();i != e;++i) {
    auto s= *i;
    s->preamble();
  }
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


