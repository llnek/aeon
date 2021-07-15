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
namespace czlab::spi{
namespace a= czlab::aeon;
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//map, keyword-type -> keyword-string
std::map<int, stdstr> TOKENS {
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
auto KEYWORDS=a::map_reflect(TOKENS);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int type){
  return s__contains(TOKENS, type) ? map__val(TOKENS,type) : ("token=" + N_STR(type)); }
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
d::DToken Lexer::skipComment()
{
  auto m= _ctx.mark();
  stdstr out;
  while(!_ctx.eof){
    auto ch= d::pop(_ctx);
    if(ch == '}') break; else out += ch; }
  return d::Token::make(d::T_COMMENT, out, m); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::number(){
  auto res = d::numeric(_ctx);
  auto cs=_1(res).c_str();
  return ::strchr(cs, '.') ? d::Token::make(_1(res), _2(res), ::atof(cs))
                           : d::Token::make(_1(res), _2(res), (llong)::atol(cs)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::string(){
  auto res= d::str(_ctx);
  return d::Token::make(_1(res),_2(res)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool filter(Tchar ch, bool first)
{
  if(first)
    return (ch == '_' || ::isalpha(ch));
  else
    return (ch == '_' || ::isalpha(ch) || ::isdigit(ch)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::id(){
  auto res= d::identifier(_ctx, &filter);
  auto S= a::to_upper(_1(res));
  return isKeyword(S) ? d::Token::make(KEYWORDS.at(S), S, _2(res))
                      : d::Token::make(d::T_IDENT, _1(res), _2(res)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Lexer::getNextToken(){
  while(!_ctx.eof){
    auto ch= d::peek(_ctx);
    if(::isspace(ch)){
      d::skipWhitespace(_ctx);
    }else if(::isdigit(ch)){
      return number();
    }else if(ch == '"'){
      return string();
    }else if(ch == '*'){
      return d::Token::make(d::T_MULT,
                            ch, d::mark_advance(_ctx));
    }else if(ch == '/'){
      return d::Token::make(d::T_DIV,
                            ch, d::mark_advance( _ctx));
    }else if(ch == '+'){
      return d::Token::make(d::T_PLUS,
                            ch, d::mark_advance(_ctx));
    }else if(ch == '-'){
      return d::Token::make(d::T_MINUS,
                            ch, d::mark_advance(_ctx));
    }else if(ch == '('){
      return d::Token::make(d::T_LPAREN,
                            ch, d::mark_advance( _ctx));
    }else if(ch == ')'){
      return d::Token::make(d::T_RPAREN,
                            ch, d::mark_advance(_ctx));
    }else if(filter(ch,true)){
      return id();
    }else if(ch== ':' && '=' == d::peekAhead(_ctx)){
      return d::Token::make(T_ASSIGN,
                            ":=", d::mark_advance(_ctx,2));
    }else if(ch == '{'){
      d::advance(_ctx);
      skipComment();
    }else if(ch == ';'){
      return d::Token::make(d::T_SEMI,
                            ch, d::mark_advance(_ctx));
    }else if(ch == ':'){
      return d::Token::make(d::T_COLON,
                            ch, d::mark_advance(_ctx));
    }else if(ch == ','){
      return d::Token::make(d::T_COMMA,
                            ch, d::mark_advance(_ctx));
    }else if(ch == '.'){
      return d::Token::make(d::T_DOT,
                            ch, d::mark_advance(_ctx));
    }else{
      return d::Token::make(d::T_ROGUE,
                            ch, d::mark_advance(_ctx));
    }
  }
  return d::Token::make(d::T_EOF,"<eof>",_ctx.mark());
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

