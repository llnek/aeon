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

auto KEYWORDS= a::map_reflect(TOKENS);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int type) {
  return s__contains(TOKENS, type)
         ? map__val(TOKENS,type)
         : ("token-type=" + N_STR(type));
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
  while (!_ctx.eof)
  { auto ch= d::pop(_ctx);
    if (ch == '}') break; else out += ch; }
  return d::Token::make(d::T_COMMENT,out, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::number() {
  auto res = d::numeric(_ctx);
  auto cs= _1(res).c_str();
  return ::strchr(cs, '.')
    ? d::Token::make(cs, _2(res), ::atof(cs))
    : d::Token::make(cs, _2(res), (llong) ::atol(cs));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::string() {
  auto res = d::str(_ctx);
  return d::Token::make(_1(res), _2(res));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool lexer_id(Tchar ch, bool first) {
  if (first)
    return (ch=='_' || ::isalpha(ch));
  else
    return (ch=='_' || ::isalpha(ch) || isdigit(ch));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::id() {
  auto res= d::identifier(_ctx, &lexer_id);
  auto S= a::to_upper(_1(res));
  return !isKeyword(S)
         ? d::Token::make(d::T_IDENT, _1(res), _2(res))
         : d::Token::make(KEYWORDS.at(S), S, _2(res));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::getNextToken() {
  while (!_ctx.eof) {
    auto ch= d::peek(_ctx);

    if (::isspace(ch))
    { d::skipWhitespace(_ctx); continue; }

    if (::isdigit(ch)) return number();

    if (ch == '"') return string();

    if (ch == '*')
      return d::Token::make(d::T_MULT,
                            ch, d::mark_advance(_ctx));

    if (ch == '/')
      return d::Token::make(d::T_DIV,
                            ch, d::mark_advance(_ctx));

    if (ch == '+')
      return d::Token::make(d::T_PLUS,
                            ch,
                            d::mark_advance(_ctx));

    if (ch == '-')
      return d::Token::make(d::T_MINUS,
                            ch, d::mark_advance(_ctx));

    if (ch == '(')
      return d::Token::make(d::T_LPAREN,
                            ch, d::mark_advance(_ctx));

    if (ch == ')')
      return d::Token::make(d::T_RPAREN,
                            ch, d::mark_advance(_ctx));

    if (lexer_id(ch,true)) return id();

    if (ch== ':' &&
        '=' == d::peekAhead(_ctx))
      return d::Token::make(T_ASSIGN,
                            ":=", d::mark_advance(_ctx,2));

    if (ch== '=' &&
        '=' == d::peekAhead(_ctx))
      return d::Token::make(T_EQUALS,
                            "==", d::mark_advance(_ctx,2));

    if (ch == '{')
    { d::advance(_ctx);
      skipComment(); continue; }

    if (ch == '<')
    { if (d::peekAhead(_ctx) == '=')
        return d::Token::make(T_LTEQ,
                              "<=", d::mark_advance(_ctx,2));
      if (d::peekAhead(_ctx) == '>')
        return d::Token::make(T_NOTEQ,
                              "<>", d::mark_advance(_ctx,2));
      return d::Token::make(d::T_LT, ch, d::mark_advance(_ctx)); }

    if (ch == '>')
    { if (d::peekAhead(_ctx) == '=')
        return d::Token::make(T_GTEQ,
                              ">=", d::mark_advance(_ctx,2));
      else
        return d::Token::make(d::T_GT, ch, d::mark_advance(_ctx)); }

    if (ch == '|' &&
        d::peekAhead(_ctx) == '|')
      return d::Token::make(T_OR, "||", d::mark_advance(_ctx,2));

    if (ch == '&' &&
        d::peekAhead(_ctx) == '&')
      return d::Token::make(T_AND, "&&", d::mark_advance(_ctx,2));

    if (ch == ';')
      return d::Token::make(d::T_SEMI, ch, d::mark_advance(_ctx));

    if (ch == '!')
      return d::Token::make(T_NOT, ch, d::mark_advance(_ctx));

    if (ch == '~')
      return d::Token::make(T_XOR, ch, d::mark_advance(_ctx));

    if (ch == ':')
      return d::Token::make(d::T_COLON, ch, d::mark_advance(_ctx));

    if (ch == ',')
      return d::Token::make(d::T_COMMA, ch, d::mark_advance(_ctx));

    if (ch == '.')
      return d::Token::make(d::T_DOT, ch, d::mark_advance(_ctx));

    //else
    return d::Token::make(d::T_ROGUE, ch, d::mark_advance(_ctx));
  }

  return d::Token::make(d::T_EOF, "<EOF>", _ctx.mark());
}








//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

