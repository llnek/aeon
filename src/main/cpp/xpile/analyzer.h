#pragma once
#include "parser.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon::analyzer {
namespace p= czlab::aeon::parser;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Analyzer : public p::Analyzer {
  Analyzer(const char* src);
  Analyzer(p::Ast*);
  ~Analyzer() {}
  void pushScope(const std::string&);
  p::SymbolTable* popScope();
  p::Symbol* lookup(const std::string&, bool traverse=true);
  void define(p::Symbol*);
private:
  void check(p::Ast*);
  p::SymbolTable* symbols;
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

