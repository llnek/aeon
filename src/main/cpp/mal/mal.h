#pragma once

#include "../dsl/dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::lisp {
namespace d=czlab::dsl;

typedef std::vector<d::ExprValue> ValueVec;
typedef std::set<d::ExprValue> ValueSet;
typedef ValueVec::iterator VVecIter;
typedef ValueSet::iterator VSetIter;


void repl(const std::string& s);








//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

