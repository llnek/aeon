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
  virtual d::DslValue setValueEx(cstdstr&, d::DslValue);
  virtual d::DslValue setValue(cstdstr&, d::DslValue);
  virtual d::DslValue getValue(cstdstr&) const;

  virtual d::DslFrame pushFrame(cstdstr& name);
  virtual d::DslFrame popFrame();
  virtual d::DslFrame peekFrame() const;


  stdstr readString();
  double readFloat();
  llong readInt();
  void writeString(cstdstr&);
  void writeFloat(double);
  void writeInt(llong);
  void writeln();

  //analyzer
  virtual d::DslSymbol search(cstdstr&) const;
  virtual d::DslSymbol find(cstdstr&) const;
  virtual d::DslTable pushScope(cstdstr&);
  virtual d::DslTable popScope();
  virtual d::DslSymbol define(d::DslSymbol);

  Interpreter(const Tchar* src);
  d::DslValue interpret();
  virtual ~Interpreter() {}

  private:

  const Tchar* source;
  d::DslFrame stack;
  d::DslTable symbols;
  void check(d::DslAst);
  d::DslValue eval(d::DslAst);
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

