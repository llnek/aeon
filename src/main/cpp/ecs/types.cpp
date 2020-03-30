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

#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::ecs {

  Registry::~Registry() {
    for (auto i=_rego.begin(),e=_rego.end();i!=e;++i) {
      DEL_PTR(i->second);
    }
  }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
AttrCache* Registry::getCache(const AttrType& c) const {
  if (auto it= _rego.find(c); it != _rego.end()) {
    return it->second;
  } else {
    return NULL;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Registry::unbind(EAttr c, ENode e) {
  unbind(c->typeId(), e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Registry::unbind(const AttrType& cid, ENode e) {
  if (auto it= _rego.find(cid); it != _rego.end()) {
    auto eid= e->eid();
    auto m= it->second;
    if (auto it2= m->find(eid); it2 != m->end()) {
      m->erase(it2);
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Registry::bind(EAttr c, ENode e) {
  auto cid= c->typeId();
  auto eid= e->eid();
  AttrCache* m;

  if (auto it= _rego.find(cid); it != _rego.end()) {
    m= it->second;
  } else {
    m= new AttrCache;
    _rego.insert(s__pair(AttrType,AttrCache*,cid,m));
  }
  m->insert(s__pair(NodeId,EAttr, eid, c));
}










//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF




