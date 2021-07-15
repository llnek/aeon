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
 * Copyright © 2013-2021, Kenneth Leung. All rights reserved. */

#include "lexer.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic{
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> TOKENS{
  {T_ARRAYINDEX, "[]"},
  {T_FUNCALL, "()"},
  {T_REM, "REM"},
  {T_DEF, "DEF"},
  {T_EOL, "<CR>"},
  {T_INPUT, "INPUT"},
  {T_PRINT, "PRINT"},
  {T_PRINTLN, "PRINTLN"},
  {T_END, "END"},
  {T_RUN, "RUN"},
  {T_LET, "LET"},
  {T_NOT, "NOT"},
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
  {T_MOD, "MOD"},
  {T_AND, "AND"},
  {T_OR, "OR"},
  {T_XOR, "XOR"},
  {T_DIM, "DIM"},
  {T_RESTORE, "RESTORE"}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
auto KEYWORDS= a::map_reflect(TOKENS);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int t){
  return s__contains(TOKENS, t) ? map__val(TOKENS, t) : ("token#" + N_STR(t)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::Lexer(const Tchar* src){
  _ctx.len= ::strlen(src);
  _ctx.src=src;
  _ctx.cur= getNextToken();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lexer::isKeyword(cstdstr& k) const{
  return s__contains(KEYWORDS, k);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::skipComment(){
  auto m= _ctx.mark();
  stdstr out;

  while(!_ctx.eof){
    auto ch= d::pop(_ctx);
    if(ch == '\n') break; else out += ch; }

  return d::Token::make(d::T_COMMENT, out, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::number(){
  auto res = d::numeric(_ctx);
  auto cs= _1(res).c_str();
  return ::strchr(cs, '.') ? d::Token::make(cs, _2(res), ::atof(cs))
                           : d::Token::make(cs, _2(res), (llong)::atol(cs)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::string(){
  auto res= d::str(_ctx);
  return d::Token::make(_1(res), _2(res));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool filter(Tchar ch, bool first){
  return first ? (ch == '_' || ::isalpha(ch))
               : (ch == '_' || ch == '$' || ch == '#' ||
                  ch == '!' || ch == '%' || ::isalpha(ch) || ::isdigit(ch)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skip_wspace(d::Context& ctx){
  while(!ctx.eof){
    auto c= peek(ctx);
    if(c != '\n' && ::isspace(c)) d::advance(ctx); else break; } }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void checkid(cstdstr& src, d::Addr m){
  auto cs= src.c_str();
  auto s= ::strchr(cs, '$');// string var
  auto i= ::strchr(cs, '%');// integer var
  auto d= ::strchr(cs, '#');// double var
  auto f= ::strchr(cs, '!');// float var
  if((s && *(s+1) != '\0') ||
     (i && *(i+1) != '\0') ||
     (f && *(f+1) != '\0') ||
     (d && *(d+1) != '\0')){
    E_SYNTAX("Bad name `%s` near %s.", cs, d::pr_addr(m).c_str()); } }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::id(){
  auto res = d::identifier(_ctx, &filter);
  auto S= a::to_upper(_1(res));
  auto b= isKeyword(S);

  if(!b)
    checkid(S, _2(res));

  return d::Token::make(b ? KEYWORDS.at(S) : d::T_IDENT, S, _2(res)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::getNextToken(){
  auto& tks= d::getStrTokens();
  while(!_ctx.eof){
    auto ch= d::peek(_ctx);
    // ORDER IS IMPORTANT !!!!
    //
    if(ch == '\n' ||
       (ch == '\r' &&
        d::peekAhead(_ctx) == '\n'))
      return d::Token::make(T_EOL,"<eol>",d::mark_advance(_ctx, ch=='\r' ? 2 : 1));

    if(::isspace(ch)){
      skip_wspace(_ctx);
      continue;
    }

    if(::isdigit(ch))
      return number();

    if(ch=='.' &&
       ::isdigit(d::peekAhead(_ctx))){
      return number();
    }

    if(::isdigit(ch)) return number();

    if(ch == '"') return string();

    if(ch == '*' ||
       ch == '/' ||
       ch == '+' ||
       ch == '-' ||
       ch == '(' ||
       ch == ')'){
      return d::Token::make(tks.at(stdstr(1,ch)),
                            ch,d::mark_advance(_ctx)); }

    if(filter(ch,true)) return id();

    if((ch == '=' && d::peekAhead(_ctx)== '>') ||
       (ch == '>' && d::peekAhead(_ctx)== '='))
      return d::Token::make(T_GTEQ, ">=", d::mark_advance(_ctx,2));

    if((ch == '=' && d::peekAhead(_ctx)== '<') ||
       (ch == '<' && d::peekAhead(_ctx)== '='))
      return d::Token::make(T_LTEQ, "<=", d::mark_advance(_ctx,2));

    if((ch == '>' && d::peekAhead(_ctx)== '<') ||
       (ch == '<' && d::peekAhead(_ctx)== '>'))
      return d::Token::make(T_NOTEQ, "<>", d::mark_advance(_ctx,2));

    if(ch == '^' ||
       ch == '>' ||
       ch == '<' ||
       ch == '=' ||
       ch == '{' ||
       ch == '}' ||
       ch == ';' ||
       ch == ':' ||
       ch == ',' ||
       ch == '\'' ||
       ch == '.'){
      return d::Token::make(tks.at(stdstr(1,ch)),
                            ch, d::mark_advance(_ctx)); }

    //else
    return d::Token::make(d::T_ROGUE,
                          ch, d::mark_advance(_ctx));
  }
  return d::Token::make(d::T_EOF,"<eof>",_ctx.mark());
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


