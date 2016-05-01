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

#pragma once
//////////////////////////////////////////////////////////////////////////////

#include "Ecs.h"
NS_BEGIN(ecs)

//////////////////////////////////////////////////////////////////////////////
//
class Engine;
class MS_DLL Node : public f::Poolable {
friend class Engine;

  //owns all the parts
  __decl_map(COMType, Component*,_parts)
  __decl_ptr(Engine, _engine)
  __decl_bf(_dead)
  __decl_md(NodeId, _eid)
  __decl_md(sstr, _name)

  Node(not_null<Engine*>, const sstr&, NodeId);
  void die() { _dead=true; }

  __decl_nocpyass(Node)
  __decl_nodft(Node)
  virtual ~Node();

public:

  // takeover the component, the node owns it now
  void checkin(not_null<Component*>);
  void purge(const COMType&);

  Component* get(const COMType&);
  bool has(const COMType&);

  NodeId getEid() { return _eid; }
  bool isOk() { return !_dead; };

  const s_vec<Component*> getAll();
};


NS_END



