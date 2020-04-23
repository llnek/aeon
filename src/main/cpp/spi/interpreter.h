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

#include "parser.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::spi {
namespace d= czlab::dsl;
//
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Interpreter : public d::IEvaluator, public d::IAnalyzer {

  //evaluator
  virtual d::DValue setValueEx(cstdstr&, d::DValue);
  virtual d::DValue setValue(cstdstr&, d::DValue);

  virtual d::DValue getValue(cstdstr&) const;
  //virtual bool contains(const stdstr&) const;
  virtual d::DFrame pushFrame(cstdstr& name);
  virtual d::DFrame popFrame();
  virtual d::DFrame peekFrame() const;

  stdstr readString() { return "";}
  double readFloat() { return 0;}
  long readInt() { return 0L;}
  void writeString(cstdstr&) {}
  void writeFloat(double) {}
  void writeInt(long) {}
  void writeln() {}

  //analyzer
  virtual d::DSymbol search(cstdstr&) const;
  virtual d::DSymbol find(cstdstr&) const;

  virtual d::DTable pushScope(cstdstr& name);
  virtual d::DTable popScope();
  virtual d::DSymbol define(d::DSymbol);

  Interpreter(const Tchar* src);
  d::DValue interpret();
  virtual ~Interpreter() {}

  private:

  const char* source;
  d::DFrame stack;
  d::DTable symbols;

  void check(d::DAst);
  d::DValue eval(d::DAst);

};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

