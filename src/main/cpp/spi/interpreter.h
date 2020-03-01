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
  void setValue(const std::string&, const d::ExprValue&, bool localOnly);
  d::ExprValue getValue(const std::string&);
  d::Frame* push(const std::string&);
  d::Frame* pop();
  d::Frame* peek();
  //analyzer
  void pushScope(const std::string&);
  d::SymbolTable* popScope();
  d::Symbol* lookup(const std::string&, bool traverse);
  void define(d::Symbol*);

  Interpreter(const char* src);
  d::ExprValue interpret();
  virtual ~Interpreter() {}

private:
  const char* source;
  d::ExprValue eval(Ast*);
  void check(Ast*);
  d::CallStack stack;
  d::SymbolTable* symbols;
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

