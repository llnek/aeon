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
  if (auto f = readForm(p); f)
    return LList::make(_A, SYMBOL_VAL(op), f);
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
    return LString::make(p->eat());
  case d::T_REAL:
  case d::T_INT:
    return LNumber::make(p->eat());
  case T_KEYWORD:
    return LKeyword::make(p->eat());
  case d::T_IDENT:
    return LSymbol::make(p->eat());
  case T_TRUE:
    return LTrue::make(p->eat()->addr());
  case T_FALSE:
    return LFalse::make(p->eat()->addr());
  case T_NIL:
    return LNil::make(p->eat()->addr());
  case d::T_AT:
    return macro_func(p, "deref");
  case d::T_BACKTICK:
    return macro_func(p, "syntax-quote");
  case d::T_QUOTE:
    return macro_func(p, "quote");
  case T_SPLICE_UNQUOTE:
    return macro_func(p, "splice-unquote");
  case d::T_TILDA:
    return macro_func(p, "unquote");
  default:
    if (!p->isCur(d::T_HAT))
      return LSymbol::make(p->eat());
    else
    { auto _A=p->eat()->addr();
      auto m = readForm(p);
      auto v = readForm(p);
      if (!v)
        E_SYNTAX("Bad form near %s", d::pr_addr(_A).c_str());
      if (!m)
        E_SYNTAX("Bad meta form near %s", d::pr_addr(_A).c_str());
      //
      return LList::make(_A, SYMBOL_VAL("with-meta"), v, m); }
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
        X_NIL(x) && E_NIL(vcast<LString>(v))) {
      scanAnonFn(x, gensym, high, varArgs);
    }
    else
    if (auto s = vcast<LSymbol>(v); X_NIL(s)) {
      checkForAnonArg(s, gensym, high, varArgs);
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue readAnonFn(d::Addr _A, d::ValVec& res) {
  auto gs = gensym("G__");
  bool varArgs=0;
  int high=0;
  auto form = LIST_VAL(res);
  scanAnonFn(vcast<LList>(form), gs, high, varArgs);
  d::ValVec out { SYMBOL_VAL("fn") };
  d::ValVec args;
  for (int i=1;i<=high;++i)
    s__conj(args, SYMBOL_VAL(gs + "_" + N_STR(i)));
  if (varArgs)
  { s__conj(args, SYMBOL_VAL("&"));
    s__conj(args, SYMBOL_VAL(gs + "_vargs")); }
  s__conj(out, VEC_VAL(args));
  s__conj(out, form);
  return LList::make(_A,out);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue readList(SExprParser* p, int cur, int ender, cstdstr& pairs) {
  auto _A= p->eat(cur)->addr();
  bool found=0;
  d::ValVec res;

  while (!p->isEof())
  { if (p->isCur(ender))
    { found = 1, p->eat(); break; }
    if (auto f= readForm(p); f) s__conj(res, f); }

  if (!found)
    E_SYNTAX("Unmatched %s near %s",
             pairs.c_str(), d::pr_addr(_A).c_str());

  switch (ender) {
  case d::T_RBRACKET:
    return LVec::make(_A,res);
  case d::T_RBRACE:
    return pairs=="#{}"
           ? LSet::make(_A, res)
           : LHash::make(_A, res);
  default:
    return pairs=="#()"
           ? readAnonFn(_A,res) : LList::make(_A,res);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DValue readForm(SExprParser* p) {
  switch (p->cur()) {
    case d::T_LPAREN:
      return readList(p, d::T_LPAREN, d::T_RPAREN, "()");
    case d::T_LBRACKET:
      return readList(p, d::T_LBRACKET, d::T_RBRACKET, "[]");
    case d::T_LBRACE:
      return readList(p, d::T_LBRACE, d::T_RBRACE,  "{}");
    case T_ANONFN:
      return readList(p, T_ANONFN, d::T_RPAREN, "#()");
    case T_SET:
      return readList(p, T_SET, d::T_RBRACE, "#{}");
    case d::T_RPAREN:
    case d::T_RBRACE:
    case d::T_RBRACKET:
      return DVAL_NIL;
    default:
      return readAtom(p);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<int,d::DValue> SExprParser::parse() {
  d::DValue ret;
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


