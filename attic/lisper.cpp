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
namespace czlab::mal {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, std::string> TOKENS {
  {T_UNQUOTE_SPLICE, "~@"},
  {T_TRUE,"true"},
  {T_FALSE,"false"},
  {T_NIL,"nil"}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<std::string,int> KEYWORDS {
  {map__val(TOKENS,T_FALSE),T_FALSE},
  {map__val(TOKENS,T_TRUE),T_TRUE},
  {map__val(TOKENS,T_NIL),T_NIL}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Token::typeToString(int type) {
  auto x= TOKENS.find(type);
  if (x != TOKENS.end()) {
    return map__val(TOKENS,type);
  } else  {
    return std::string("token-type=") + std::to_string(type);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const char ch, d::TokenInfo info) {
  impl.text= std::string();
  impl.text += ch;
  impl.line=info.line;
  impl.col=info.col;
  impl.type=type;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const std::string& s, d::TokenInfo info) {
  impl.text= s;
  impl.line=info.line;
  impl.col=info.col;
  impl.type=type;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::~Token() {
  if (impl.type == d::T_IDENT ||
      impl.type == d::T_STRING) {
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Token::type() {
  return impl.type;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Token::getLiteralAsReal() {
  if (impl.type != d::T_REAL) {
    //error("REAL", impl);
  }
  return impl.value.u.r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
long Token::getLiteralAsInt() {
  if (impl.type != d::T_INTEGER) {
    //error("INTEGER", impl);
  }
  return impl.value.u.n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Token::getLiteralAsStr() {
  if (impl.type == d::T_IDENT ||
      impl.type == d::T_STRING) {
    return impl.value.cs.get()->get();
  }

  if (! contains(TOKENS,impl.type)) {
    //error("ID,String,Keyword", impl);
  }

  return TOKENS.at(impl.type);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* token(int type, const SString& x, d::TokenInfo info, const SString& s) {
  auto t= new Token(type, x, info);
  auto len= s.length();
  t->impl.value.cs = std::make_shared<a::CString>(len);
  t->impl.value.cs.get()->copy(s.c_str());
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* token(int type, const SString& s, d::TokenInfo info, long n) {
  auto t= new Token(type, s, info);
  t->impl.value.u.n=n;
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* token(int type, const std::string& s, d::TokenInfo info, double d) {
  auto t= new Token(type,s, info);
  t->impl.value.u.r=d;
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Token::toString() {
  char buf[1024];
  ::sprintf(buf, "Token#{type = %d, text = %s}", impl.type, impl.text.c_str());
  return SString(buf);
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
Reader::~Reader() {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Reader::isKeyword(const std::string& k) {
  return KEYWORDS.find(k) != KEYWORDS.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Reader::skipComment() {
  auto m= ctx.mark();
  SString s;
  char c;

  while (!ctx.eof) {
    d::advance(ctx);
    c= d::peek(ctx);
    if (c == '\n') {
      break;
    }
    s += c;
  }

  return new Token(T_COMMENT, s, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Reader::number() {
  auto m= ctx.mark();
  auto s = d::numeric(ctx).c_str();
  return ::strchr(s, '.')
    ? token(d::T_REAL, s, m, ::atof(s))
    : token(d::T_INTEGER, s, m, ::atol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Reader::string() {
  auto m= ctx.mark();
  auto s = d::str(ctx);
  return token(d::T_STRING, s, m, s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Reader::id() {
  auto m= ctx.mark();
  auto s= d::identifier(ctx);
  return !isKeyword(s)
    ? token(d::T_IDENT, s, m, s)
    : new Token(KEYWORDS.at(s), s, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Reader::skipCommas() {
  while (!ctx.eof &&
         (peek(ctx) == ',')) advance(ctx);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Reader::getNextToken() {
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
        d::peek(ctx) == '@') {
    }
    else
    if (ch == '\'') {
    }
    else
    if (ch == '`') {
    }
    else
    if (ch == '~') {
    }
    else
    if (ch == '^') {
    }
    else
    if (ch == '@') {
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
      return new Token(d::T_LPAREN, ch, m);
    }
    else
    if (ch == ')') {
      auto m=ctx.mark();
      d::advance(ctx);
      return new Token(d::T_RPAREN, ch, m);
    }
    else
    if (ch == '{') {
      auto m=ctx.mark();
      d::advance(ctx);
      return new Token(d::T_LBRACE, ch, m);
    }
    else
    if (ch == '}') {
      auto m=ctx.mark();
      d::advance(ctx);
      return new Token(d::T_RBRACE, ch, m);
    }
    else
    if (ch == '[') {
      auto m=ctx.mark();
      d::advance(ctx);
      return new Token(d::T_LBRACKET, ch, m);
    }
    else
    if (ch == ']') {
      auto m=ctx.mark();
      d::advance(ctx);
      return new Token(d::T_RBRACKET, ch, m);
    }
    else
    if (ch == ':') {
      auto m=ctx.mark();
      d::advance(ctx);
      return new Token(d::T_COLON, ch, m);
    }
    else
    if (ch == '.') {
      auto m=ctx.mark();
      d::advance(ctx);
      return new Token(d::T_DOT, ch, m);
    }
    else
    if (ch) {
      return id();
    }
    else {
      DSL_ERROR(d::SyntaxError, "Unexpected char %c near line %d, col %d.\n", ch, ctx.line, ctx.col);
    }
  }

  return new Token(d::T_EOF, "<EOF>", ctx.mark());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


