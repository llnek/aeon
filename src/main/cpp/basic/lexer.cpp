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
namespace czlab::basic {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> TOKENS {
  {T_ARRAYINDEX, "[]"},
  {T_REM, "REM"},
  {T_DEF, "DEF"},
  {T_EOL, "<CR>"},
  {T_INPUT, "INPUT"},
  {T_PRINT, "PRINT"},
  {T_END, "END"},
  {T_RUN, "RUN"},
  {T_LET, "LET"},
  {T_ON, "ON"},
  {T_IF, "IF"},
  {T_THEN, "THEN"},
  {T_ELSE, "ELSE"},
  {T_GOTO, "GOTO"},
  {T_FOR, "FOR"},
  {T_TO, "TO"},
  {T_NEXT, "NEXT"},
  {T_STEP, "STEP"},
  {T_READ, "READ"},
  {T_DATA, "DATA"},
  {T_GOSUB, "GOSUB"},
  {T_RETURN, "RETURN"},
  {T_INT_DIV, "DIV"},
  {d::T_COMMA, ","},
  {d::T_SEMI, ";"},
  {T_MOD, "MOD"},
  {T_AND, "AND"},
  {T_OR, "OR"},
  {T_XOR, "XOR"},
  {T_DIM, "DIM"},
  {T_RESTORE, "RESTORE"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<stdstr,int> KWDS {
  {map__val(TOKENS,T_ARRAYINDEX), T_ARRAYINDEX},
  {map__val(TOKENS,T_ON), T_ON},
  {map__val(TOKENS,T_DEF), T_DEF},
  {map__val(TOKENS,T_INT_DIV), T_INT_DIV},
  {map__val(TOKENS,d::T_SEMI), d::T_SEMI},
  {map__val(TOKENS,d::T_COMMA), d::T_COMMA},
  {map__val(TOKENS,T_INPUT), T_INPUT},
  {map__val(TOKENS,T_PRINT), T_PRINT},
  {map__val(TOKENS,T_STEP), T_STEP},
  {map__val(TOKENS,T_REM), T_REM},
  {map__val(TOKENS,T_END), T_END},
  {map__val(TOKENS,T_EOL), T_EOL},
  {map__val(TOKENS,T_TO), T_TO},
  {map__val(TOKENS,T_RUN), T_RUN},
  {map__val(TOKENS,T_LET), T_LET},
  {map__val(TOKENS,T_IF), T_IF},
  {map__val(TOKENS,T_THEN), T_THEN},
  {map__val(TOKENS,T_ELSE), T_ELSE},
  {map__val(TOKENS,T_GOTO), T_GOTO},
  {map__val(TOKENS,T_FOR), T_FOR},
  {map__val(TOKENS,T_NEXT), T_NEXT},
  {map__val(TOKENS,T_READ), T_READ},
  {map__val(TOKENS,T_DATA), T_DATA},
  {map__val(TOKENS,T_GOSUB), T_GOSUB},
  {map__val(TOKENS,T_RETURN), T_RETURN},
  {map__val(TOKENS,T_MOD), T_MOD},
  {map__val(TOKENS,T_AND), T_AND},
  {map__val(TOKENS,T_OR), T_OR},
  {map__val(TOKENS,T_XOR), T_XOR},
  {map__val(TOKENS,T_DIM), T_DIM},
  {map__val(TOKENS,T_RESTORE), T_RESTORE}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int t) {
  return s__contains(TOKENS, t)
         ? map__val(TOKENS, t) : ("token#" + N_STR(t));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BToken::BToken(int type, cstdstr& s, d::Mark info) : d::Lexeme(type, info) {
  lexeme= s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BToken::BToken(int type, Tchar ch, d::Mark info) : d::Lexeme(type, info) {
  lexeme= stdstr { ch };
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double BToken::getFloat() const {
  return type() == d::T_REAL ? number.r : number.n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong BToken::getInt() const {
  return type() == d::T_INTEGER ? number.n : number.r;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BToken::getStr() const {
  auto t=type();
  return (t == d::T_IDENT ||
          t == d::T_STRING)
    ? lexeme : (s__contains(TOKENS,t) ? TOKENS.at(t) : lexeme);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BToken::pr_str() const { return lexeme; }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token(int t, cstdstr& x, d::Mark info) {
  return BToken::make(t, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token(int t, Tchar c, d::Mark info) {
  return BToken::make(t, c, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token(d::Context& ctx) {
  return token(d::T_EOF, "<EOF>", ctx.mark());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token_eol(d::Mark m) {
  return token(T_EOL, "<EOL>", m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token_str(cstdstr& x, d::Mark info) {
  return BToken::make(d::T_STRING, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token_long(cstdstr& s, d::Mark info) {
  auto b= BToken::make(d::T_INTEGER, s, info);
  auto n= ::atol(s.c_str());
  DCAST(BToken, b)->setLiteral((llong)n);
  return b;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken token_real(cstdstr& s, d::Mark info) {
  auto b= BToken::make(d::T_REAL,s, info);
  auto d= ::atof(s.c_str());
  DCAST(BToken,b)->setLiteral(d);
  return b;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::Lexer(const Tchar* src) {
  _ctx.len= ::strlen(src);
  _ctx.src=src;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lexer::isKeyword(cstdstr& k) const {
  return s__contains(KWDS, k);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::skipComment() {
  auto m= _ctx.mark();
  stdstr out;
  while (!_ctx.eof) {
    auto ch= d::peek(_ctx);
    d::advance(_ctx);
    if (ch == '\n')
    break;
    out += ch;
  }
  return token(d::T_COMMENT, out, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::number() {
  auto res = d::numeric(_ctx);
  return ::strchr(res.first.c_str(), '.')
         ? token_real(res.first, res.second) : token_long(res.first, res.second);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::string() {
  auto res= d::str(_ctx);
  return token_str(res.first, res.second);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool filter(Tchar ch, bool first) {
  return first
    ? (ch == '_' || ::isalpha(ch))
    : (ch == '_' || ch == '$' || ::isalpha(ch) || ::isdigit(ch));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skip_wspace(d::Context& ctx) {
  while (!ctx.eof) {
    auto c= peek(ctx);
    if (c != '\n' &&
        ::isspace(c)) d::advance(ctx); else break;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void checkid(cstdstr& src, d::Mark m) {
  auto cs= src.c_str();
  auto s= ::strchr(cs, '$');// string var
  auto i= ::strchr(cs, '%');// integer var
  auto d= ::strchr(cs, '#');// double var
  auto f= ::strchr(cs, '!');// float var
  if ((s && *(s+1) != '\0') ||
      (i && *(i+1) != '\0') ||
      (f && *(f+1) != '\0') ||
      (d && *(d+1) != '\0')) {
    E_SYNTAX("Naming error: `%s` near line %d:%d.", cs, m.first,m.second);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::id() {
  auto res = d::identifier(_ctx, &filter);
  auto S= a::to_upper(res.first);

  if (isKeyword(S)) {
    return token(KWDS.at(S), S, res.second);
  } else {
    checkid(S.c_str(), res.second);
    return token(d::T_IDENT, S, res.second);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::getNextToken() {
  while (!_ctx.eof) {
    auto ch= d::peek(_ctx);
    // ORDER IS IMPORTANT !!!!
    if (ch == '\n') {
      return token_eol(d::mark_advance(_ctx));
    }
    else
    if (ch == '\r' &&
        d::peekAhead(_ctx) == '\n') {
      return token_eol(d::mark_advance(_ctx,2));
    }
    else
    if (::isspace(ch)) {
      skip_wspace(_ctx);
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
      return token(d::T_MULT, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '/') {
      return token(d::T_DIV, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '+') {
      return token(d::T_PLUS, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '-') {
      return token(d::T_MINUS, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '(') {
      return token(d::T_LPAREN, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == ')') {
      return token(d::T_RPAREN, ch, d::mark_advance(_ctx));
    }
    else
    if (filter(ch,true)) {
      return id();
    }
    else
    if (ch == '^') {
      return token(T_POWER, ch, d::mark_advance(_ctx));
    }
    else
    if ((ch == '=' && d::peekAhead(_ctx)== '>') ||
        (ch == '>' && d::peekAhead(_ctx)== '=')) {
      return token(T_GTEQ, ">=", d::mark_advance(_ctx,2));
    }
    else
    if ((ch == '=' && d::peekAhead(_ctx)== '<') ||
        (ch == '<' && d::peekAhead(_ctx)== '=')) {
      return token(T_LTEQ, "<=", d::mark_advance(_ctx,2));
    }
    else
    if ((ch == '>' && d::peekAhead(_ctx)== '<') ||
        (ch == '<' && d::peekAhead(_ctx)== '>')) {
      return token(T_NOTEQ, "<>", d::mark_advance(_ctx,2));
    }
    else
    if (ch == '>') {
      return token(d::T_GT, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '<') {
      return token(d::T_LT, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '=') {
      return token(d::T_EQ, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '{') {
      return token(d::T_LBRACE, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '}') {
      return token(d::T_RBRACE, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == ';') {
      return token(d::T_SEMI, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == ':') {
      return token(d::T_COLON, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == ',') {
      return token(d::T_COMMA, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '.') {
      return token(d::T_DOT, ch, d::mark_advance(_ctx));
    }
    else
    if (ch == '\'') {
      // quote is short-hand for comment
      return token(T_REM, "REM", d::mark_advance(_ctx));
    }
    else {
      return token(d::T_ROGUE, ch, d::mark_advance(_ctx));
    }
  }
  return token(_ctx);
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


