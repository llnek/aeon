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

#include "nlohmann/json.hpp"
#include "System.h"
#include "Node.h"

NS_ALIAS(j, nlohmann)
NS_BEGIN(ecs)

//////////////////////////////////////////////////////////////////////////////
//
class TypeRegistry;
class MS_DLL Engine {

  __decl_md(SystemList, _systemList)
  __decl_md(NodeCache, _ents)

  __decl_ptr(TypeRegistry, _types)
  __decl_bf(_updating)
  __decl_iz(_lastId)

  __decl_vec(Node*,_garbo)
  __decl_nocpyass(Engine)

  void doHouseKeeping();
  NodeId generateEid();

protected:

  virtual void initEntities() = 0;
  virtual void initSystems() = 0;
  __decl_md(j::json, _config)

public:

  // return entities with these components
  void getNodes(const s_vec<COMType>&, s_vec<Node*>&);
  const s_vec<Node*> getNodes(const s_vec<COMType>&);

  // return entities with this component
  void getNodes(const COMType&, s_vec<Node*>&);
  const s_vec<Node*> getNodes(const COMType&);

  // return all the entities
  void getNodes(s_vec<Node*>& );
  const s_vec<Node*> getNodes();

  // @name name a node, really for debugging only
  // @take only relevant when entity is pooled
  Node* reifyNode(const sstr &name, bool take=false);

  // return the config
  const j::json& getCfg() { return _config; }

  // the singleton registry
  TypeRegistry* rego() { return _types; }

  // remove systems
  void purgeSystem (not_null<System*>);
  void purgeSystems();

  // remove nodes
  void purgeNode(not_null<Node*>);
  void purgeNodes();

  // register+add a system
  void regoSystem(not_null<System*> );

  // start the engine
  void ignite();

  // each update called will update each system
  // in order
  void update(float time);

  // you can pass in some configurations
  Engine(j::json c) : Engine() { _config=c; }
  Engine();
  virtual ~Engine();

};



NS_END


