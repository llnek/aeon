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
#include "Engine.h"
#include "Node.h"
NS_BEGIN(ecs)

//////////////////////////////////////////////////////////////////////////////
//
Node::Node(not_null<Engine*> e, const sstr &n, NodeId eid) {
  this->_engine= e;
  this->_eid= eid;
  this->_name=n;
}

//////////////////////////////////////////////////////////////////////////////
//
Node::~Node() {
  //printf("Node dtor\n");
  F__LOOP(it, _parts) {
    auto c= it->second;
    _engine->rego()->unbind(c,this);
    // only delete if the object is not a reference-counted
    // type
    if (!c->isAuto())
    delete c;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
void Node::checkin(not_null<Component*> c) {
  auto z = c->typeId();
  assert(! has(z));
  _engine->rego()->bind(c,this);
  c->setNode(this);
  _parts.insert(S__PAIR(COMType, Component*, z, c));
}

//////////////////////////////////////////////////////////////////////////////
//
void Node::purge(const COMType &z) {
  auto it = _parts.find(z);
  if (it != _parts.end()) {
    auto c= it->second;
    _parts.erase(it);
    _engine->rego()->unbind(c,this);
    if (!c->isAuto())
    delete c;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
Component* Node::get(const COMType &z) {
  auto it=  _parts.find(z);
  if (it != _parts.end()) {
    return it->second;
  } else {
    return P_NIL;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
const s_vec<Component*> Node::getAll() {
  s_vec<Component*> v;
  F__LOOP(it, _parts) {
    v.push_back(it->second);
  }
  return v;
}

//////////////////////////////////////////////////////////////////////////////
//
bool Node::has(const COMType &z) {
  return _parts.find(z) != _parts.end();
}


NS_END


