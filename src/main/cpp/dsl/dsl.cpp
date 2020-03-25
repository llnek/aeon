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
Data* nothing() { return new Nothing(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, std::string> TOKENS {
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
SemanticError::SemanticError(const std::string& x) : a::Exception(x) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SyntaxError::SyntaxError(const std::string& x) : a::Exception(x) {}

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
bool advance(Context& ctx) {
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
void skipWhitespace(Context& ctx) {
  while (!ctx.eof &&
         ::isspace(peek(ctx))) advance(ctx);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string digits(Context& ctx) {
  std::string res;
  while (!ctx.eof && ::isdigit(peek(ctx))) {
    res += peek(ctx);
    advance(ctx);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string numeric(Context& ctx) {
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
std::string str(Context& ctx) {

  std::string res;
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
std::string identifier(Context& ctx, IdPredicate pred) {
  std::string res;
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
  if (type==T_INTEGER) { return u.n==0; } else { return u.r==0.0; }
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
Frame::Frame(const std::string& n, DslFrame outer) : _name(n) { prev=outer; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const std::string& n) : _name(n) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Frame::toString() const {
  std::string b(40, '=');
  std::string z(40, '+');
  std::string out;
  Tchar buf[1024];

  ::sprintf(buf,
            "%s\nkey: %s\n%s\n",
            C_STR(b), C_STR(_name), C_STR(b));
  out += buf;

  for (auto& x : slots) {
    ::sprintf(buf,
              "%s = %s\n",
              C_STR(x.first),
              C_STR(x.second->toString()));
    out += buf;
  }

  return out + z + "\n";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::set<std::string> Frame::keys() const {
  std::set<std::string> out;
  for (auto &x : slots) {
    out.insert(x.first);
  }
  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Frame::get(const std::string& key) const {

  auto x= slots.find(key);
  auto r= x != slots.end()
          ? x->second
          : (prev.isSome() ? prev->get(key) : DslValue());

  DEBUG("frame:get %s <- %s\n",
        C_STR(key), C_STR(r->toString()));

  return r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Frame::set(const std::string& key, DslValue v, bool localOnly) {

  auto x= slots.find(key);
  if (x != slots.end() || localOnly) {
    slots[key]=v;
  } else if (prev.isSome()) {
    prev->set(key,v,localOnly);
  }

  DEBUG("frame:set %s -> %s\n",
        C_STR(key), C_STR(v->toString()));

  return v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Frame::containsKey(const stdstr& key) const {
  return slots.find(key) != slots.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::find(const stdstr& key) const {
  auto x= slots.find(key);
  return x != slots.end()
         ? DslFrame(const_cast<Frame*>(this))
         : (prev.isSome() ? prev->find(key) : DslFrame());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::getOuterRoot() const {
  return prev.isSome()
         ? prev->getOuterRoot()
         : DslFrame(const_cast<Frame*>(this));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::getOuter() const { return prev; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const std::string& n,
             const std::map<std::string, DslSymbol>& root) : Table(n) {
  for (auto& x : root) {
    symbols[x.first]=x.second;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const std::string& n, DslTable outer) : Table(n) {
  enclosing=outer;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const std::string& n) : _name(n) {}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Table::insert(DslSymbol s) {
  if (s.isSome())
    symbols[s->name()] = s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslSymbol Table::lookup(const std::string& name, bool traverse) const {
  if (auto s = symbols.find(name); s != symbols.end()) {
    return s->second;
  } else {
    return
      (!traverse || enclosing.isNull())
      ? DslSymbol() : enclosing->lookup(name, traverse);
  }
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


