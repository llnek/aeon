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
namespace czlab::otto {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readList(SExprParser*, int, stdstr);
d::DslValue readForm(SExprParser*);
d::DslValue readAtom(SExprParser*);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr gensym(const stdstr& prefix) {
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
  return prefix + out + std::to_string(seed);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue macro_func(SExprParser* p, stdstr op) {
  if (auto f = readForm(p); f) {
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
      return NUMBER_VAL(p->eat()->getLiteralAsReal());
    break;
    case d::T_INTEGER:
      return NUMBER_VAL(p->eat()->getLiteralAsInt());
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
        if (!m)
          RAISE(d::SyntaxError, "Bad form: %s.\n", "meta");
        if (!v)
          RAISE(d::SyntaxError, "Bad form: %s.\n", "value");
        return LIST_VAL3(SYMBOL_VAL("with-meta"), v, m);
      }
      return SYMBOL_VAL(p->eat()->getLiteralAsStr());
    break;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void checkForAnonArg(LSymbol* sym, const stdstr& gensym, int& high, bool& varArgs) {
  static std::regex ANON_ARG("%(\\d+)");
  auto s = sym->impl();
  if (s[0] != '%') { return; }
  auto vargs = (s == "%&");
  auto v1= (s== "%");
  stdstr out;
  if (!vargs && !v1) {
    std::smatch m;
    if (!std::regex_match(s, m, ANON_ARG)) { return;}
    else ASSERT1(m.size()==2);
    out=m[1];
  }
  int pos=1;
  if (!out.empty())
    pos= ::atoi(out.c_str());
  ASSERT1(pos >= 1);

  if (vargs) {
    sym->rename(gensym + "_vargs");
    varArgs=true;
  } else {
    sym->rename(gensym + "_" + std::to_string(pos));
    if (pos > high) { high= pos; }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void XXcheckForAnonArg(LSymbol* sym, const stdstr& gensym, int& high, bool& varArgs) {
  auto s = sym->impl();
  if (s[0] != '%') { return; }
  auto vargs = (s == "%&");
  stdstr out;
  if (!vargs) {
    for (auto i= s.begin()+1,e=s.end(); i != e; ++i) {
      if (! ::isdigit(*i)) { return; }
      out += *i;
    }
  }
  int pos=1;
  if (!out.empty())
    pos= ::atoi(out.c_str());
  ASSERT1(pos >=1);

  if (vargs) {
    sym->rename(gensym + "_vargs");
    varArgs=true;
  } else {
    sym->rename(gensym + "_" + std::to_string(pos));
    if (pos > high) { high= pos; }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void scanAnonFn(LSeqable* seq, const stdstr& gensym, int& high, bool& varArgs) {
  for (int i=0, e=seq->count(); i < e; ++i) {
    auto v= seq->nth(i);
    if (auto x= cast_seqable(v);
        X_NIL(x) && E_NIL(cast_string(v))) {
      scanAnonFn(x, gensym, high, varArgs);
    }
    else
    if (auto s = cast_symbol(v); X_NIL(s)) {
      checkForAnonArg(s, gensym, high, varArgs);
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readAnonFn(d::ValVec& res) {
  auto gs = gensym("G__");
  bool varArgs=false;
  int high=0;
  auto form = LIST_VAL(res);
  scanAnonFn(cast_list(form,1), gs, high, varArgs);
  d::ValVec out { SYMBOL_VAL("fn") };
  d::ValVec args;
  for (int i = 1; i <= high; ++i) {
    s__conj(args, SYMBOL_VAL(gs + "_" + std::to_string(i)));
  }
  if (varArgs) {
    s__conj(args, SYMBOL_VAL("&"));
    s__conj(args, SYMBOL_VAL(gs + "_vargs"));
  }
  s__conj(out, VEC_VAL(args));
  s__conj(out, form);
  return LIST_VAL(out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue readList(SExprParser* p, int ender, stdstr pairs) {
  auto m= p->rdr()->ctx().mark();
  auto found=0;
  d::ValVec res;
  while (!p->isEof()) {
    if (p->isCur(ender)) { found = 1, p->eat(); break; }
    if (auto f= readForm(p); f) { s__conj(res, f); }
  }
  if (!found) {
    RAISE(d::SyntaxError,
          "Unmatched %s near line %d(%d).\n",
          C_STR(pairs), m.first, m.second);
  }
  switch (ender) {
    case d::T_RBRACKET:
      return VEC_VAL(res);
    case d::T_RBRACE:
      return pairs=="#{}" ? SET_VAL(res) : MAP_VAL(res);
    default:
      return pairs=="#()" ? readAnonFn(res) : LIST_VAL(res);
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
    case T_ANONFN:
      return (p->eat(), readList(p, d::T_RPAREN, "#()"));
    case T_SET:
      return (p->eat(), readList(p, d::T_RBRACE, "#{}"));
    case d::T_RPAREN:
    case d::T_RBRACE:
    case d::T_RBRACKET:
      return NULL;
    default:
      return readAtom(p);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<int,d::DslValue> SExprParser::parse() {
  d::DslValue ret;
  d::ValVec out;
  while (!isEof()) {
    if (auto f= readForm(this); f) {
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
  if (t->type() != wanted) {
    RAISE(d::SyntaxError,
          "Expected token %s, found token %s near line %d(%d).\n",
          C_STR(Token::typeToString(wanted)),
          C_STR(t->pr_str()),
          t->srcInfo().first,
          t->srcInfo().second);
  }
  lexer->ctx().cur= lexer->getNextToken();
  return t;
}




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


