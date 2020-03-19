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
void readList(SExprParser*, VVec&, int,  stdstr);
d::DslValue readForm(SExprParser*);
d::DslValue readAtom(SExprParser*);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue macro_func(SExprParser* p, stdstr op) {
  auto f = readForm(p);
  VVec x;
  if (!f.ptr()) {
    RAISE(d::SyntaxError, "Bad Form %s\n", "macro");
  }
  s__conj(x, d::DslValue(new LSymbol(op)));
  s__conj(x, f);
  return d::DslValue(new LList(x));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SExprParser::~SExprParser() {
  DEL_PTR(rdr);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SExprParser::SExprParser(const char* src) {
  rdr = new Reader(src);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readAtom(SExprParser* p) {
  d::DslToken t;
  stdstr s;
  ::printf("token = %d\n", p->cur());
  switch (p->cur()) {
    case d::T_STRING:
      t= p->eat();
      s=t->getLiteralAsStr();
      return d::DslValue(new LString(s));
    break;
    case d::T_REAL:
      t= p->eat();
      return d::DslValue(new LFloat(t->getLiteralAsReal()));
    break;
    case d::T_INTEGER:
      t= p->eat();
      return d::DslValue(new LInt(t->getLiteralAsInt()));
    break;
    case T_KEYWORD:
      t= p->eat();
      s=t->getLiteralAsStr();
      return d::DslValue(new LKeyword(s));
    break;
    case d::T_IDENT:
      t= p->eat();
      s=t->getLiteralAsStr();
      return d::DslValue(new LSymbol(s));
    break;
    case T_FALSE:
      p->eat();
      return false_value();
    break;
    case T_TRUE:
      p->eat();
      return true_value();
    break;
    case T_NIL:
      p->eat();
      return nil_value();
    break;
    case d::T_AT:
      p->eat();
      return macro_func(p, "deref");
    break;
    case d::T_BACKTICK:
      p->eat();
      return macro_func(p, "backtick");
    break;
    case d::T_QUOTE:
      p->eat();
      return macro_func(p, "quote");
    break;
    case T_UNQUOTE_SPLICE:
      p->eat();
      return macro_func(p, "unquote-splice");
    break;
    case d::T_TILDA:
      p->eat();
      return macro_func(p, "unquote");
    break;
  }

  if (p->isCur(d::T_HAT)) {
    s= "with-meta";
    p->eat();
    d::DslValue m = readForm(p);
    d::DslValue v = readForm(p);
    if (!m.ptr()) {
      RAISE(d::SyntaxError, "Bad form %s\n", "meta");
    }
    if (!v.ptr()) {
      RAISE(d::SyntaxError, "Bad form %s\n", "value");
    }
    VVec x { d::DslValue(new LSymbol(s)), v, m };
    return d::DslValue(new LList(x));
  }

  t= p->eat();
  s=t->getLiteralAsStr();
  return d::DslValue(new LSymbol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void readList(SExprParser* p, VVec& res, int ender, stdstr pairs) {
  auto m= p->rdr->ctx.mark();
  auto found=false;
  while (!p->isEof()) {
    if (p->isCur(ender)) {
      found = true;
      p->eat();
      break;
    }
    auto f= readForm(p);
    if (f.ptr()) {
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
      return d::DslValue(new LList(vec));
    case d::T_LBRACKET:
      p->eat();
      readList(p, vec, d::T_RBRACKET, "[]");
      return d::DslValue(new LVec(vec));
    case d::T_LBRACE:
      p->eat();
      readList(p, vec, d::T_RBRACE,  "{}");
      return d::DslValue(new LHash(vec));
    case d::T_RPAREN:
    case d::T_RBRACE:
    case d::T_RBRACKET:
      return d::DslValue();
    default:
      return readAtom(p);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue SExprParser::parse() {
  VVec out;
  while (!isEof()) {
    auto f= readForm(this);
    if (f.ptr()) { s__conj(out, f); }
  }
  if (out.size() == 0) {
    return nil_value();
  }
  if (out.size() == 1) {
    return out[0];
  } else {
    return d::DslValue(new LList(out));
  }
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


