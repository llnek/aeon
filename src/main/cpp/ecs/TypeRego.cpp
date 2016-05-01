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

#include "TypeRego.h"
#include "Node.h"
NS_BEGIN(ecs)

//////////////////////////////////////////////////////////////////////////////
//
TypeRegistry::~TypeRegistry() {
  //printf("TypeRegistry dtor\n");
  //shallow delete the caches
  F__LOOP(it, _rego) { delete it->second; }
}

//////////////////////////////////////////////////////////////////////////////
//
CompoCache* TypeRegistry::getCache(const COMType &c) {
  auto it= _rego.find(c);
  if (it != _rego.end()) {
    return it->second;
  } else {
    return P_NIL;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
void TypeRegistry::unbind(not_null<Component*> c, not_null<Node*> e) {
  unbind( c->typeId(), e);
}

//////////////////////////////////////////////////////////////////////////////
//
void TypeRegistry::unbind(const COMType &cid, not_null<Node*> e) {
  auto it= _rego.find(cid);
  if (it != _rego.end()) {
    auto eid= e->getEid();
    auto m= it->second;
    auto it2= m->find(eid);
    if (it2 != m->end()) {
      m->erase(it2);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
//
void TypeRegistry::bind(not_null<Component*> c, not_null<Node*> e) {
  auto cid= c->typeId();
  auto eid= e->getEid();
  CompoCache *m;

  auto it= _rego.find(cid);
  if (it != _rego.end()) {
    m= it->second;
  } else {
    m= mc_new(CompoCache);
    _rego.insert(S__PAIR(COMType,CompoCache*,cid,m));
  }
  m->insert(S__PAIR(NodeId,Component*, eid, c));
}



NS_END



