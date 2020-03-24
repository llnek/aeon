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
namespace czlab::kirby {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readList(SExprParser*, int, stdstr);
d::DslValue readForm(SExprParser*);
d::DslValue readAtom(SExprParser*);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue macro_func(SExprParser* p, stdstr op) {
  if (auto f = readForm(p); f.isSome()) {
    return LIST_VAL2(SYMBOL_VAL(op), f);
  }
  RAISE(d::SyntaxError, "Bad form: %s.\n", "macro");
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
d::DslValue readAtom(SExprParser* p) {
  DEBUG("token = %d.\n", p->cur());
  switch (p->cur()) {
    case d::T_STRING:
      return STRING_VAL(p->eat()->getLiteralAsStr());
    break;
    case d::T_REAL:
      return FLOAT_VAL(p->eat()->getLiteralAsReal());
    break;
    case d::T_INTEGER:
      return INT_VAL(p->eat()->getLiteralAsInt());
    break;
    case T_KEYWORD:
      return KEYWORD_VAL(p->eat()->getLiteralAsStr());
    break;
    case d::T_IDENT:
      return SYMBOL_VAL(p->eat()->getLiteralAsStr());
    break;
    case T_TRUE:
      return (p->eat(), TRUE_VAL());
    break;
    case T_FALSE:
      return (p->eat(), FALSE_VAL());
    break;
    case T_NIL:
      return (p->eat(), NIL_VAL());
    break;
    case d::T_AT:
      return (p->eat(), macro_func(p, "deref"));
    break;
    case d::T_BACKTICK:
      return (p->eat(), macro_func(p, "syntax-quote"));
    break;
    case d::T_QUOTE:
      return (p->eat(), macro_func(p, "quote"));
    break;
    case T_SPLICE_UNQUOTE:
      return (p->eat(), macro_func(p, "splice-unquote"));
    break;
    case d::T_TILDA:
      return (p->eat(), macro_func(p, "unquote"));
    break;

    default:
      if (p->isCur(d::T_HAT)) {
        auto m = (p->eat(), readForm(p));
        auto v = readForm(p);
        if (m.isNull())
          RAISE(d::SyntaxError, "Bad form: %s.\n", "meta");
        if (v.isNull())
          RAISE(d::SyntaxError, "Bad form: %s.\n", "value");
        return LIST_VAL3(SYMBOL_VAL("with-meta"), v, m);
      }
      return SYMBOL_VAL(p->eat()->getLiteralAsStr());
    break;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readList(SExprParser* p, int ender, stdstr pairs) {
  auto m= p->rdr()->ctx().mark();
  auto found=0;
  VVec res;
  while (!p->isEof()) {
    if (p->isCur(ender)) { found = 1, p->eat(); break; }
    if (auto f= readForm(p); f.isSome()) { s__conj(res, f); }
  }
  if (!found) {
    RAISE(d::SyntaxError,
          "Unmatched %s near line %d(%d).\n",
          C_STR(pairs), m.first, m.second);
  }
  switch (ender) {
    case d::T_RBRACE: return MAP_VAL(res);
    case d::T_RBRACKET: return VEC_VAL(res);
    default: return LIST_VAL(res);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readForm(SExprParser* p) {
  switch (p->cur()) {
    case d::T_LPAREN:
      return (p->eat(), readList(p, d::T_RPAREN, "()"));
    case d::T_LBRACKET:
      return (p->eat(), readList(p, d::T_RBRACKET, "[]"));
    case d::T_LBRACE:
      return (p->eat(), readList(p, d::T_RBRACE,  "{}"));
    case d::T_RPAREN:
    case d::T_RBRACE:
    case d::T_RBRACKET:
      return d::DslValue();
    default:
      return readAtom(p);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<int,d::DslValue> SExprParser::parse() {
  d::DslValue ret;
  VVec out;
  while (!isEof()) {
    if (auto f= readForm(this); f.isSome()) {
      s__conj(out, f);
    }
  }
  int cnt= out.size();
  switch (cnt) {
    case 0: ret= NIL_VAL(); break;
    case 1: ret= out[0]; break;
    default: ret= LIST_VAL(out); break;
  }
  return s__pair(int,d::DslValue,cnt,ret);
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
d::DslToken SExprParser::token() const {
  return lexer->ctx().cur;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken SExprParser::eat() {
  auto t= lexer->ctx().cur;
  lexer->ctx().cur= lexer->getNextToken();
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken SExprParser::eat(int wanted) {
  auto t= token();
  if (t.ptr()->type() != wanted) {
    RAISE(d::SyntaxError,
          "Expected token %s, found token %s near line %d(%d).\n",
          C_STR(Token::typeToString(wanted)),
          C_STR(t->toString()),
          s__cast(Token,t.ptr())->impl().line,
          s__cast(Token,t.ptr())->impl().col);
  }
  lexer->ctx().cur= lexer->getNextToken();
  return t;
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


