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

#include <typeinfo>
#include "dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::dsl {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a=czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> I_TOKENS {
  {T_INT, "long"},
  {T_REAL, "double"},
  {T_STRING, "string"},
  {T_COMMA, ","},
  {T_COLON, ":"},
  {T_LPAREN, "("},
  {T_RPAREN, ")"},
  {T_LBRACE, "{"},
  {T_RBRACE, "}"},
  {T_EQ, "="},
  {T_MINUS, "-"},
  {T_PLUS, "+"},
  {T_MULT, "*"},
  {T_DIV, "/"},
  {T_GT, ">"},
  {T_LT, "<"},
  {T_SEMI, ";"},
  {T_DOT, "."},
  {T_TILDA, "~"},
  {T_BACKTICK, "`"},
  {T_QUOTE, "'"},
  {T_DQUOTE, "\""},
  {T_BANG, "!"},
  {T_AMPER, "&"},
  {T_AT, "@"},
  {T_PERCENT,"%"},
  {T_QMARK, "?"},
  {T_HAT, "^"},
  {T_LBRACKET, "["},
  {T_RBRACKET, "]"},
  {T_EOF, "<EOF>"},
  {T_ETHEREAL, "<?>"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
auto S_TOKENS=a::map_reflect(I_TOKENS);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
const std::map<stdstr,int>& getStrTokens() { return S_TOKENS; }
const std::map<int,stdstr>& getIntTokens() { return I_TOKENS; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool is_same(const Data* x, const Data* y) {
  return typeid(*x) == typeid(*y);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool is_same(DValue x, const Data* y) {
  if (auto p= x.get(); p)
    return typeid(*p) == typeid(*y); else return false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEqual(int wanted, int got, cstdstr& fn) {
  if (wanted != got)
    RAISE(BadArity,
          "%s wanted %d args, got %d.", C_STR(fn), wanted, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preMax(int max, int got, cstdstr& fn) {
  if (got > max)
    RAISE(BadArity,
          "%s wanted at most %d args, got %d.", C_STR(fn), max, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preMin(int min, int got, cstdstr& fn) {
  if (got < min)
    RAISE(BadArity,
          "%s wanted at least %d args, got %d.", C_STR(fn), min, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preNonZero(int c, cstdstr& fn) {
  if (c == 0)
    RAISE(BadArity,
          "%s wanted some args, got %d.", C_STR(fn), c);
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEven(int c, cstdstr& fn) {
  if (!a::is_even(c))
    RAISE(BadArity,
          "%s wanted even args, got %d.", C_STR(fn), c);
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr unescape(cstdstr& src) {
  auto len = src.length();
  auto ch= '\n';
  stdstr res;
  if (len == 0 ||
      !(src[0]=='"' && src[len-1]=='"')) { return src; }
  // skip 1st and last => no dqoutes
  --len;
  for (auto i = 1; i < len; ++i) {
    ch = src[i];
    if (ch == '\\') {
      i += 1;
      res += a::unescape_char(src[i]);
    } else {
      res += ch;
    }
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr escape(cstdstr& src) {
  auto len = src.length();
  stdstr res;
  if (len == 0 ||
      (src[0]=='"' && src[len-1]=='"')) { return src; }
  for (auto i = 0; i < len; ++i) {
    res += a::escape_char(src[i]);
  }
  return "\"" + res + "\"";
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar peek(Context& ctx) {
  return ctx.eof ? '\0' : ctx.src[ctx.pos];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar pop(Context& ctx) {
  auto ch= peek(ctx);
  return ctx.eof ? '\0' : (advance(ctx), ch);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool peekPattern(Context& ctx, cstdstr& pattern) {
  auto pos=0;
  for (auto& x : pattern) {
    if (peekAhead(ctx,pos)!=x)
    break; else ++pos;
  }
  return pos==pattern.size();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Peek into the buffer and see what's ahead.
Tchar peekAhead(Context& ctx, int offset) {
  auto nx = ctx.pos + offset;
  return nx >= 0 && nx < ctx.len ? ctx.src[nx] : '\0';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr pr_addr(Addr m) {
  Tchar buf[1024];
  ::sprintf(buf, "line: %d, col: %d", _1(m), _2(m));
  return stdstr(buf);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool forward(Context& ctx) {
  // move up one char, handling newline.
  if (ctx.eof) { return false; }

  if (peek(ctx) == '\n') {
    ++ctx.line;
    ctx.col=0; }

  ++ctx.pos;

  if (ctx.pos >= ctx.len) {
    ctx.eof=true; }
  else {
    ++ctx.col; }

  return !ctx.eof;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool advance(Context& ctx, int steps) {
  for (auto i=0; i < steps; ++i) { forward(ctx); }
  return !ctx.eof;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Addr mark_advance(Context& ctx, int steps) {
  auto m=ctx.mark();
  return advance(ctx, steps), m;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skipWhitespace(Context& ctx) {
  while (!ctx.eof &&
         ::isspace(peek(ctx))) advance(ctx);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<stdstr,Addr> digits(Context& ctx) {
  // grab a sequence of digits
  auto m= ctx.mark();
  stdstr res;
  while (!ctx.eof &&
         ::isdigit(peek(ctx))) { res += peek(ctx); advance(ctx); }
  return s__pair(stdstr,Addr,res,m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<stdstr,Addr> numeric(Context& ctx) {
  bool dot = peek(ctx)=='.' ; // leading dot
  if (dot)
  { advance(ctx);
    if (ctx.eof || !::isdigit(peek(ctx)))
      E_SYNTAX("Bad number near line %d, col %d.", ctx.line, ctx.col); }
  auto res = digits(ctx);
  auto s= _1(res);
  if (!dot &&
      !ctx.eof &&
      peek(ctx) == '.') {
    s += peek(ctx);
    advance(ctx);
    s += _1(digits(ctx));
  }
  return s__pair(stdstr,Addr,s, _2(res));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<stdstr,Addr> str(Context& ctx) {
  auto m= ctx.mark();
  stdstr res;
  auto ch= '\0';
  if (!ctx.eof &&
      peek(ctx) == '"') {
    advance(ctx);
    while (!ctx.eof) {
      ch= peek(ctx);
      if (ch == '"')
      break;
      if (ch == '\\') {
        if (!advance(ctx)) {
          E_SYNTAX(
              "Bad escaped char %c near line %d, col %d.", ch, _1(m), _2(m)); }
        ch=a::unescape_char(peek(ctx)); }
      res += ch;
      advance(ctx); }

    if (ctx.eof || ch != '"') {
      E_SYNTAX("Bad string value, missing \" near line %d, col %d.", _1(m), _2(m)); }

    // good, got the end dquote
    advance(ctx);
  }

  return s__pair(stdstr,Addr,res,m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<stdstr,Addr> identifier(Context& ctx, IdPredicate pred) {
  auto m= ctx.mark();
  stdstr res;

  while (!ctx.eof) {
    auto ch=peek(ctx);
    if (res.empty()) {
      if (pred(ch,1)) {
        res += ch;
        advance(ctx); } else break; }
    else {
      if (pred(ch,0)) {
        res += ch;
        advance(ctx); } else break; } }

  return s__pair(stdstr,Addr,res,m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Context::Context() {
  S_NIL(src); len=0; line=1; col=1; pos=0; eof=0; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DFrame Frame::make(cstdstr& n, DFrame outer) {
  return WRAP_ENV(Frame, n, outer);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DFrame Frame::make(cstdstr& n) {
  return WRAP_ENV(Frame,n);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(cstdstr& n, DFrame outer) : _name(n) { prev=outer; }
Frame::Frame(cstdstr& n) : _name(n) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Frame::pr_str() const {
  stdstr b(40, '=');
  stdstr z(40, '+');
  stdstr out;
  stdstr bits;

  out += b;
  out += "\n";
  out += "frame: " + _name + " => ";

  for (auto i= slots.begin(), e= slots.end(); i != e; ++i) {
    auto v= i->second;
    auto vs= v ? v->pr_str(1) : stdstr("null");
    if (!bits.empty()) bits += ", ";
    bits += i->first + "=" + vs;
  }
  if (!bits.empty()) bits += "\n";

  return out + bits + z;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::set<stdstr> Frame::keys() const {
  std::set<stdstr> out;
  for (auto &x : slots) {
    out.insert(x.first);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DValue Frame::get(cstdstr& key) const {

  auto x= slots.find(key);
  auto r= x != slots.end()
          ? x->second
          : (prev ? prev->get(key) : DVAL_NIL);

  DEBUG("frame:get %s <- %s\n",
        C_STR(key), C_STR(r->pr_str()));

  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DValue Frame::setEx(cstdstr& key, DValue v) {

  DEBUG("frame:setEx %s -> %s\n", C_STR(key), C_STR(v->pr_str(1)));

  if (s__contains(slots,key)) {
    return (slots[key]=v), v;
  } else {
    return prev ? prev->setEx(key,v) : DVAL_NIL;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DValue Frame::set(cstdstr& key, DValue v) {
  DEBUG("frame:set %s -> %s\n", C_STR(key), C_STR(v->pr_str(1)));
  return (slots[key]=v), v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Frame::contains(cstdstr& key) const {
  return slots.find(key) != slots.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DFrame Frame::search(cstdstr& key, DFrame from) {
  ASSERT1(from);
  return from->contains(key) ? from :
         (from->prev ? search(key, from->prev) : DENV_NIL);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DFrame Frame::getRoot(DFrame from) {
  ASSERT1(from);
  return from->prev ? getRoot(from->prev) : from;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DFrame Frame::getOuter() const { return prev; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DTable Table::make(cstdstr& n, const SymbolMap& root) {
  return DTable(new Table(n,root));
}
DTable Table::make(cstdstr& n, DTable outer) {
  return DTable(new Table(n, outer));
}
DTable Table::make(cstdstr& n) {
  return DTable(new Table(n));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(cstdstr& n, DTable outer) : Table(n) { enclosing=outer; }
Table::Table(cstdstr& n, const SymbolMap& root) : Table(n) {
  for (auto& x : root) { symbols[x.first]=x.second; }
}
Table::Table(cstdstr& n) : _name(n) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Table::insert(DSymbol s) {
  if (s)
    symbols[s->name()] = s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DSymbol Table::search(cstdstr& name) const {
  if (auto s = symbols.find(name); s != symbols.end()) {
    return s->second;
  } else {
    return enclosing ? enclosing->search(name) :  DSYM_NIL;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DSymbol Table::find(cstdstr& name) const {
  if (auto s = symbols.find(name); s != symbols.end()) {
    return s->second;
  } else {
    return DSYM_NIL;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool String::equals(DValue rhs) const {
  return is_same(rhs, this) &&
         DCAST(String,rhs)->value == value;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int String::compare(DValue rhs) const {
  if (!is_same(rhs, this)) {
    return pr_str().compare(rhs->pr_str()); }
  else {
    return value.compare(DCAST(String,rhs)->value); }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Number::match(const Number* rhs) const {
  return (isInt() && rhs->isInt())
    ? getInt() == rhs->getInt()
    : a::fuzzy_equals(getFloat(), rhs->getFloat());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Number::equals(DValue rhs) const {
  return is_same(rhs, this) &&
         match(DCAST(Number, rhs));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Number::compare(DValue rhs) const {
  if (!is_same(rhs, this)) {
    return pr_str().compare(rhs->pr_str());
  } else {
    auto p= DCAST(Number, rhs);
    return match(p) ? 0 : (getFloat() > p->getFloat() ? 1 : -1);
  }
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


