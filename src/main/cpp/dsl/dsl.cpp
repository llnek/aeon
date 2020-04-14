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

#include "dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::dsl {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a=czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Data* nothing() { return new Nothing(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> TOKENS {
  {T_INTEGER, "long"},
  {T_REAL, "double"},
  {T_STRING, "string"},
  {T_IDENT, "id"},
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
  {T_QUOTE,"'"},
  {T_DQUOTE,"\""},
  {T_SEMI, ";"},
  {T_TILDA, "~"},
  {T_BACKTICK, "`"},
  {T_BANG, "!"},
  {T_AMPER, "&"},
  {T_AT, "@"},
  {T_PERCENT, "%"},
  {T_QMARK, "?"},
  {T_HAT, "^"},
  {T_LBRACKET, "["},
  {T_RBRACKET, "]"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SemanticError::SemanticError(const stdstr& x) : a::Exception(x) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SyntaxError::SyntaxError(const stdstr& x) : a::Exception(x) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEqual(int wanted, int got, const stdstr& fn) {
  if (wanted != got)
    RAISE(BadArity,
          "%s requires %d args, got %d.\n", C_STR(fn), wanted, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preMax(int max, int got, const stdstr& fn) {
  if (got > max)
    RAISE(BadArity,
          "%s requires at most %d args, got %d.\n", C_STR(fn), max, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preMin(int min, int got, const stdstr& fn) {
  if (got < min)
    RAISE(BadArity,
          "%s requires at least %d args, got %d.\n", C_STR(fn), min, got);
  return got;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preNonZero(int c, const stdstr& fn) {
  if (c == 0)
    RAISE(BadArity,
          "%s requires some args, got %d.\n", C_STR(fn), c);
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEven(int c, const stdstr& fn) {
  if (!a::is_even(c))
    RAISE(BadArity,
          "%s requires even args, got %d.\n", C_STR(fn), c);
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar peek(Context& ctx) {
  // return the current char.
  return ctx.src[ctx.pos];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar peekNext(Context& ctx, int offset) {
  // Peek into the buffer and see what's ahead.
  auto nx = ctx.pos + offset;
  return nx >= 0 && nx < ctx.len ? ctx.src[nx] : '\0';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool forward(Context& ctx) {
  // move up one char, handling newline.
  if (ctx.eof) { return false; }
  if (peek(ctx) == '\n') {
    ++ctx.line;
    ctx.col=0;
  }
  ++ctx.pos;
  if (ctx.pos >= ctx.len) {
    ctx.eof=true;
  } else {
    ++ctx.col;
  }
  return !ctx.eof;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool advance(Context& ctx, int steps) {
  for (int i=0; i < steps; ++i) { forward(ctx); }
  return !ctx.eof;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skipWhitespace(Context& ctx) {
  while (!ctx.eof &&
         ::isspace(peek(ctx))) advance(ctx);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr line(Context& ctx) {
  stdstr res;
  while (!ctx.eof) {
    auto ch = peek(ctx);
    advance(ctx);
    if (ch == '\n')
    break;
    res += peek(ctx);
  }
  return res;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr digits(Context& ctx) {
  stdstr res;
  while (!ctx.eof && ::isdigit(peek(ctx))) {
    res += peek(ctx);
    advance(ctx);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr numeric(Context& ctx) {
  // handles 'signed' and floating points.
  auto ch= peek(ctx);
  auto minus=false;
  if (ch == '-' || ch== '+') {
    minus= (ch=='-');
    advance(ctx);
  }
  auto res = digits(ctx);
  if (!ctx.eof && peek(ctx) == '.') {
    res += peek(ctx);
    advance(ctx);
    res += digits(ctx);
  }

  return minus ? "-"+res : res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr str(Context& ctx) {

  stdstr res;
  Tchar ch;

  if (!ctx.eof &&
      peek(ctx) == '"') {
    advance(ctx);
    while (!ctx.eof) {
      ch= peek(ctx);
      if (ch == '"') {
        break;
      }
      if (ch == '\\') {
        if (!advance(ctx)) {
          RAISE(SyntaxError,
                "Malformed string value, bad escaped char %c\n.", ch);
        }
        ch=a::unescape_char(peek(ctx));
      }
      res += ch;
      advance(ctx);
    }
    if (ctx.eof || ch != '"') {
      RAISE(SyntaxError,
            "Malformed string value, missing %s\n.", "dquote");
    }
    // good, got the end dquote
    advance(ctx);
  }

  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr identifier(Context& ctx, IdPredicate pred) {
  stdstr res;
  Tchar ch;
  while (!ctx.eof) {
    ch=peek(ctx);
    if (res.empty()) {
      if (pred(ch,true)) {
        res += ch;
        advance(ctx);
      } else {
        break;
      }
    } else {
      if (pred(ch,false)) {
        res += ch;
        advance(ctx);
      } else {
        break;
      }
    }
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Number::isZero() const {
  return type==T_INTEGER ? u.n==0 : a::fuzzy_zero(u.r);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Number::isInt() const { return type== T_INTEGER;}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Number::getFloat() const { return u.r; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Number::getInt() const { return u.n; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Number::setFloat(double d) { u.r=d; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Number::setInt(llong n) { u.n=n; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Context::Context() {
  S_NIL(src);
  len=0;
  line=0;
  col=0;
  pos=0;
  eof=false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SrcInfo Context::mark() { return s__pair(int,int,line,col); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::make(const stdstr& n, DslFrame outer) {
  return DslFrame(new Frame(n, outer));
}
DslFrame Frame::make(const stdstr& n) {
  return DslFrame(new Frame(n));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const stdstr& n, DslFrame outer) : _name(n) { prev=outer; }
Frame::Frame(const stdstr& n) : _name(n) { }

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
DslValue Frame::get(const stdstr& key) const {

  auto x= slots.find(key);
  auto r= x != slots.end()
          ? x->second
          : (prev ? prev->get(key) : DslValue(P_NIL));

  DEBUG("frame:get %s <- %s\n",
        C_STR(key), C_STR(r->pr_str()));

  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Frame::setEx(const stdstr& key, DslValue v) {

  auto x= slots.find(key);
  if (x != slots.end()) {
    slots[key]=v;
    DEBUG("frame:setEx %s -> %s\n",
          C_STR(key), C_STR(v->pr_str(1)));
    return v;
  } else if (prev) {
    return prev->setEx(key,v);
  } else {
    return DslValue(P_NIL);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Frame::set(const stdstr& key, DslValue v) {
  slots[key]=v;
  DEBUG("frame:set %s -> %s\n",
        C_STR(key), C_STR(v->pr_str(1)));
  return v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Frame::contains(const stdstr& key) const {
  return slots.find(key) != slots.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::search(const stdstr& key, DslFrame from) {
  ASSERT1(from);
  return from->contains(key) ? from :
         (from->prev ? search(key, from->prev) : DslFrame(P_NIL));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::getRoot(DslFrame from) {
  ASSERT1(from);
  return from->prev ? getRoot(from->prev) : from;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::getOuter() const { return prev; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslTable Table::make(const stdstr& n, const SymbolMap& root) {
  return DslTable(new Table(n,root));
}
DslTable Table::make(const stdstr& n, DslTable outer) {
  return DslTable(new Table(n, outer));
}
DslTable Table::make(const stdstr& n) {
  return DslTable(new Table(n));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const stdstr& n, DslTable outer) : Table(n) { enclosing=outer; }
Table::Table(const stdstr& n, const SymbolMap& root) : Table(n) {
  for (auto& x : root) { symbols[x.first]=x.second; }
}
Table::Table(const stdstr& n) : _name(n) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Table::insert(DslSymbol s) {
  if (s)
    symbols[s->name()] = s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslSymbol Table::search(const stdstr& name) const {
  if (auto s = symbols.find(name); s != symbols.end()) {
    return s->second;
  } else {
    return enclosing ? enclosing->search(name) : DslSymbol(P_NIL);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslSymbol Table::find(const stdstr& name) const {
  if (auto s = symbols.find(name); s != symbols.end()) {
    return s->second;
  } else {
    return DslSymbol(P_NIL);
  }
}





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


