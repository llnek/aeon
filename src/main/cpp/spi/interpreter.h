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
  virtual d::DslValue setValueEx(const stdstr&, d::DslValue);
  virtual d::DslValue setValue(const stdstr&, d::DslValue);

  virtual d::DslValue getValue(const stdstr&) const;
  //virtual bool contains(const stdstr&) const;
  virtual d::DslFrame pushFrame(const stdstr& name);
  virtual d::DslFrame popFrame();
  virtual d::DslFrame peekFrame() const;

  stdstr readString() { return "";}
  double readFloat() { return 0;}
  long readInt() { return 0L;}
  void writeString(const stdstr&) {}
  void writeFloat(double) {}
  void writeInt(long) {}
  void writeln() {}

  //analyzer
  virtual d::DslSymbol search(const stdstr&) const;
  virtual d::DslSymbol find(const stdstr&) const;

  virtual d::DslTable pushScope(const stdstr& name);
  virtual d::DslTable popScope();
  virtual d::DslSymbol define(d::DslSymbol);

  Interpreter(const char* src);
  d::DslValue interpret();
  virtual ~Interpreter() {}

  private:

  const char* source;
  d::DslFrame stack;
  d::DslTable symbols;

  void check(d::DslAst);
  d::DslValue eval(d::DslAst);

};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

