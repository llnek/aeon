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

#define __decl_sys_tpid(x) \
  virtual const ecs::SystemType typeId() { return x; }

#define __decl_sys_priority(x) \
  virtual int priority() { return x; }

#define __decl_sys_preamble() \
  virtual void preamble();

#define __decl_sys_update() \
  virtual bool update(float);

//////////////////////////////////////////////////////////////////////////////
//
class Engine;
class MS_DLL System : public f::FDListItem<System> {
protected:

  __decl_ptr(Engine, _engine)
  __decl_bt(_active)

  __decl_nocpyass(System)
  __decl_nodft(System)

public:

  virtual const SystemType typeId() = 0;
  virtual bool update(float time) = 0;
  virtual void preamble()= 0;
  virtual int priority() = 0;

  Engine* getEngine() { return _engine; }
  bool isActive() { return _active; }
  bool isa(const SystemType&  t) {
    return typeId() == t;
  }

  void suspend() { _active=false; }
  void restart() { _active=true; }

  System(not_null<Engine*> e) { _engine= e; }
  virtual ~System() {}
};

//////////////////////////////////////////////////////////////////////////////
//
class MS_DLL SystemList  : public f::FDList<System> {
  __decl_nocpyass(SystemList)
public:

  virtual void add(not_null<System*> );
  System* get(const SystemType&);

  virtual ~SystemList() {}
  SystemList() {}

};


NS_END
#include "Engine.h"



