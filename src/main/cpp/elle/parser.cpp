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

#include <regex>
#include <ctime>
#include "parser.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::elle {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue readList(SExprParser*, int, int, cstdstr&);
d::DValue readForm(SExprParser*);
d::DValue readAtom(SExprParser*);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr gensym(cstdstr& prefix) {
  static int seed=0;
  stdstr out;
  int x;
  srand(time(NULL));
  /*
  for (auto i = 0; i < 3; ++i) {
    x= rand() % 26;
    out += (char) (((int)'a') + x);
    x= rand() % 26;
    out += (char) (((int)'A') + x);
  }
  */
  for (auto i = 0; i < 6; ++i) {
    x= rand() % 10;
    out += (char) (((int)'0') + x);
  }

  ++seed;
  return prefix + out + N_STR(seed);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue macro_func(SExprParser* p, stdstr op) {
  auto _A= p->eat()->addr();
  if (auto f = readForm(p); f) {
    d::ValVec out{SSymbol::make(_A,op), f};
    return makeList(_A, out);
  }
  E_SYNTAX("Bad macro near %s", d::pr_addr(_A).c_str());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SExprParser::~SExprParser() {
  DEL_PTR(lexer);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SExprParser::SExprParser(const Tchar* src) {
  lexer = new Reader(src);
  rdr(); // to get rid of warnings, stupid and weird
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue readAtom(SExprParser* p) {
  switch (p->cur()) {
  case d::T_STRING:
    return SString::make(p->eat());
  case d::T_REAL:
  case d::T_INT:
    return SNumber::make(p->eat());
  case d::T_IDENT:
    return SSymbol::make(p->eat());
  case T_TRUE:
    return STrue::make();
  case T_FALSE:
    return SFalse::make();
  case T_SYNTAX_QUOTE:
    return macro_func(p, "syntax-quote");
  case T_QUOTE:
    return macro_func(p, "quote");
  case T_SPLICE_UNQUOTE:
    return macro_func(p, "splice-unquote");
  case T_UNQUOTE:
    return macro_func(p, "unquote");
  default:
    return SSymbol::make(p->eat());
  }
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue readList(SExprParser* p, int cur, int ender, cstdstr& pairs) {
  auto _A= p->eat(cur)->addr();
  d::DToken dot;
  bool found=0;
  d::ValVec res;

  while (!p->isEof()) {

    if (p->isCur(ender))
    { found = 1, p->eat(); break; }

    if (p->isCur(d::T_DOT)) {
      if (res.empty())
        E_SYNTAX("Bad pair near %s",
                 d::pr_addr(_A).c_str());
      dot=p->eat(); // a dotted pair
    }

    auto f= readForm(p);
    //
    if (dot && !f)
      E_SYNTAX("Bad dotted pair near %s",
               d::pr_addr(dot->addr()).c_str());

    if (f)
      s__conj(res, f);

    if (dot)
    { found=1;
      p->eat(ender); break; }
  }

  if (!found)
    E_SYNTAX("Unmatched %s near %s",
             pairs.c_str(), d::pr_addr(_A).c_str());
  return pairs=="#()"
         ? SVec::make(_A,res)
         : (dot ? makePair(_A,res) : makeList(_A,res));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue readForm(SExprParser* p) {
  //std::cout << "token= " << p->token()->pr_str() << "\n";
  switch (p->cur()) {
  case T_LIST:
    return readList(p, T_LIST, d::T_RPAREN, "()");
  case T_VEC:
    return readList(p, T_VEC, d::T_RPAREN, "#()");
  case d::T_RPAREN:
    return DVAL_NIL;
  default:
    return readAtom(p);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<int,d::DValue> SExprParser::parse() {
  d::DValue ret;
  d::ValVec out;
  while (!isEof())
  { auto f= readForm(this);
    if (f) s__conj(out, f); }
  int cnt= out.size();
  switch (cnt) {
  case 0: ret= DVAL_NIL; break;
  case 1: ret= out[0]; break;
  default: ret= SVec::make(out); break;
  }
  return s__pair(int,d::DValue,cnt,ret);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SExprParser::cur() const {
  return lexer->ctx().cur->type();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar SExprParser::peek() const {
  return d::peek(lexer->ctx());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SExprParser::isEof() const {
  return lexer->ctx().cur->type() == d::T_EOF;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SExprParser::isCur(int type) const {
  return lexer->ctx().cur->type() == type;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken SExprParser::token() const {
  return lexer->ctx().cur;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken SExprParser::eat() {
  auto t= lexer->ctx().cur;
  lexer->ctx().cur= lexer->getNextToken();
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken SExprParser::eat(int wanted) {
  auto t= token();
  if (t->type() != wanted) {
    auto i= t->addr();
    RAISE(d::SyntaxError,
          "Expected token %s, found token %s near line %d(%d).\n",
          C_STR(typeToString(wanted)),
          C_STR(t->pr_str()),
          _1(i),
          _2(i));
  }
  lexer->ctx().cur= lexer->getNextToken();
  return t;
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


