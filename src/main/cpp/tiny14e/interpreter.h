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
namespace czlab::tiny14e {
namespace d= czlab::dsl;
//
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Interpreter : public EvaluatorAPI, public AnalyzerAPI {
  //evaluator
  virtual d::DslValue setValue(const stdstr&, d::DslValue, bool localOnly);
  virtual d::DslValue getValue(const stdstr&) const;
  virtual bool containsSymbol(const stdstr&) const;
  virtual d::DslFrame pushFrame(const stdstr& name);
  virtual d::DslFrame popFrame();
  virtual d::DslFrame peekFrame() const;


  stdstr readString();
  double readFloat();
  llong readInt();
  void writeString(const stdstr&);
  void writeFloat(double);
  void writeInt(llong);
  void writeln();

  //analyzer
  virtual d::DslSymbol lookup(const stdstr&, bool traverseOuterScope=true) const;
  virtual void pushScope(const stdstr& name);
  virtual d::DslTable popScope();
  virtual d::DslSymbol define(d::DslSymbol);

  Interpreter(const char* src);
  d::DslValue interpret();
  virtual ~Interpreter() {}

private:
  const char* source;
  d::DslValue eval(d::DslAst);
  void check(d::DslAst);
  d::DslFrame stack;
  d::DslTable symbols;
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

