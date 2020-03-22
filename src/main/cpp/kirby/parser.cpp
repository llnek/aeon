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
void readList(SExprParser*, VVec&, int, stdstr);
d::DslValue readForm(SExprParser*);
d::DslValue readAtom(SExprParser*);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue macro_func(SExprParser* p, stdstr op) {
  if (auto f = readForm(p); f.isSome()) {
    VVec x{ symbol_value(op), f };
    return list_value(VSlice(x));
  }
  //else
  RAISE(d::SyntaxError, "Bad Form %s\n", "macro");
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SExprParser::~SExprParser() {
  DEL_PTR(rdr);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SExprParser::SExprParser(const Tchar* src) {
  rdr = new Reader(src);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readAtom(SExprParser* p) {
  DEBUG("token = %d\n", p->cur());
  d::DslToken t;
  stdstr s;
  switch (p->cur()) {
    case d::T_STRING:
      return string_value(p->eat()->getLiteralAsStr());
    break;
    case d::T_REAL:
      return float_value(p->eat()->getLiteralAsReal());
    break;
    case d::T_INTEGER:
      return int_value(p->eat()->getLiteralAsInt());
    break;
    case T_KEYWORD:
      return keyword_value(p->eat()->getLiteralAsStr());
    break;
    case d::T_IDENT:
      return symbol_value(p->eat()->getLiteralAsStr());
    break;
    case T_TRUE:
      return (p->eat(), true_value());
    break;
    case T_FALSE:
      return (p->eat(), false_value());
    break;
    case T_NIL:
      return (p->eat(), nil_value());
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
    case T_UNQUOTE_SPLICE:
      return (p->eat(), macro_func(p, "splice-unquote"));
    break;
    case d::T_TILDA:
      return (p->eat(), macro_func(p, "unquote"));
    break;
  }

  if (p->isCur(d::T_HAT)) {
    s= "with-meta";
    p->eat();
    d::DslValue m = readForm(p);
    d::DslValue v = readForm(p);
    if (m.isNull()) {
      RAISE(d::SyntaxError,
          "Bad form %s\n", "meta");
    }
    if (v.isNull()) {
      RAISE(d::SyntaxError,
          "Bad form %s\n", "value");
    }
    VVec x { symbol_value(s), v, m };
    return list_value(VSlice(x));
  }

  return symbol_value(p->eat()->getLiteralAsStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void readList(SExprParser* p, VVec& res, int ender, stdstr pairs) {
  auto m= p->rdr->ctx.mark();
  auto found=0;
  while (!p->isEof()) {
    if (p->isCur(ender)) {
      found = 1;
      p->eat();
      break;
    }
    if (auto f= readForm(p); f.isSome()) {
      s__conj(res, f);
    }
  }
  if (!found) {
    RAISE(d::SyntaxError,
        "Unmatched %s near line %d(%d).\n",
        pairs.c_str(), m.first, m.second);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readForm(SExprParser* p) {
  VVec vec;
  switch (p->cur()) {
    case d::T_LPAREN:
      p->eat();
      readList(p, vec, d::T_RPAREN, "()");
      return list_value(VSlice(vec));
    case d::T_LBRACKET:
      p->eat();
      readList(p, vec, d::T_RBRACKET, "[]");
      return vector_value(VSlice(vec));
    case d::T_LBRACE:
      p->eat();
      readList(p, vec, d::T_RBRACE,  "{}");
      return map_value(VSlice(vec));
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
    case 0: ret= nil_value(); break;
    case 1: ret= out[0]; break;
    default: ret= list_value(out); break;
  }
  return s__pair(int,d::DslValue,cnt,ret);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int SExprParser::cur() {
  return rdr->ctx.cur->type();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char SExprParser::peek() {
  return d::peek(rdr->ctx);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SExprParser::isEof() {
  return rdr->ctx.cur->type() == d::T_EOF;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool SExprParser::isCur(int type) {
  return rdr->ctx.cur->type() == type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken SExprParser::token() {
  return rdr->ctx.cur;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken SExprParser::eat() {
  auto t= rdr->ctx.cur;
  rdr->ctx.cur=rdr->getNextToken();
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken SExprParser::eat(int wanted) {
  auto t= token();
  if (t.ptr()->type() != wanted) {
    RAISE(d::SyntaxError,
        "Expected token %s, found token %s near line %d(%d).\n",
        Token::typeToString(wanted).c_str(),
        t->toString().c_str(),
        s__cast(Token,t.ptr())->impl.line,
        s__cast(Token,t.ptr())->impl.col);
  }
  rdr->ctx.cur= rdr->getNextToken();
  return t;
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


