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

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> TOKENS {
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
std::map<stdstr,int> KEYWORDS {
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
stdstr typeToString(int type) {
  return s__contains(TOKENS, type)
         ? map__val(TOKENS,type)
         : stdstr("token-type=") + N_STR(type);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double SToken::getFloat() const {
  return type() == d::T_REAL ? number.r : number.n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong SToken::getInt() const {
  return type() == d::T_INTEGER ? number.n : number.r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr SToken::getStr() const {
  return s__contains(TOKENS,type())  ? TOKENS.at(type()) : lexeme;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token(int t, cstdstr& s, d::Mark info) {
  return SToken::make(t, s, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token(int t, Tchar c, d::Mark info) {
  return SToken::make(t, c, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token_long(cstdstr& s, d::Mark info) {
  auto k= SToken::make(d::T_INTEGER, s, info);
  llong n = ::atol(s.c_str());
  DCAST(SToken,k)->setLiteral(n);
  return k;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token_real(cstdstr& s, d::Mark info) {
  auto k= SToken::make(d::T_REAL, s, info);
  auto d= ::atof(s.c_str());
  DCAST(SToken,k)->setLiteral(d);
  return k;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::Lexer(const Tchar* src) {
  _ctx.len= ::strlen(src);
  _ctx.src=src;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lexer::isKeyword(cstdstr& k) const {
  return s__contains(KEYWORDS, k);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::skipComment() {
  auto m= _ctx.mark();
  stdstr out;
  while (!_ctx.eof) {
    auto ch= d::peek(_ctx);
    d::advance(_ctx);
    if (ch == '}')
    break;
    out += ch;
  }
  return token(d::T_COMMENT,out, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::number() {
  auto m= _ctx.mark();
  auto res = d::numeric(_ctx);
  return ::strchr(res.first.c_str(), '.')
    ? token_real(res.first, m)
    : token_long(res.first, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::string() {
  auto res = d::str(_ctx);
  return token(d::T_STRING, res.first, res.second);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool lexer_id(Tchar ch, bool first) {
  if (first) {
    return (ch=='_' || ::isalpha(ch));
  } else {
    return (ch=='_' || ::isalpha(ch) || isdigit(ch));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::id() {
  auto res= d::identifier(_ctx, &lexer_id);
  auto S= a::to_upper(res.first);
  return !isKeyword(S)
         ? token(d::T_IDENT, res.first, res.second)
         : token(KEYWORDS.at(S), S, res.second);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::getNextToken() {
  Tchar ch;
  while (!_ctx.eof) {
    ch= d::peek(_ctx);
    if (::isspace(ch)) {
      d::skipWhitespace(_ctx);
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
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_MULT, ch, m);
    }
    else
    if (ch == '/') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_DIV, ch, m);
    }
    else
    if (ch == '+') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_PLUS, ch, m);
    }
    else
    if (ch == '-') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_MINUS, ch, m);
    }
    else
    if (ch == '(') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_LPAREN, ch, m);
    }
    else
    if (ch == ')') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_RPAREN, ch, m);
    }
    else
    if (lexer_id(ch,true)) {
      return id();
    }
    else
    if (ch== ':' && '=' == d::peekAhead(_ctx)) {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_ASSIGN, ":=", m);
    }
    else
    if (ch== '=' && '=' == d::peekAhead(_ctx)) {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_EQUALS, "==", m);
    }
    else
    if (ch == '{') {
      d::advance(_ctx);
      skipComment();
    }
    else
    if (ch == '<') {
      auto m= _ctx.mark();
      if (d::peekAhead(_ctx) == '=') {
        d::advance(_ctx,2);
        return token(T_LTEQ, "<=", m);
      }
      else if (d::peekAhead(_ctx) == '>') {
        d::advance(_ctx,2);
        return token(T_NOTEQ, "<>", m);
      }
      else {
        d::advance(_ctx);
        return token(d::T_LT, ch, m);
      }
    }
    else
    if (ch == '>') {
      auto m= _ctx.mark();
      if (d::peekAhead(_ctx) == '=') {
        d::advance(_ctx,2);
        return token(T_GTEQ, ">=", m);
      } else {
        d::advance(_ctx);
        return token(d::T_GT, ch, m);
      }
    }
    else
    if (ch == '|') {
      if (d::peekAhead(_ctx) == '|') {
        auto m= _ctx.mark();
        d::advance(_ctx,2);
        return token(T_OR, "||", m);
      }
    }
    else
    if (ch == '&') {
      if (d::peekAhead(_ctx) == '&') {
        auto m= _ctx.mark();
        d::advance(_ctx,2);
        return token(T_AND, "&&", m);
      }
    }
    else
    if (ch == ';') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_SEMI, ch, m);
    }
    else
    if (ch == '!') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(T_NOT, ch, m);
    }
    else
    if (ch == '~') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(T_XOR, ch, m);
    }
    else
    if (ch == ':') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_COLON, ch, m);
    }
    else
    if (ch == ',') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_COMMA, ch, m);
    }
    else
    if (ch == '.') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_DOT, ch, m);
    }
    else {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_ROGUE, ch, m);
    }
  }

  return token(d::T_EOF, "<EOF>", _ctx.mark());
}








//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

