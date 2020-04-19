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
std::map<stdstr,int> KEYWORDS {
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
         ? map__val(TOKENS, t) : ("token=" + N_STR(t));
}

////;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//static void error(cstdstr& expected, const BToken* tkn) {
//  auto i= tkn->marker();
//  RAISE(d::SyntaxError,
//        "Expecting %s, got token#%d, near line %d(%d).",
//        C_STR(expected), tkn->type(), i.first, i.second);
//}
//
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr BToken::getStr() const {
  return (type() == d::T_IDENT ||
          type() == d::T_STRING)
    ? lexeme
    : (s__contains(TOKENS,type()) ? TOKENS.at(type()) : lexeme);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int t, cstdstr& x, d::Mark info) {
  return BToken::make(t, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int t, Tchar c, d::Mark info) {
  return BToken::make(t, c, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int t, cstdstr& s, d::Mark info, llong n) {
  auto b= BToken::make(t, s, info);
  DCAST(BToken, b)->setLiteral(n);
  //std::cout << "token num = " << i.num.getInt() << "\n";
  return b;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int t, cstdstr& s, d::Mark info, double d) {
  auto b= BToken::make(t,s, info);
  DCAST(BToken,b)->setLiteral(d);
  return b;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::Lexer(const Tchar* src) {
  _ctx.len= ::strlen(src);
  _ctx.eof=false;
  _ctx.src=src;
  _ctx.line=0;
  _ctx.col=0;
  _ctx.pos=0;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lexer::isKeyword(cstdstr& k) const {
  return s__contains(KEYWORDS, k);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::skipComment() {
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
d::DslToken Lexer::number() {
  auto m= _ctx.mark();
  auto s = C_STR(d::numeric(_ctx));
  return ::strchr(s, '.')
    ? token(d::T_REAL, s, m, ::atof(s))
    : token(d::T_INTEGER, s, m, (llong) ::atol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::string() {
  auto m= _ctx.mark();
  return token(d::T_STRING, d::str(_ctx), m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool filter(Tchar ch, bool first) {
  if (first) {
    return (ch == '_' || ::isalpha(ch));
  } else {
    return (ch == '_' || ch == '$' || ::isalpha(ch) || ::isdigit(ch));
  }
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
d::DslToken Lexer::id() {
  auto m= _ctx.mark();
  auto s= d::identifier(_ctx, &filter);
  auto S= a::to_upper(s);
  if (isKeyword(S)) {
    return token(KEYWORDS.at(S), S, m);
  } else {
    auto p= ::strchr(S.c_str(), '$');
    if (p) {
      ASSERT(*(p+1)=='\0',
             "Malformed var name %s.", C_STR(s));
    }
    return token(d::T_IDENT, S, m);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::getNextToken() {
  Tchar ch;
  while (!_ctx.eof) {
    ch= d::peek(_ctx);
    if (ch == '\r' &&
        d::peekAhead(_ctx) == '\n') {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_EOL, "<EOL>", m);
    }
    else
    if (ch == '\n') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(T_EOL, "<EOL>", m);
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
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_MULT, ch, m);
    }
    else
    if (ch == '/') {
      auto m= _ctx.mark();
      d::advance( _ctx);
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
      d::advance( _ctx);
      return token(d::T_LPAREN, ch, m);
    }
    else
    if (ch == ')') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_RPAREN, ch, m);
    }
    else
    if (filter(ch,true)) {
      return id();
    }
    else
    if (ch == '^') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(T_POWER, ch, m);
    }
    else
    if ((ch == '=' && d::peekAhead(_ctx)== '>') ||
        (ch == '>' && d::peekAhead(_ctx)== '=')) {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_GTEQ, ch, m);
    }
    else
    if ((ch == '=' && d::peekAhead(_ctx)== '<') ||
        (ch == '<' && d::peekAhead(_ctx)== '=')) {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_LTEQ, ch, m);
    }
    else
    if ((ch == '>' && d::peekAhead(_ctx)== '<') ||
        (ch == '<' && d::peekAhead(_ctx)== '>')) {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_NOTEQ, ch, m);
    }
    else
    if (ch == '>') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_GT, ch, m);
    }
    else
    if (ch == '<') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_LT, ch, m);
    }
    else
    if (ch == '=') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_EQ, ch, m);
    }
    else
    if (ch == '{') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_LBRACE, ch, m);
    }
    else
    if (ch == '}') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_RBRACE, ch, m);
    }
    else
    if (ch == ';') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_SEMI, ch, m);
    }
    else
    if (ch == ':') {
      auto m=_ctx.mark();
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
      //RAISE(d::SyntaxError,
            //"Unexpected char %c near line %d(%d).", ch, _ctx.line, _ctx.col);
    }
  }

  return token(d::T_EOF, "<EOF>", _ctx.mark());
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


