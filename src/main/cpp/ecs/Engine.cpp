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
Engine::Engine() {
  _types= new Registry();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Engine::~Engine() {
  doHouseKeeping();
  DEL_PTR(_types);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NodeVec Engine::getNodes(const ATypeVec& cs) const {
  std::vector<AttrCache*> ccs;
  NodeVec out;
  AttrCache* pm;
  auto pmin= INT_MAX;
  auto missed=false;

  //find shortest cache, doing an intersection
  for (auto it= cs.begin(), e=cs.end(); it != e; ++it) {
    auto cid= *it;
    auto c= _types->getCache(cid);
    if (E_NIL(c)) {
      DEBUG("cache missed when looking for intersection on %s", C_STR(cid));
      missed=true;
      break;
    }
    if (c->size() < pmin) {
      pmin= c->size();
      pm=c;
    }
    s__conj(ccs, c);
  }

  if (missed) { return out; }

  DEBUG("intesection on %d caches", (int)ccs.size());

  if (ccs.size() > 0) {
    //use the shortest cache as the baseline
    for (auto it = pm->begin(), e= pm->end(); it != e; ++it) {
      auto eid= it->first;
      auto sum=0;
      for (auto it2 = ccs.begin(), e2= ccs.end(); it2 != e2; ++it2) {
        // look for intersection
        auto c= *it2;
        if (c == pm) {
          ++sum;
          continue;
        }
        if (auto it3= c->find(eid); it3 != c->end()) {
          ++sum;
        }
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
NodeVec Engine::getNodes(const AttrType& c) const {
  auto cc= _types->getCache(c);
  NodeVec out;

  if (cc)
  for (auto it= cc->begin(), e=cc->end(); it != e; ++it) {
    auto z= it->first;
    if (auto it2= _ents.find(z); it2 != _ents.end()) {
      s__conj(out,it2->second);
    }
  }

  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NodeVec Engine::getNodes() const {
  NodeVec out;
  for (auto it = _ents.begin(), e= _ents.end(); it != e; ++it) {
    s__conj(out, it->second);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NodeId Engine::generateEid() {
  auto rc= ++_lastId;
  if (rc < INT_MAX) {} else {
    throw "too many entities";
  }
  return rc;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ENode Engine::reifyNode(const stdstr& n, bool take) {
  auto eid= generateEid();
  auto e= new Node(this, n, eid);
  _ents.insert(s__pair(NodeId,ENode,eid,e));
  //if (take) {e->take();}
  return e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::purgeNode(ENode e) {
  assert(e.isSome());
  e->die();
  s__conj(_garbo, e);

  if (auto it= _ents.find(e->eid()); it != _ents.end()) {
    _ents.erase(it);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::purgeNodes() {
  _ents.clear();
  doHouseKeeping();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ESystem Engine::regoSystem(ESystem arg) {
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
  for (auto i= _systems.begin(), e= _systems.end(); i != e; ++i) {
    auto s= *i;
    if (s->isActive()) {
      if (! s->update(time)) { break; }
    }
  }
  doHouseKeeping();
  _updating = false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::ignite() {
  initEntities();
  initSystems();
  for (auto i= _systems.begin(), e= _systems.end(); i != e; ++i) {
    auto s= *i;
    s->preamble();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Engine::doHouseKeeping() {
  _garbo.clear();
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


