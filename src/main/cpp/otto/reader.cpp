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

#include "reader.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::otto {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> TOKENS {
  {T_SPLICE_UNQUOTE, "~@"},
  {T_ANONFN, "#("},
  {T_SET, "#{"},
  {T_TRUE,"true"},
  {T_FALSE,"false"},
  {T_NIL,"nil"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<stdstr,int> KEYWORDS {
  {map__val(TOKENS,T_SPLICE_UNQUOTE),T_SPLICE_UNQUOTE},
  {map__val(TOKENS,T_ANONFN),T_ANONFN},
  {map__val(TOKENS,T_SET),T_SET},
  {map__val(TOKENS,T_FALSE),T_FALSE},
  {map__val(TOKENS,T_TRUE),T_TRUE},
  {map__val(TOKENS,T_NIL),T_NIL}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int type) {
  return s__contains(TOKENS,type)
         ? map__val(TOKENS,type) : ("token-type=" + N_STR(type));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LToken::LToken(int t, cstdstr& s, d::Mark m) : d::Lexeme(t,m) {
  lexeme=s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LToken::LToken(int t, Tchar c, d::Mark m) : d::Lexeme(t,m) {
  lexeme= stdstr { c };
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LToken::pr_str() const {
  return lexeme;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double LToken::getFloat() const {
  return type() == d::T_REAL ? number.r : number.n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong LToken::getInt() const {
  return type() == d::T_INTEGER ? number.n : number.r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LToken::getStr() const {
  return s__contains(TOKENS,type()) ? TOKENS.at(type()) : lexeme;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token(int t, cstdstr& x, d::Mark info) {
  return LToken::make(t, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token(int t, Tchar x, d::Mark info) {
  return LToken::make(t, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token_long(cstdstr& s, d::Mark info) {
  auto k= LToken::make(d::T_INTEGER, s, info);
  llong n= ::atol(s.c_str());
  DCAST(LToken,k)->setLiteral(n);
  return k;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token_real(cstdstr& s, d::Mark info) {
  auto k= LToken::make(d::T_REAL, s, info);
  auto d= ::atof(s.c_str());
  DCAST(LToken,k)->setLiteral(d);
  return k;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token(d::Context& ctx) {
  return LToken::make(d::T_EOF, "<EOF>", ctx.mark());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Reader::Reader(const Tchar* src) {
  _ctx.len= ::strlen(src);
  _ctx.src=src;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Reader::isKeyword(cstdstr&) const {
  throw d::Unsupported("isKeyword() not allowed!");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::skipComment() {
  auto m= _ctx.mark();
  stdstr s;
  while (!_ctx.eof) {
    d::advance(_ctx);
    auto c= d::peek(_ctx);
    if (c == '\n') { break; }
    s += c;
  }
  return token(d::T_COMMENT, s, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::number() {
  auto ch= d::peek(_ctx);
  bool minus=0;

  if (ch == '-' ||
      ch == '+') {
    minus= (ch == '-');
    d::advance(_ctx); }

  auto res= d::numeric(_ctx);
  auto ds= res.first;

  if (minus)
    ds = "-" + ds;

  return ::strchr(ds.c_str(), '.')
    ? token_real(ds, res.second)
    : token_long(ds, res.second);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::string() {
  auto res= d::str(_ctx);
  return token(d::T_STRING, res.first, res.second);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool isKeywdChar(Tchar ch)  {
  return ch == '_' || ch == '-' ||
         ch == '/' ||
         ::isalpha(ch) || ::isdigit(ch);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::keywd() {
  auto m= d::mark_advance(_ctx); // skip the colon
  stdstr res;
  while (!_ctx.eof) {
    auto ch=peek(_ctx);
    if (isKeywdChar(ch)) {
      res += ch;
      advance(_ctx); } else { break; } }
  return token(T_KEYWORD, res, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::id() {
  static stdstr bad("{}[]()'\"\\`@~^,.;");
  auto m= _ctx.mark();
  stdstr res;
  while (!_ctx.eof) {
    auto ch=peek(_ctx);
    auto pos = bad.find(ch);
    if (::isspace(ch) ||
        pos != std::string::npos) { break; }
    res += ch;
    d::advance(_ctx);
  }
  if (res == "false") {
    return token(T_FALSE, res, m); }
  if (res == "true") {
    return token(T_TRUE, res, m); }
  if (res == "nil") {
    return token(T_NIL, res, m); }
  DEBUG("::id = %s.\n", C_STR(res));
  return token(d::T_IDENT, res, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Reader::skipCommas() {
  while (!_ctx.eof &&
         (peek(_ctx) == ',')) advance(_ctx);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::getNextToken() {
  while (!_ctx.eof) {
    auto ch= d::peek(_ctx);
    if (::isspace(ch)) {
      d::skipWhitespace(_ctx);
    }
    else
    if (ch == ',') {
      skipCommas();
    }
    else
    if (ch == ';') {
      skipComment();
    }
    else
    if (ch == '~' &&
        d::peekAhead(_ctx) == '@') {
      auto m= d::mark_advance(_ctx, 2);
      return token(T_SPLICE_UNQUOTE, "~@", m);
    }
    else
    if (ch == '\'') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_QUOTE, ch, m);
    }
    else
    if (ch == '`') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_BACKTICK, ch, m);
    }
    else
    if (ch == '~') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_TILDA, ch, m);
    }
    else
    if (ch == '^') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_HAT, ch, m);
    }
    else
    if (ch == '@') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_AT, ch, m);
    }
    else
    if ((ch == '-' || ch == '+') &&
        ::isdigit(d::peekAhead(_ctx))) {
      return number();
    }
    else
    if (::isdigit(ch)) {
      return number();
    }
    else
    if (ch == '"') {
      return string();
    }
    else
    if (ch == '#' &&
        d::peekAhead(_ctx) == '{') {
      auto m= d::mark_advance(_ctx, 2);
      return token(T_SET, "#{", m);
    }
    else
    if (ch == '#' &&
        d::peekAhead(_ctx) == '(') {
      auto m= d::mark_advance(_ctx, 2);
      return token(T_ANONFN, "#(", m);
    }
    else
    if (ch == '(') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_LPAREN, ch, m);
    }
    else
    if (ch == ')') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_RPAREN, ch, m);
    }
    else
    if (ch == '{') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_LBRACE, ch, m);
    }
    else
    if (ch == '}') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_RBRACE, ch, m);
    }
    else
    if (ch == '[') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_LBRACKET, ch, m);
    }
    else
    if (ch == ']') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_RBRACKET, ch, m);
    }
    else
    if (ch == ':') {
      auto nx= d::peekAhead(_ctx);
      if (isKeywdChar(nx)) {
        return keywd();
      } else {
        auto m= d::mark_advance(_ctx);
        return token(d::T_COLON, ch, m);
      }
    }
    else
    if (ch == '.') {
      auto m= d::mark_advance(_ctx);
      return token(d::T_DOT, ch, m);
    }
    else
    if (ch) {
      return id();
    }
    else {
      auto m= d::mark_advance(_ctx);
      return token(d::T_ROGUE, ch,m);
    }
  }

  return token(_ctx);
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


