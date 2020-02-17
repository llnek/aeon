#include "analyzer.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon::analyzer {
namespace p= czlab::aeon::parser;
namespace l= czlab::aeon::lexer;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Analyzer::Analyzer(const char* src) {
  auto c= l::lexer(src, "");
  auto t= p::parser(c);
  check(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Analyzer::Analyzer(p::Ast* tree) {
  check(tree);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Analyzer::check(p::Ast* tree) {
  symbols= new p::SymbolTable("root",1);
  tree->visit(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
p::Symbol* Analyzer::lookup(const std::string& n, bool traverse) {
  return symbols->lookup(n, traverse ? false : true);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Analyzer::define(p::Symbol* s) {
  if (s) symbols->insert(s);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Analyzer::pushScope(const std::string& id) {
  auto s= new p::SymbolTable(id, symbols->scope_level+1, symbols);
  symbols=s;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
p::SymbolTable* Analyzer::popScope() {
  if (! symbols->enclosing) {
    return nullptr;
  } else {
    auto cur = symbols;
    symbols = cur->enclosing;
    cur->enclosing=nullptr;
    return cur;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

