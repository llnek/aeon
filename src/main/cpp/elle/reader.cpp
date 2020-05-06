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
namespace czlab::elle {
namespace a = czlab::aeon;
namespace d = czlab::dsl;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> TOKENS {
  {T_SPLICE_UNQUOTE,",@"},
  {T_SYNTAX_QUOTE, "`"},
  {T_UNQUOTE, ","},
  {T_QUOTE, "'"},
  {T_CHAR, "#\\"},
  {T_VEC, "#()"},
  {T_LIST, "()"},
  {T_TRUE, "#t"},
  {T_FALSE, "#f"},
  {T_COMMENT, ";"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
auto KEYWORDS = a::map_reflect(TOKENS);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int type) {
  auto& i= d::getIntTokens();
  return s__contains(TOKENS, type)
         ? map__val(TOKENS, type)
         : (s__contains(i,type) ? map__val(i,type) : ("token#" + N_STR(type)));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Reader::Reader(const Tchar* src) {
  _ctx.len= ::strlen(src);
  _ctx.src=src;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Reader::isKeyword(cstdstr&) const {
  RAISE(d::Unsupported, "%s not allowed!", "isKeyword");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::skipComment() {
  auto m= _ctx.mark();
  stdstr s;
  while (!_ctx.eof)
  { d::advance(_ctx);
    auto c= d::peek(_ctx);
    if (c == '\n') break; else s += c; }
  return d::Token::make(d::T_COMMENT, s, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::number() {
  auto ch= d::peek(_ctx);
  bool minus=0;

  if (ch == '-' ||
      ch == '+') {
    minus= (ch == '-'); d::advance(_ctx); }

  auto res= d::numeric(_ctx);
  auto ds= _1(res);

  if (minus)
    ds = "-" + ds;

  auto cs= ds.c_str();
  return ::strchr(cs, '.')
         ? d::Token::make(ds, _2(res), ::atof(cs))
         : d::Token::make(ds, _2(res), (llong) ::atol(cs));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::string() {
  auto res= d::str(_ctx);
  return d::Token::make(_1(res), _2(res));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::id() {
  static stdstr bad("{}[]()'\"\\`@~^,.;");
  auto m= _ctx.mark();
  int t;
  stdstr res;
  while (!_ctx.eof) {
    auto ch=peek(_ctx);
    auto pos = bad.find(ch);
    if (::isspace(ch) ||
        pos != STDS_NPOS)
      break;
    else
    { res += ch;
      d::advance(_ctx);} }

  DEBUG("::id = %s", res.c_str());
  return d::Token::make(d::T_IDENT, res, m);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool testNumber(Tchar c) {
  switch (c) {
  case 'b':
  case 'o':
  case 'x':
  case 'i':
  case 'd':
  case 'e': return true;
  default: return false;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar readChar(d::Context& ctx) {
  auto ch= d::peek(ctx);
  if ((ch=='s' &&
       d::peekPattern(ctx, "space")) ||
      (ch=='S' &&
       d::peekPattern(ctx, "SPACE"))) {
    d::advance(ctx,5);
    ch= ' ';
  }
  else
  if ((ch=='t' &&
      d::peekPattern(ctx, "tab")) ||
      (ch=='T' &&
      d::peekPattern(ctx, "TAB"))) {
    d::advance(ctx,3);
    ch= '\t';
  }
  else
  if ((ch=='n' &&
      d::peekPattern(ctx, "newline")) ||
      (ch=='N' &&
      d::peekPattern(ctx, "NEWLINE"))) {
    d::advance(ctx,7);
    ch= '\n';
  }
  else
  if ((ch=='r' &&
      d::peekPattern(ctx, "return")) ||
      (ch=='R' &&
      d::peekPattern(ctx, "RETURN"))) {
    d::advance(ctx,6);
    ch= '\r';
  }
  else
    d::advance(ctx);

  return ch;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::getNextToken() {
  //auto& S= d::getStrTokens();
  while (!_ctx.eof) {
    auto ch= d::peek(_ctx);

    // ORDER IS IMPORTANT!
    if (::isspace(ch))
    { d::skipWhitespace(_ctx); continue; }

    if (ch == ',')
    { if (d::peekAhead(_ctx)=='@')
        return d::Token::make(T_SPLICE_UNQUOTE,
                              ",@",
                              d::mark_advance(_ctx,2));
      else
        return d::Token::make(T_UNQUOTE,
                              ch,d::mark_advance(_ctx)); }

    if (ch == ';')
    { skipComment(); continue; }

    if (ch=='\'')
      return d::Token::make(T_QUOTE,
                            ch, d::mark_advance(_ctx));

    if (ch=='`')
      return d::Token::make(T_SYNTAX_QUOTE,
                            ch, d::mark_advance(_ctx));

    if ((ch == '-' || ch == '+') &&
        '.'==d::peekAhead(_ctx) &&
        ::isdigit(d::peekAhead(_ctx,2)))
      return number();

    if ((ch == '-' || ch == '+') &&
        ::isdigit(d::peekAhead(_ctx)))
      return number();

    if (ch=='.' &&
        ::isdigit(d::peekAhead(_ctx)))
      return number();

    if (::isdigit(ch))
      return number();

    if (ch == '"')
      return string();

    if (ch=='#' && d::peekAhead(_ctx)=='t')
      return d::Token::make(T_TRUE,
                            "#t", d::mark_advance(_ctx,2));

    if (ch=='#' && d::peekAhead(_ctx)=='f')
      return d::Token::make(T_FALSE,
                            "#f", d::mark_advance(_ctx,2));

    if (ch == '#' &&
        d::peekAhead(_ctx) == '\\') {
      //expecting a char
      auto m= d::mark_advance(_ctx,2);
      auto c= readChar(_ctx);
      return c!='\0'
        ? d::Token::make(T_CHAR,c,m)
        : d::Token::make(d::T_EOF,"<eof>",m);
    }

    if (ch=='#' &&
        testNumber(d::peekAhead(_ctx))) {
      auto m= d::mark_advance(_ctx); // rid of #
      ch=d::pop(_ctx); // the number tag
      // parse the number
      RAISE(d::Unsupported,
            "#%c not supported near %s",
            ch, d::pr_addr(m).c_str());
    }

    if (ch=='#' &&
        d::peekAhead(_ctx) == '(')
      return d::Token::make(T_VEC,
                            "#(", d::mark_advance(_ctx, 2));


    if (ch == '(')
      return d::Token::make(T_LIST,
                            ch, d::mark_advance(_ctx));

    if (ch == ')')
      return d::Token::make(d::T_RPAREN,
                            ch, d::mark_advance(_ctx));

    if (ch == '.')
      return d::Token::make(d::T_DOT,
                            ch, d::mark_advance(_ctx));

    if (ch)
      return id();

    //else
    return d::Token::make(d::T_ROGUE,
                          ch, d::mark_advance(_ctx));
  }

  return d::Token::make(d::T_EOF, "<eof>", _ctx.mark());
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


