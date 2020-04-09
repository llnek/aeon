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
  {T_EOL, "<CR>"},
  {T_INPUT, "INPUT"},
  {T_PRINT, "PRINT"},
  {T_END, "END"},
  {T_RUN, "RUN"},
  {T_LET, "LET"},
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
static void error(const stdstr& expected, const Token* tkn) {
  RAISE(d::SyntaxError,
        "Expecting %s, got token#%d, near line %d(%d).\n",
        C_STR(expected),
        tkn->type(), tkn->srcInfo().first, tkn->srcInfo().second);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::typeToString(int type) {
  return s__contains(TOKENS, type)
         ? map__val(TOKENS,type)
         : ("token=" + std::to_string(type));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const stdstr& s, d::SrcInfo info) : AbstractToken(type) {
  _impl.txt= s;
  this->info = info;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, Tchar ch, d::SrcInfo info) : d::AbstractToken(type) {
  this->info = info;
  _impl.txt= stdstr { ch };
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Token::getLiteralAsReal() const {
  if (type() != d::T_REAL) { error("REAL", this); }
  return _impl.num.getFloat();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Token::getLiteralAsInt() const {
  if (type() != d::T_INTEGER) { error("INTEGER", this); }
  return _impl.num.getInt();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::getLiteralAsStr() const {
  if (type() == d::T_IDENT ||
      type() == d::T_STRING) { return _impl.txt; }

  if (! s__contains(TOKENS, type())) {
    //std::cout <<  "token is not ID,String,Keyword!!!!" << "\n";
    //error("ID,String,Keyword", this);
    //std::cout << "<<<" << pr_str() << ">>>";
    return pr_str();
  }

  return TOKENS.at(type());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, Tchar c, d::SrcInfo info) {
  return new Token(type, c, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& x, d::SrcInfo info) {
  return new Token(type, x, info);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& s, d::SrcInfo info, llong n) {
  auto t= new Token(type, s, info);
  t->impl().num.setInt(n);
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& s, d::SrcInfo info, double d) {
  auto t= new Token(type,s, info);
  t->impl().num.setFloat(d);
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::pr_str() const {
  return _impl.txt;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::Lexer(const char* src) {
  _ctx.len= ::strlen(src);
  _ctx.eof=false;
  _ctx.src=src;
  _ctx.line=0;
  _ctx.col=0;
  _ctx.pos=0;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lexer::isKeyword(const stdstr& k) const {
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
    if (c != '\n' && ::isspace(c))
      d::advance(ctx);
    else
      break;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::id() {
  auto m= _ctx.mark();
  auto s= d::identifier(_ctx, &filter);
  auto S= a::to_upper(s);
  return !isKeyword(S)
    ? token(d::T_IDENT, S, m)
    : token(KEYWORDS.at(S), S, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::getNextToken() {
  auto t = _getNextToken();
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::_getNextToken() {
  Tchar ch;
  while (!_ctx.eof) {
    ch= d::peek(_ctx);
    if (ch == '\r' &&
        d::peekNext(_ctx) == '\n') {
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
    if ((ch == '=' && d::peekNext(_ctx)== '>') ||
        (ch == '>' && d::peekNext(_ctx)== '=')) {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_GTEQ, ch, m);
    }
    else
    if ((ch == '=' && d::peekNext(_ctx)== '<') ||
        (ch == '<' && d::peekNext(_ctx)== '=')) {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_LTEQ, ch, m);
    }
    else
    if ((ch == '>' && d::peekNext(_ctx)== '<') ||
        (ch == '<' && d::peekNext(_ctx)== '>')) {
      auto m= _ctx.mark();
      d::advance(_ctx,2);
      return token(T_NOTEQ, ch, m);
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
      RAISE(d::SyntaxError,
          "Unexpected char %c near line %d(%d).\n", ch, _ctx.line, _ctx.col);
    }
  }

  return token(d::T_EOF, "<EOF>", _ctx.mark());
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


