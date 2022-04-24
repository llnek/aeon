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
typedef long EntityId;
typedef long Cid;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Component;
struct System;
struct Entity;
struct Engine;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef a::RefPtr<Component> EComponent;
typedef a::RefPtr<System> ESystem;
typedef a::RefPtr<Entity> EEntity;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct EntityFeatureBase {
  protected:
  static Cid nextId() { return ++_lastId; }
  static Cid _lastId;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
struct EntityFeature : public EntityFeatureBase {
  static Cid id() {
    static Cid _id = nextId(); return _id; }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct MSVC_DLL Component : public a::Counted {
  virtual ~Component();
  Component() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::map<EntityId,EComponent> MapEidC;
typedef std::map<EntityId,EEntity> MapEidE;
typedef std::vector<EEntity> EntVec;
typedef std::vector<EComponent> ComVec;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct MSVC_DLL Entity : public a::Counted {

  bool isOk() const { return !_dead; };
  EntityId id() const { return _eid; }

  virtual ~Entity() {}

  friend struct Engine;
  private:

  Engine* _engine;
  bool _dead=false;
  EntityId _eid;
  stdstr _name;

  Entity(Engine*, const stdstr&);
  Entity(Engine*);
  void die() { _dead=true; }

  Entity(const Entity&) = delete;
  Entity() = delete;
  Entity& operator=(const Entity&) = delete;
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

  MapEidC* getCache(const Cid&) const;

  template<typename T>
  MapEidC* getCache() const;

  template<typename T>
  void unbind(EEntity e);

  template<typename T>
  void bind(T* c, EEntity e);

  virtual ~Registry();
  Registry() {}

  private:

  std::map<Cid, MapEidC*> _rego;
  Registry(const Registry&) = delete;
  Registry& operator=(const Registry&) = delete;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct MSVC_DLL Engine {

  // return entities with these components
  EntVec getEnts(const std::vector<Cid>&) const;

  // return entities with this component
  template<typename T>
  EntVec getEnts() const;

  // return all the entities
  EntVec getEnts() const;

  // @name name a node, really for debugging only
  // @take only relevant when entity is pooled
  EEntity reifyEnt(const stdstr& name, bool take=false);
  EEntity reifyEnt(bool take=false);

  // return the config
  const j::json& getCfg() const { return _config; }

  // the singleton registry
  Registry* rego() const { return _types; }

  // remove systems
  void purgeSystem(ESystem);
  void purgeSystems();

  // remove nodes
  void purgeEnt(EEntity);
  void purgeEnts();

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
  virtual void initEnts() = 0;

  private:

  std::vector<ESystem> _systems;
  j::json _config;
  MapEidE _ents;
  EntVec _garbo;
  Registry* _types;
  bool _updating=false;

  Engine(const Engine&)=delete;
  Engine& operator=(const Engine&)=delete;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
MapEidC* Registry::getCache() const {
  if (auto i=_rego.find(EntityFeature<T>::id()); i != _rego.end()) {
    return i->second;
  } else {
    return NULL;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
void Registry::unbind(EEntity e) {
  if (auto i= _rego.find(EntityFeature<T>::id()); i != _rego.end()) {
    auto eid= e->id();
    auto m= i->second;
    if (auto it2= m->find(eid); it2 != m->end()) {
      m->erase(it2);
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
void Registry::bind(T* c, EEntity e) {
  auto cid= EntityFeature<T>::id();
  auto eid= e->id();

  if (auto i= _rego.find(cid); i != _rego.end()) {} else {
    _rego.insert(s__pair(Cid,MapEidC*,cid, new MapEidC));
  }
  _rego[cid]->insert(s__pair(EntityId,EComponent, eid, c));
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
template<typename T>
EntVec Engine::getEnts() const {
  EntVec out;
  if (auto cc= _types->getCache<T>(); cc) {
    for (auto i= cc->begin(),e=cc->end();i != e;++i) {
      auto z= i->first;
      if (auto it2= _ents.find(z); it2 != _ents.end()) {
        s__conj(out,it2->second);
      }
    }
  }
  return out;
}



















//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

