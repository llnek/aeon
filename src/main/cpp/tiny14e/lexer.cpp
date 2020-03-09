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
namespace czlab::tiny14e {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
using namespace czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, std::string> TOKENS {
  {T_PROCEDURE, "PROCEDURE"},
  {T_PROGRAM, "PROGRAM"},
  {T_WRITELN, "WRITELN"},
  {T_WRITE, "WRITE"},
  {T_READLN, "READLN"},
  {T_READ, "READ"},
  {T_WHILE, "WHILE"},
  {T_ENDWHILE, "ENDWHILE"},
  {T_REPEAT, "REPEAT"},
  {T_UNTIL, "UNTIL"},
  {T_FOR, "FOR"},
  {T_ENDFOR, "ENDFOR"},
  {T_IF, "IF"},
  {T_ELSE,"ELSE"},
  {T_ENDIF, "ENDIF"},
  {T_VAR, "VAR"},
  {T_REAL, "REAL"},
  {T_INT, "INTEGER"},
  {T_STR, "STRING"},
  {T_INT_DIV, "DIV"},
  {T_BEGIN, "BEGIN"},
  {T_END, "END"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<std::string,int> KEYWORDS {
  {map__val(TOKENS,T_BEGIN), T_BEGIN},
  {map__val(TOKENS,T_END), T_END},
  {map__val(TOKENS,T_PROGRAM), T_PROGRAM},
  {map__val(TOKENS,T_PROCEDURE), T_PROCEDURE},
  {map__val(TOKENS,T_WRITELN),T_WRITELN},
  {map__val(TOKENS,T_WRITE),T_WRITE},
  {map__val(TOKENS,T_READLN),T_READLN},
  {map__val(TOKENS,T_READ),T_READ},
  {map__val(TOKENS,T_WHILE), T_WHILE},
  {map__val(TOKENS,T_ENDWHILE), T_ENDWHILE},
  {map__val(TOKENS,T_REPEAT), T_REPEAT},
  {map__val(TOKENS,T_UNTIL), T_UNTIL},
  {map__val(TOKENS,T_FOR), T_FOR},
  {map__val(TOKENS,T_ENDFOR), T_ENDFOR},
  {map__val(TOKENS,T_IF),T_IF},
  {map__val(TOKENS,T_ELSE),T_ELSE},
  {map__val(TOKENS,T_ENDIF),T_ENDIF},
  {map__val(TOKENS,T_VAR), T_VAR},
  {map__val(TOKENS,T_STR), T_STR},
  {map__val(TOKENS,T_INT), T_INT},
  {map__val(TOKENS,T_REAL), T_REAL},
  {map__val(TOKENS,T_INT_DIV), T_INT_DIV}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Token::typeToString(int type) {
  if (auto x= TOKENS.find(type); x != TOKENS.end()) {
    return map__val(TOKENS,type);
  } else  {
    return std::string("token-type=") + std::to_string(type);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const char ch, SrcInfo info) : d::Chunk(type) {
  impl.text= std::string();
  impl.text += ch;
  impl.line=info.first;
  impl.col=info.second;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const std::string& s, SrcInfo info) : d::Chunk(type) {
  impl.text= s;
  impl.line=info.first;
  impl.col=info.second;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Token::getLiteralAsReal() {
  if (type() != d::T_REAL) {
    raise(SemanticError,
        "Expecting float near %d(%d).\n", impl.line, impl.col);
  }
  return impl.value.u.r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Token::getLiteralAsInt() {
  if (type() != d::T_INTEGER) {
    raise(SemanticError,
        "Expecting int near %d(%d).\n", impl.line, impl.col);
  }
  return impl.value.u.n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Token::getLiteralAsStr() {
  if (type() == d::T_IDENT ||
      type() == d::T_STRING) {
    return impl.value.cs.get()->get();
  }

  if (! contains(TOKENS,type())) {
    raise(SemanticError,
        "Expecting identifier near %d(%d).\n", impl.line, impl.col);
  }

  return TOKENS.at(type());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken token(int type, const char c, SrcInfo info) {
  return DslToken(new Token(type, c, info));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken token(int type, const std::string& s, SrcInfo info) {
  return DslToken(new Token(type, s, info));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken token(int type, const std::string& x,
    SrcInfo info, const std::string& s) {
  auto t= new Token(type, x, info);
  auto len= s.length();
  t->impl.value.cs = std::make_shared<a::CString>(len);
  t->impl.value.cs.get()->copy(s.c_str());
  return DslToken(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken token(int type,
    const std::string& s, SrcInfo info, llong n) {
  auto t= new Token(type, s, info);
  t->impl.value.u.n=n;
  return DslToken(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken token(int type,
    const std::string& s, SrcInfo info, double d) {
  auto t= new Token(type,s, info);
  t->impl.value.u.r=d;
  return DslToken(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Token::toString() {
  char buf[1024];
  ::sprintf(buf,
      "Token#{type = %d, text = %s}", type(), impl.text.c_str());
  return std::string(buf);
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
DslToken Lexer::number() {
  auto m= ctx.mark();
  auto s = d::numeric(ctx).c_str();
  return ::strchr(s, '.')
    ? token(d::T_REAL, s, m, ::atof(s))
    : token(d::T_INTEGER, s, m, (llong) ::atol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken Lexer::string() {
  auto m= ctx.mark();
  auto s = d::str(ctx);
  return token(d::T_STRING, s, m, s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken Lexer::id() {
  auto s= d::identifier(ctx);
  auto S= a::toupper(s);
  auto m= ctx.mark();
  return !isKeyword(S)
    ? token(d::T_IDENT, s, m, s)
    : DslToken(new Token(KEYWORDS.at(S), S, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DslToken Lexer::getNextToken() {
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
      auto m= ctx.mark();
      d::advance(ctx);
      return token(d::T_MULT, ch, m);
    }
    else
    if (ch == '/') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_DIV, ch, m);
    }
    else
    if (ch == '+') {
      auto m= ctx.mark();
      d::advance(ctx);
      return token(d::T_PLUS, ch, m);
    }
    else
    if (ch == '-') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_MINUS, ch, m);
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
    if (ch == '_' || ::isalpha(ch)) {
      return id();
    }
    else
    if (ch== ':' && '=' == d::peekNext(ctx)) {
      auto m= ctx.mark();
      d::advance(ctx);
      d::advance(ctx);
      return token(T_ASSIGN, ":=", m);
    }
    else
    if (ch== '=' && '=' == d::peekNext(ctx)) {
      auto m= ctx.mark();
      d::advance(ctx);
      d::advance(ctx);
      return token(T_EQUALS, "==", m);
    }
    else
    if (ch == '{') {
      d::advance(ctx);
      skipComment();
    }
    else
    if (ch == '<') {
      auto m=ctx.mark();
      if (d::peekNext(ctx) == '=') {
        d::advance(ctx);
        d::advance(ctx);
        return token(T_LTEQ, "<=", m);
      }
      else if (d::peekNext(ctx) == '>') {
        d::advance(ctx);
        d::advance(ctx);
        return token(T_NOTEQ, "<>", m);
      }
      else {
        d::advance(ctx);
        return token(d::T_LT, ch, m);
      }
    }
    else
    if (ch == '>') {
      auto m=ctx.mark();
      if (d::peekNext(ctx) == '=') {
        d::advance(ctx);
        d::advance(ctx);
        return token(T_GTEQ, ">=", m);
      } else {
        d::advance(ctx);
        return token(d::T_GT, ch, m);
      }
    }
    else
    if (ch == '|') {
      if (d::peekNext(ctx) == '|') {
        auto m=ctx.mark();
        d::advance(ctx);
        d::advance(ctx);
        return token(T_OR, "||", m);
      }
    }
    else
    if (ch == '&') {
      if (d::peekNext(ctx) == '&') {
        auto m=ctx.mark();
        d::advance(ctx);
        d::advance(ctx);
        return token(T_AND, "&&", m);
      }
    }
    else
    if (ch == ';') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_SEMI, ch, m);
    }
    else
    if (ch == '!') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(T_NOT, ch, m);
    }
    else
    if (ch == '~') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(T_XOR, ch, m);
    }
    else
    if (ch == ':') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_COLON, ch, m);
    }
    else
    if (ch == ',') {
      auto m= ctx.mark();
      d::advance(ctx);
      return token(d::T_COMMA, ch, m);
    }
    else
    if (ch == '.') {
      auto m=ctx.mark();
      d::advance(ctx);
      return token(d::T_DOT, ch, m);
    }
    else {
      raise(SyntaxError,
          "Unexpected char %c near line %d(%d).\n", ch, ctx.line, ctx.col);
    }
  }

  return token(d::T_EOF, "<EOF>", ctx.mark());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

