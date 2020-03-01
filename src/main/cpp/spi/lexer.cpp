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

#include "lexer.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::spi {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, std::string> TOKENS {
  {T_PROCEDURE, "PROCEDURE"},
  {T_PROGRAM, "PROGRAM"},
  {T_VAR, "VAR"},
  {T_REAL, "REAL"},
  {T_INT, "INTEGER"},
  {T_STR, "STRING"},
  {T_INT_DIV, "DIV"},
  {T_BEGIN, "BEGIN"},
  {T_ASSIGN, ":="},
  {T_END, "END"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<std::string,int> KEYWORDS {
  {map__val(TOKENS,T_BEGIN), T_BEGIN},
  {map__val(TOKENS,T_END), T_END},
  {map__val(TOKENS,T_PROGRAM), T_PROGRAM},
  {map__val(TOKENS,T_PROCEDURE), T_PROCEDURE},
  {map__val(TOKENS,T_VAR), T_VAR},
  {map__val(TOKENS,T_STR), T_STR},
  {map__val(TOKENS,T_INT), T_INT},
  {map__val(TOKENS,T_REAL), T_REAL},
  {map__val(TOKENS,T_INT_DIV), T_INT_DIV}
};

static char BUF[1024];
static void error(const std::string& expected, const d::Lexeme& lex) {
  ::sprintf(BUF,
            "Expecting %s, but token type = %d, near line %d, column %d.\n",
            expected.c_str(), lex.type, lex.line, lex.col);
  throw d::SyntaxError(BUF);
}

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
Token::Token(int type, const char ch, int line, int col) {
  impl.text= std::string();
  impl.text += ch;
  impl.line=line;
  impl.col=col;
  impl.type=type;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const std::string& s, int line, int col) {
  impl.text= s;
  impl.line=line;
  impl.col=col;
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
    error("REAL", impl);
  }
  return impl.value.u.r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
long Token::getLiteralAsInt() {
  if (impl.type != d::T_INTEGER) {
    error("INTEGER", impl);
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
    error("ID,String,Keyword", impl);
  }

  return TOKENS.at(impl.type);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* token(int type, const std::string& x, int line, int pos, const std::string& s) {
  auto t= new Token(type, x, line,pos);
  auto len= s.length();
  t->impl.value.cs = std::make_shared<a::CString>(len);
  t->impl.value.cs.get()->copy(s.c_str());
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* token(int type, const std::string& s, int line, int col, long n) {
  auto t= new Token(type, s, line, col);
  t->impl.value.u.n=n;
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* token(int type, const std::string& s, int line, int col, double d) {
  auto t= new Token(type,s, line,col);
  t->impl.value.u.r=d;
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Token::toString() {
  ::sprintf(BUF, "Token#{type = %d, text = %s}", impl.type, impl.text.c_str());
  return std::string(BUF);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::Lexer(const char* src) {
  ctx.len= ::strlen(src);
  ctx.eof=false;
  ctx.src=src;
  ctx.line=0;
  ctx.col=0;
  ctx.pos=0;
  ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::~Lexer() {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lexer::isKeyword(const std::string& k) {
  return KEYWORDS.find(k) != KEYWORDS.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Lexer::skipComment() {
  while (!ctx.eof) {
    d::advance(ctx);
    if (d::peek(ctx) == '}') {
      d::advance(ctx);
      break;
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Lexer::number() {
  auto s = d::numeric(ctx).c_str();
  return ::strchr(s, '.')
    ? token(d::T_REAL, s, ctx.line, ctx.col, ::atof(s))
    : token(d::T_INTEGER, s, ctx.line, ctx.col, ::atol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Lexer::string() {
  auto s = d::str(ctx);
  return token(d::T_STRING, s, ctx.line, ctx.col, s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Lexer::id() {
  auto s= d::identifier(ctx);
  auto S= a::toupper(s);
  return !isKeyword(S)
    ? token(d::T_IDENT, s, ctx.line, ctx.col, s)
    : new Token(KEYWORDS.at(S), S, ctx.line, ctx.col);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Lexer::getNextToken() {
  char ch;
  while (!ctx.eof) {
    ch= d::peek(ctx);
    if (::isspace(ch)) {
      d::skipWhitespace(ctx);
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
    if (ch == '*') {
      d::advance(ctx);
      return new Token(d::T_MULT, ch, ctx.line, ctx.col);
    }
    else
    if (ch == '/') {
      d::advance(ctx);
      return new Token(d::T_DIV, ch, ctx.line, ctx.col);
    }
    else
    if (ch == '+') {
      d::advance(ctx);
      return new Token(d::T_PLUS, ch, ctx.line, ctx.col);
    }
    else
    if (ch == '-') {
      d::advance(ctx);
      return new Token(d::T_MINUS, ch, ctx.line, ctx.col);
    }
    else
    if (ch == '(') {
      d::advance(ctx);
      return new Token(d::T_LPAREN, ch, ctx.line, ctx.col);
    }
    else
    if (ch == ')') {
      d::advance(ctx);
      return new Token(d::T_RPAREN, ch, ctx.line, ctx.col);
    }
    else
    if (ch == '_' || ::isalpha(ch)) {
      return id();
    }
    else
    if (ch== ':' && '=' == d::peekNext(ctx)) {
      d::advance(ctx);
      d::advance(ctx);
      return new Token(T_ASSIGN, ":=", ctx.line, ctx.col);
    }
    else
    if (ch == '{') {
      d::advance(ctx);
      skipComment();
    }
    else
    if (ch == ';') {
      d::advance(ctx);
      return new Token(d::T_SEMI, ch, ctx.line, ctx.col);
    }
    else
    if (ch == ':') {
      d::advance(ctx);
      return new Token(d::T_COLON, ch, ctx.line, ctx.col);
    }
    else
    if (ch == ',') {
      d::advance(ctx);
      return new Token(d::T_COMMA, ch, ctx.line, ctx.col);
    }
    else
    if (ch == '.') {
      d::advance(ctx);
      return new Token(d::T_DOT, ch, ctx.line, ctx.col);
    }
    else {
      ::sprintf(BUF,"Unexpected char %c near line %d, col %d.\n", ch, ctx.line, ctx.col);
      throw d::SyntaxError(BUF);
    }
  }

  return new Token(d::T_EOF, "<EOF>", ctx.line, ctx.col);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

