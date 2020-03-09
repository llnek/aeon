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
using namespace czlab::dsl;
namespace d= czlab::dsl;
//
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Interpreter : public EvaluatorAPI, public AnalyzerAPI {
  //evaluator
  DslValue setValue(const stdstr&, d::DslValue, bool localOnly);
  DslValue getValue(const stdstr&);
  DslFrame push(const stdstr&);
  DslFrame pop();
  DslFrame peek();

  stdstr readString();
  double readFloat();
  llong readInt();
  void writeString(const stdstr&);
  void writeFloat(double);
  void writeInt(llong);
  void writeln();

  //analyzer
  void pushScope(const stdstr&);
  DslSymbolTable popScope();
  DslSymbol lookup(const stdstr&, bool traverse);
  void define(DslSymbol);


  Interpreter(const char* src);
  DslValue interpret();
  virtual ~Interpreter() {}

private:
  const char* source;
  DslValue eval(DslAst);
  void check(DslAst);
  DslFrame stack;
  DslSymbolTable symbols;
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

