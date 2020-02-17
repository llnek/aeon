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
#include "fusilli.h"
namespace
fusii
{
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
class MS_DLL Poolable {
  bool _status=false;
protected:
  Poolable() {}
public:
  virtual void yield() { _status=false; }
  virtual void take() { _status=true; }
  bool status() { return _status; }
  virtual ~Poolable() {}
};

//////////////////////////////////////////////////////////////////////////////
class MS_DLL Pool {

  std::function<Poolable* ()> _ctor;
  std::vector<Poolable*> _objs;
  bool _ownObjects=false;
  int _batch=0;

  Pool&operator =(const Pool&) = delete;
  Pool&operator =(Pool&&) = delete;

  Pool(const Pool&) = delete;
  Pool(Pool&&) = delete;

public:

  const std::vector<Poolable*>& list() { return _objs; }
  const std::vector<Poolable*> actives();

  Poolable* select(std::function<bool (Poolable*)>);
  void preset(std::function<Poolable* ()>, int);

  Poolable* take(bool create=false);
  Poolable* get(bool create=false);
  Poolable* getAt(int n);

  int size() { return _objs.size(); }
  int countActives();

  void foreach(std::function<void (Poolable*)>);
  bool some(std::function<bool (Poolable*)>);
  void clearAll();

  void checkin(Poolable*);
  void reset();

  Pool(bool owner) { _ownObjects=owner; }
  virtual ~Pool() {  clearAll(); }
};


}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

