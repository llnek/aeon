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
char peek(Context& ctx) {
  return ctx.src[ctx.pos];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char peekNext(Context& ctx) {
  auto nx = ctx.pos + 1;
  return (nx >= ctx.len) ? '\0': ctx.src[nx];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool advance(Context& ctx) {
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
  auto res = digits(ctx);
  if (!ctx.eof && peek(ctx) == '.') {
    res += peek(ctx);
    advance(ctx);
    res += digits(ctx);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string str(Context& ctx) {

  std::string res;
  char ch;

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
std::string identifier(Context& ctx) {
  std::string res;
  char ch;
  while (!ctx.eof) {
    ch=peek(ctx);
    if (res.empty()) {
      if (ch == '_' || ::isalpha(ch)) {
        res += ch;
        advance(ctx);
      } else {
        break;
        //error(ctx);
      }
    } else {
      if (ch == '_' || ::isalpha(ch) || ::isdigit(ch)) {
        res += ch;
        advance(ctx);
      } else {
        break;
        //error(ctx);
      }
    }
  }
  return res;
}

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
Frame::Frame(const std::string& name, DslFrame outer) : name(name) {
  prev=outer;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const std::string& name) : name(name) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::~Frame() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Frame::toString() {
  std::string b(40,'=');
  std::string z(40,'+');
  std::string out;
  char buf[1024];
  ::sprintf(buf,
            "%s\nkey: %s\n%s\n",
            b.c_str(), name.c_str(),b.c_str());
  out += buf;
  for (auto& x : slots) {
    ::sprintf(buf,
              "%s = %s\n",
              x.first.c_str(),
              x.second.ptr()->toString().c_str());
    out += buf;
  }
  return out + z + "\n";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::set<std::string> Frame::keys() {
  std::set<std::string> out;
  for (auto &x : slots) {
    out.insert(x.first);
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslValue Frame::get(const std::string& key) {
  auto x= slots.find(key);
  auto r= x != slots.end()
    ? x->second
    : (prev.isSome() ? prev.ptr()->get(key) : DslValue());
  DEBUG("frame:getting %s to %s\n",
      key.c_str(), r.ptr()->toString().c_str());
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Frame::set(const std::string& key, DslValue v, bool localOnly) {
  auto x= slots.find(key);

  DEBUG("frame:setting %s to %s\n",
      key.c_str(), v.ptr()->toString().c_str());

  if (x != slots.end() || localOnly) {
    slots[key]=v;
  } else if (prev.isSome()) {
    prev.ptr()->set(key,v,localOnly);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Frame::find(const stdstr& key, bool localOnly) {
  auto x= slots.find(key);
  if (x != slots.end()) {
    return true;
  }
  return (!localOnly && prev.isSome())
    ? prev.ptr()->find(key,localOnly) : false;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslFrame Frame::getOuter() { return prev; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const std::string& n,
    const std::map<std::string, DslSymbol>& root)
  : Table(n) {
  for (auto& x : root) {
    symbols[x.first]=x.second;
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const std::string& n, DslTable outer)
  : Table(n) {
  enclosing=outer;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::Table(const std::string& n) {
  name=n;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Table::~Table() {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Table::insert(DslSymbol s) {
  if (s.isSome()) symbols[s.ptr()->name] = s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslSymbol Table::lookup(const std::string& name, bool traverse) {
  if (auto s = symbols.find(name); s != symbols.end()) {
    return s->second;
  } else {
    return
      (!traverse || enclosing.isNull())
      ? DslSymbol() : enclosing.ptr()->lookup(name, traverse);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





