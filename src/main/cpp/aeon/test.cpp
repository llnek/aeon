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

#include "aeon.h"
//////////////////////////////////////////////////////////////////////////////
namespace czlab::aeon {
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Poop {
  Poop(int n) {x=n;}
  int x;
};

void* mkpoop() {
  return (void*) new Poop(0);
}

void test0() {
  auto v= tokenize("hello joe,how are you,goodbye!", ',');
  for (auto& s : v) {
    ::printf("t=%s\n", s.c_str());
  }
}

void test3() {
  DList<int> v;

  v.add(1);
  v.add(2);
  v.add(3);
  ::printf("size = %d\n", v.size());
  ::printf("empty? = %d\n", (int)v.isEmpty());
  v.remove(2);
  v.remove(1);
  ::printf("size = %d\n", v.size());
  ::printf("empty? = %d\n", (int)v.isEmpty());
  v.add(4);
  auto z= v.list();
  for (auto& x : z) {
    ::printf("e = %d\n", x);
  }
  v.clear();
  ::printf("size = %d\n", v.size());
  ::printf("empty? = %d\n", (int)v.isEmpty());

}

void test2() {
  MemPool p(mkpoop,2);
  Poop* p1 = (Poop*) p.take();
  p1->x=1;
  Poop* p2 = (Poop*) p.take();
  p2->x=2;
  int z1 = p.capacity();
  int n1 = p.count();
  Poop* p3 = (Poop*) p.take();
  p3->x=3;
  Poop* p4 = (Poop*) p.take();
  p4->x=4;
  int z2 = p.capacity();
  int n2 = p.count();
  ::printf("z1 = %d, n1 = %d\n", z1, n1);
  ::printf("z2 = %d, n2 = %d\n", z2, n2);
  p.drop(p1);
  p.drop(p2);
  int z3 = p.capacity();
  int n3 = p.count();
  ::printf("z3 = %d, n3 = %d\n", z3, n3);
  Poop* p5 = (Poop*) p.take();
  Poop* p6 = (Poop*) p.take();
  int z4 = p.capacity();
  int n4 = p.count();
  ::printf("z4 = %d, n4 = %d\n", z4, n4);

  Poop* obj1= (Poop*) p.nth(0);
  Poop* obj2= (Poop*) p.nth(1);

  ::printf("p1 = %d, p2 = %d\n", obj1->x, obj2->x);
  ::printf("p5 = %d, p6 = %d\n", p5->x, p6->x);
}

void test1() {
  Array<Poop*> a(4);
  a.set(0,new Poop(1));
  a.set(1,new Poop(2));
  a.set(2,new Poop(3));
  a.set(3,new Poop(4));
  for (int i=0; i < 4; ++i) {
    auto x = a[i];
    ::printf("x[%d] = %d\n", i, x->x);
  }




}







//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}


int Xmain(int ac, char* av[]) {
  czlab::aeon::test0();
  //czlab::aeon::test1();
  //czlab::aeon::test2();
  //czlab::aeon::test3();
  return 0;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


