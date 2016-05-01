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

#include "fusilli.h"
NS_ALIAS(s,std)
NS_BEGIN(fusii)

//////////////////////////////////////////////////////////////////////////////
//
class MS_DLL Poolable {
  __decl_bf(_status)
protected:
  Poolable() {}
public:
  virtual void yield() { _status=false; }
  virtual void take() { _status=true; }
  bool status() { return _status; }
  virtual ~Poolable() {}
};

//////////////////////////////////////////////////////////////////////////////
//
class MS_DLL FPool {

  s::function<Poolable* ()> _ctor;
  s_vec<Poolable*> _objs;
  __decl_bf(_ownObjects)
  __decl_iz(_batch)
  __decl_nocpyass(FPool)

public:

  const s_vec<Poolable*>& ls() { return _objs; }
  const s_vec<Poolable*> actives();

  Poolable* select(s::function<bool (Poolable*)>);
  void preset(s::function<Poolable* ()>, int);

  Poolable* take(bool create=false);
  Poolable* get(bool create=false);
  Poolable* getAt(int n);

  int size() { return _objs.size(); }
  int countActives();

  void foreach(s::function<void (Poolable*)>);
  bool some(s::function<bool (Poolable*)>);
  void clearAll();

  void checkin(not_null<Poolable*>);
  void reset();

  FPool(bool owner) { _ownObjects=owner; }
  virtual ~FPool() {  clearAll(); }

};


NS_END




