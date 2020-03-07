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
  void setValue(const std::string&, const d::DslValue&, bool localOnly);
  d::DslValue getValue(const std::string&);
  d::DslFrame push(const std::string&);
  d::DslFrame pop();
  d::DslFrame peek();

  std::string readString();
  double readFloat();
  long readInt();
  void writeString(const std::string&);
  void writeFloat(double);
  void writeInt(long);
  void writeln();

  //analyzer
  void pushScope(const std::string&);
  d::SymbolTable* popScope();
  d::Symbol* lookup(const std::string&, bool traverse);
  void define(d::Symbol*);


  Interpreter(const char* src);
  d::DslValue interpret();
  virtual ~Interpreter() {}

private:
  const char* source;
  d::DslValue eval(Ast*);
  void check(Ast*);
  d::DslFrame stack;
  d::SymbolTable* symbols;
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

