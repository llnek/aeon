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
void readList(SExprParser*, ValueVec&, int);
d::DslValue readForm(SExprParser*);
d::DslValue readAtom(SExprParser*);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue macro_func(SExprParser* p, stdstr op) {
  ValueVec x;
  s__conj(x, d::DslValue(new LSymbol(op)));
  s__conj(x, readForm(p));
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
      return false_value();
    break;
    case T_TRUE:
      return true_value();
    break;
    case T_NIL:
      return nil_value();
    break;
    case d::T_AT:
      return macro_func(p, "deref");
    break;
    case d::T_BACKTICK:
      return macro_func(p, "backtick");
    break;
    case d::T_QUOTE:
      return macro_func(p, "quote");
    break;
    case T_UNQUOTE_SPLICE:
      return macro_func(p, "unquote-splice");
    break;
    case d::T_TILDA:
      return macro_func(p, "unquote");
    break;
  }

  if (p->isCur(d::T_HAT)) {
    s= "with-meta";
    p->eat();
    d::DslValue m = readForm(p);
    d::DslValue v = readForm(p);
    ValueVec x { d::DslValue(new LSymbol(s)), v, m };
    return d::DslValue(new LList(x));
  }

  t= p->eat();
  s=t->getLiteralAsStr();
  ::printf("sdfdsfsd = %s\n", s.c_str());
  return d::DslValue(new LSymbol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void readList(SExprParser* p, ValueVec& res, int ender) {
  while (!p->rdr->ctx.eof) {
    if (p->isCur(ender)) {
      p->eat();
      break;
    }
    s__conj(res, readForm(p));
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readForm(SExprParser* p) {
  ValueVec vec;
  switch (p->cur()) {
    case d::T_LPAREN:
      p->eat();
      readList(p, vec, d::T_RPAREN);
      return d::DslValue(new LList(vec));
    case d::T_LBRACKET:
      p->eat();
      readList(p, vec, d::T_RBRACKET);
      return d::DslValue(new LVec(vec));
    case d::T_LBRACE:
      p->eat();
      readList(p, vec, d::T_RBRACE);
      return d::DslValue(new LHash(vec));
    default:
      return readAtom(p);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue SExprParser::parse() {
  ValueVec out;
  while (!rdr->ctx.eof) {
    s__conj(out, readForm(this));
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


