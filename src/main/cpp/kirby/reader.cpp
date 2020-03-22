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
namespace czlab::kirby {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> TOKENS {
  {T_UNQUOTE_SPLICE, "~@"},
  {T_TRUE,"true"},
  {T_FALSE,"false"},
  {T_NIL,"nil"}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<stdstr,int> KEYWORDS {
  {map__val(TOKENS,T_UNQUOTE_SPLICE),T_UNQUOTE_SPLICE},
  {map__val(TOKENS,T_FALSE),T_FALSE},
  {map__val(TOKENS,T_TRUE),T_TRUE},
  {map__val(TOKENS,T_NIL),T_NIL}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::typeToString(int type) {
  if (auto x= TOKENS.find(type); x != TOKENS.end()) {
    return map__val(TOKENS,type);
  } else  {
    return stdstr("token-type=") + std::to_string(type);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const char ch, d::SrcInfo info) : d::Chunk(type) {
  impl.text= stdstr() + ch;
  impl.line=info.first;
  impl.col=info.second;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const stdstr& s, d::SrcInfo info) : d::Chunk(type) {
  impl.line=info.first;
  impl.col=info.second;
  impl.text= s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Token::getLiteralAsReal() {
  if (type() != d::T_REAL) {
    RAISE(d::SemanticError,
        "Expecting float near %d(%d).\n", impl.line, impl.col);
  }
  return impl.value.u.r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Token::getLiteralAsInt() {
  if (type() != d::T_INTEGER) {
    RAISE(d::SemanticError,
        "Expecting int near %d(%d).\n", impl.line, impl.col);
  }
  return impl.value.u.n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::getLiteralAsStr() {
  if (type() == d::T_IDENT ||
      type() == T_KEYWORD ||
      type() == d::T_STRING) {
    return impl.value.cs.get()->get();
  }

  if (! s__contains(TOKENS,type())) {
    RAISE(d::SemanticError,
        "Unexpected token %d near %d(%d).\n",
        type(), impl.line, impl.col);
  }

  return TOKENS.at(type());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& x, d::SrcInfo info) {
  return d::DslToken(new Token(type, x, info));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const char x, d::SrcInfo info) {
  return d::DslToken(new Token(type, x, info));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& x, d::SrcInfo info, const stdstr& s) {
  auto t= new Token(type, x, info);
  auto len= s.length();
  t->impl.value.cs = std::make_shared<a::CString>(len);
  t->impl.value.cs.get()->copy(s.c_str());
  return d::DslToken(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& s, d::SrcInfo info, llong n) {
  auto t= new Token(type, s, info);
  t->impl.value.u.n=n;
  return d::DslToken(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& s, d::SrcInfo info, double d) {
  auto t= new Token(type,s, info);
  t->impl.value.u.r=d;
  return d::DslToken(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::toString() {
  char buf[1024];
  ::sprintf(buf, "Token#{type = %d, text = %s}", type(), impl.text.c_str());
  return stdstr(buf);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Reader::Reader(const char* src) {
  ctx.len= ::strlen(src);
  ctx.eof=false;
  ctx.src=src;
  ctx.line=0;
  ctx.col=0;
  ctx.pos=0;
  ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Reader::isKeyword(const stdstr& k) {
  throw d::SyntaxError("not allowed!");
  //return KEYWORDS.find(k) != KEYWORDS.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Reader::skipComment() {
  //auto m= ctx.mark();
  stdstr s;
  char c;

  while (!ctx.eof) {
    d::advance(ctx);
    c= d::peek(ctx);
    if (c == '\n') {
      break;
    }
    s += c;
  }

  //DslToken(new Token(d::T_COMMENT, s, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::number() {
  auto m= ctx.mark();
  auto s = d::numeric(ctx).c_str();
  return ::strchr(s, '.')
    ? token(d::T_REAL, s, m, ::atof(s))
    : token(d::T_INTEGER, s, m, (llong) ::atol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::string() {
  auto m= ctx.mark();
  auto s = d::str(ctx);
  return token(d::T_STRING, s, m, s);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool isKeywdChar(char ch)  {
  return ch == '_' || ch == '-' ||
         ch == '/' ||
         ::isalpha(ch) || ::isdigit(ch);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::keywd() {
  auto m= ctx.mark();
  stdstr res;
  d::advance(ctx); // skip the colon
  while (!ctx.eof) {
    auto ch=peek(ctx);
    if (isKeywdChar(ch)) {
      res += ch;
      advance(ctx);
    } else {
      break;
    }
  }
  return token(T_KEYWORD, res, m, res);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::id() {
  static stdstr bad("{}[]()'\"\\`@~^,.;");
  auto m= ctx.mark();
  stdstr res;
  while (!ctx.eof) {
    auto ch=peek(ctx);
    auto pos = bad.find(ch);
    if (pos != std::string::npos) {
      break;
    }
    if (::isspace(ch)) {
      break;
    }
    res += ch;
    d::advance(ctx);
  }
  if (res == "false") {
    return token(T_FALSE, res, m);
  }
  if (res == "true") {
    return token(T_TRUE, res, m);
  }
  if (res == "nil") {
    return token(T_NIL, res, m);
  }
  ::printf("id = %s\n", res.c_str());
  return token(d::T_IDENT, res, m, res);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Reader::skipCommas() {
  while (!ctx.eof &&
         (peek(ctx) == ',')) advance(ctx);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Reader::getNextToken() {
  char ch;
  while (!ctx.eof) {
    ch= d::peek(ctx);
    if (::isspace(ch)) {
      d::skipWhitespace(ctx);
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
        d::peekNext(ctx) == '@') {
      auto m= ctx.mark();
      d::advance(ctx);
      d::advance(ctx);
      return token(T_UNQUOTE_SPLICE, "~@", m);
    }
    else
    if (ch == '\'') {
      auto m= ctx.mark();
      d::advance(ctx);
      return token(d::T_QUOTE, ch, m);
    }
    else
    if (ch == '`') {
      auto m= ctx.mark();
      d::advance(ctx);
      return token(d::T_BACKTICK, ch, m);
    }
    else
    if (ch == '~') {
      auto m= ctx.mark();
      d::advance(ctx);
      return token(d::T_TILDA, ch, m);
    }
    else
    if (ch == '^') {
      auto m= ctx.mark();
      d::advance(ctx);
      return token(d::T_HAT, ch, m);
    }
    else
    if (ch == '@') {
      auto m= ctx.mark();
      d::advance(ctx);
      return token(d::T_AT, ch, m);
    }
    else
    if ((ch == '-' || ch == '+') &&
        ::isdigit(d::peekNext(ctx))) {
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
    if (ch == '(') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_LPAREN, ch, m);
    }
    else
    if (ch == ')') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_RPAREN, ch, m);
    }
    else
    if (ch == '{') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_LBRACE, ch, m);
    }
    else
    if (ch == '}') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_RBRACE, ch, m);
    }
    else
    if (ch == '[') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_LBRACKET, ch, m);
    }
    else
    if (ch == ']') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_RBRACKET, ch, m);
    }
    else
    if (ch == ':') {
      auto nx= d::peekNext(ctx);
      if (isKeywdChar(nx)) {
        return keywd();
      } else {
        auto m=ctx.mark();
        d::advance(ctx);
        return token(d::T_COLON, ch, m);
      }
    }
    else
    if (ch == '.') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_DOT, ch, m);
    }
    else
    if (ch) {
      return id();
    }
    else {
      RAISE(d::SyntaxError,
          "Unexpected char %c near line %d(%d).\n", ch, ctx.line, ctx.col);
    }
  }

  return token(d::T_EOF, "<EOF>", ctx.mark());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


