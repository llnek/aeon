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
         ? map__val(TOKENS,type)
         : stdstr("token-type=") + N_STR(type);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LToken::LToken(int t, cstdstr& s, d::Mark m) : d::Token(t,s,m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LToken::LToken(int t, Tchar c, d::Mark m) : d::Token(t,c,m) { }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LToken::getStr() const {
  return s__contains(TOKENS,type()) ? TOKENS.at(type()) : lexeme;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int t, cstdstr& x, d::Mark info) {
  return LToken::make(t, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int t, Tchar x, d::Mark info) {
  return LToken::make(t, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int t, cstdstr& x, d::Mark info, cstdstr&) {
  return LToken::make(t, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int t, cstdstr& s, d::Mark info, llong n) {
  auto k= LToken::make(t, s, info);
  DCAST(LToken,k)->setLiteral(n);
  return k;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int t, cstdstr& s, d::Mark info, double d) {
  auto k= LToken::make(t, s, info);
  DCAST(LToken,k)->setLiteral(d);
  return k;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Reader::Reader(const Tchar* src) {
  _ctx.len= ::strlen(src);
  _ctx.eof=false;
  _ctx.src=src;
  _ctx.line=0;
  _ctx.col=0;
  _ctx.pos=0;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Reader::isKeyword(cstdstr& k) const {
  throw d::Unsupported("not allowed!");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::skipComment() {
  auto m= _ctx.mark();
  stdstr s;
  Tchar c;

  while (!_ctx.eof) {
    d::advance(_ctx);
    c= d::peek(_ctx);
    if (c == '\n') {
      break;
    }
    s += c;
  }

  return token(d::T_COMMENT, s, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::number() {
  auto m= _ctx.mark();
  auto s= C_STR(d::numeric(_ctx));
  return ::strchr(s, '.')
    ? token(d::T_REAL, s, m, ::atof(s))
    : token(d::T_INTEGER, s, m, (llong) ::atol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::string() {
  auto m= _ctx.mark();
  auto s= d::str(_ctx);
  return token(d::T_STRING, s, m, s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool isKeywdChar(Tchar ch)  {
  return ch == '_' || ch == '-' ||
         ch == '/' ||
         ::isalpha(ch) || ::isdigit(ch);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::keywd() {
  auto m= _ctx.mark();
  stdstr res;
  d::advance(_ctx); // skip the colon
  while (!_ctx.eof) {
    auto ch=peek(_ctx);
    if (isKeywdChar(ch)) {
      res += ch;
      advance(_ctx); } else { break; }
  }
  return token(T_KEYWORD, res, m, res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::id() {
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
  return token(d::T_IDENT, res, m, res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Reader::skipCommas() {
  while (!_ctx.eof &&
         (peek(_ctx) == ',')) advance(_ctx);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::getNextToken() {
  Tchar ch;
  while (!_ctx.eof) {
    ch= d::peek(_ctx);
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
      auto m= _ctx.mark();
      d::advance(_ctx, 2);
      return token(T_SPLICE_UNQUOTE, "~@", m);
    }
    else
    if (ch == '\'') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_QUOTE, ch, m);
    }
    else
    if (ch == '`') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_BACKTICK, ch, m);
    }
    else
    if (ch == '~') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_TILDA, ch, m);
    }
    else
    if (ch == '^') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_HAT, ch, m);
    }
    else
    if (ch == '@') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_AT, ch, m);
    }
    else
    if ((ch == '-' || ch == '+') &&
        ::isdigit(d::peekAhead(_ctx))) {
      //skip the pointless + sign
      if (ch=='+') { d::advance(_ctx); }
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
      auto m=_ctx.mark();
      d::advance(_ctx, 2);
      return token(T_SET, "#{", m);
    }
    else
    if (ch == '#' &&
        d::peekAhead(_ctx) == '(') {
      auto m=_ctx.mark();
      d::advance(_ctx, 2);
      return token(T_ANONFN, "#(", m);
    }
    else
    if (ch == '(') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_LPAREN, ch, m);
    }
    else
    if (ch == ')') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_RPAREN, ch, m);
    }
    else
    if (ch == '{') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_LBRACE, ch, m);
    }
    else
    if (ch == '}') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_RBRACE, ch, m);
    }
    else
    if (ch == '[') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_LBRACKET, ch, m);
    }
    else
    if (ch == ']') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_RBRACKET, ch, m);
    }
    else
    if (ch == ':') {
      auto nx= d::peekAhead(_ctx);
      if (isKeywdChar(nx)) {
        return keywd();
      } else {
        auto m=_ctx.mark();
        d::advance(_ctx);
        return token(d::T_COLON, ch, m);
      }
    }
    else
    if (ch == '.') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_DOT, ch, m);
    }
    else
    if (ch) {
      return id();
    }
    else {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_ROGUE, ch,m);
    }
  }

  return token(d::T_EOF, "<EOF>", _ctx.mark());
}







//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


