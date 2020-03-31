#pragma once
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

//////////////////////////////////////////////////////////////////////////////

#include "../nlohmann/json.hpp"
#include "../aeon/smptr.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::ecs {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a= czlab::aeon;
namespace j= nlohmann;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef long AttrId;
typedef long NodeId;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Attribute;
struct System;
struct Node;
struct Engine;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef a::RefPtr<Attribute> EAttr;
typedef a::RefPtr<System> ESystem;
typedef a::RefPtr<Node> ENode;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct MSVC_DLL Attribute : public a::Counted {

  friend struct Node;

  ENode node() const { return _node; }
  AttrId id() { return _id; }

  virtual ~Attribute() {}

  protected:

  Attribute();

  private:

  void setNode(ENode n) { _node=n; }

  ENode _node;
  AttrId _id;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::map<NodeId,EAttr> AttrCache;
typedef std::map<NodeId,ENode> NodeCache;

typedef std::vector<ESystem> SystemVec;
typedef std::vector<AttrId> ATypeVec;
typedef std::vector<EAttr> AttrVec;
typedef std::vector<ENode> NodeVec;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct MSVC_DLL Node : public a::Counted {

  friend struct Engine;

  bool isOk() const { return !_dead; };
  NodeId id() const { return _eid; }

  EAttr get(const AttrId&) const;
  bool has(const AttrId&) const;

  void purge(const AttrId&);
  void checkin(EAttr);

  AttrVec getAll() const;

  virtual ~Node() {}

  private:

  std::map<AttrId, EAttr> _attrs;
  Engine* _engine;
  bool _dead=false;
  NodeId _eid;
  stdstr _name;

  Node(Engine*, const stdstr&);
  Node(Engine*);
  void die() { _dead=true; }

  Node(const Node&) = delete;
  Node() = delete;
  Node& operator=(const Node&) = delete;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct MSVC_DLL System : public a::Counted {

  Engine* engine() const { return _engine; }
  bool isActive() const { return _active; }

  void suspend() { _active=false; }
  void restart() { _active=true; }

  virtual bool update(float time) = 0;
  virtual void preamble() = 0;
  virtual int priority() const = 0;

  virtual ~System() {}

  protected:

  System(Engine* e) { _engine= e; }

  Engine* _engine;
  bool _active=true;

  System()=delete;
  System(const System&)=delete;
  System& operator=(const System&)=delete;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct MSVC_DLL Registry {

  AttrCache* getCache(const AttrId&) const;

  void unbind(const AttrId&, ENode);
  void unbind(EAttr, ENode);
  void bind(EAttr, ENode);

  virtual ~Registry();
  Registry() {}

  private:

  std::map<AttrId, AttrCache*> _rego;
  Registry(const Registry&) = delete;
  Registry& operator=(const Registry&) = delete;

};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct MSVC_DLL Engine {

  // return entities with these components
  NodeVec getNodes(const ATypeVec&) const;

  // return entities with this component
  NodeVec getNodes(const AttrId&) const;

  // return all the entities
  NodeVec getNodes() const;

  // @name name a node, really for debugging only
  // @take only relevant when entity is pooled
  ENode reifyNode(const stdstr& name, bool take=false);
  ENode reifyNode(bool take=false);

  // return the config
  const j::json& getCfg() const { return _config; }

  // the singleton registry
  Registry* rego() const { return _types; }

  // remove systems
  void purgeSystem(ESystem);
  void purgeSystems();

  // remove nodes
  void purgeNode(ENode);
  void purgeNodes();

  // register+add a system
  ESystem addSystem(ESystem);

  // start the engine
  void ignite();

  // each update called will update each system
  // in order
  void update(float time);

  // you can pass in some configurations
  Engine(j::json c) : Engine() { _config=c; }
  Engine();
  virtual ~Engine();

  protected:

  virtual void initSystems() = 0;
  virtual void initNodes() = 0;

  private:

  SystemVec _systems;
  j::json _config;
  NodeCache _ents;
  NodeVec _garbo;
  Registry* _types;
  bool _updating=false;

  Engine(const Engine&)=delete;
  Engine& operator=(const Engine&)=delete;
};






















//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

