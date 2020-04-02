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

#include <iostream>
#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::ecs {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a= czlab::aeon;
namespace e= czlab::ecs;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Location : public e::Component {
  Location() {}
  virtual ~Location() {}
};

struct Health : public e::Component {
  Health() {}
  virtual ~Health() {}
};

struct Flyable : public e::Component {
  Flyable() {}
  virtual ~Flyable() {}
};

struct Runnable : public e::Component {
  Runnable() {}
  virtual ~Runnable() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct S1 : public e::System {

  virtual ~S1() {}
  S1(Engine* g) : e::System(g) {}

  virtual bool update(float time) {
    std::cout << "update - S1\n";
    return true;
  }
  virtual void preamble() {
    std::cout << "preamble - S1\n";
  }
  virtual int priority() const {
    return 1;
  }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct S2 : public e::System {

  virtual ~S2() {}
  S2(Engine* g) : e::System(g) {}

  virtual bool update(float time) {
    std::cout << "update - S2\n";
    return true;
  }
  virtual void preamble() {
    std::cout << "preamble - S2\n";
  }
  virtual int priority() const { return 2; }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct S3 : public e::System {

  virtual ~S3() {}
  S3(Engine* g) : e::System(g) {}

  virtual bool update(float time) {
    std::cout << "update - S3\n";
    return true;
  }
  virtual void preamble() {
    std::cout << "preamble - S3\n";
  }
  virtual int priority() const { return 3; }
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Game : public e::Engine {
  Game() {}
  virtual ~Game() {}

  virtual void initEnts() {
    auto a= reifyEnt("a");
    auto b= reifyEnt("b");
    rego()->bind<Location>(new Location(),a);
    rego()->bind<Location>(new Location(),b);

    rego()->bind<Health>(new Health(),a);
    rego()->bind<Health>(new Health(),b);

    rego()->bind<Runnable>(new Runnable(),a);
    rego()->bind<Flyable>(new Flyable(),b);
  }

  virtual void initSystems() {
    addSystem(new S2(this));
    addSystem(new S1(this));
    addSystem(new S3(this));
  }

};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
#if 0
using namespace czlab::ecs;

int main(int ac, char** av) {
  Game* g = new Game();
  g->ignite();
  g->update(1);

  auto rc= g->getEnts();
  for (auto & i : rc) {
    std::cout << "eid = " << i->id() << "\n";
    g->rego()->unbind<Flyable>(i);
  }


  auto t= g->getEnts<Flyable>();
  std::cout << "cnt = " << t.size() << "\n";




  delete g;
  std::cout << "yo! "    << "\n";
  return 0;
}
#endif


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

