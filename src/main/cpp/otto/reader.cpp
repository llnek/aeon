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

#include "reader.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::otto{
namespace a= czlab::aeon;
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//special tokens
std::map<int, stdstr> TOKENS{
  {T_SPLICE_UNQUOTE, "~@"},
  {T_ANONFN, "#("},
  {T_SET, "#{"},
  {T_TRUE,"true"},
  {T_FALSE,"false"},
  {T_NIL,"nil"}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
auto KEYWORDS = a::map_reflect(TOKENS);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr typeToString(int type){
  auto& i= d::getIntTokens();//get base tokens
  return s__contains(TOKENS, type)
         ? map__val(TOKENS, type)
         : (s__contains(i,type) ? map__val(i,type) : ("token#" + N_STR(type))); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Reader::Reader(const Tchar* src){
  //LOG("ReaderInput: %s", src);
  _ctx.len= ::strlen(src);
  _ctx.src=src;
  _ctx.cur= getNextToken();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//this is not a clojure keyword, rather, it is a typical language keyword like
//`class`.  we don't support such here.
bool Reader::isKeyword(cstdstr&) const{
  RAISE(d::Unsupported, "%s not allowed!", "isKeyword"); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//handle comments
d::DToken Reader::skipComment(){
  auto m= _ctx.mark();
  stdstr s;
  //skip till end of line
  while(!_ctx.eof){
    d::advance(_ctx);
    auto c= d::peek(_ctx);
    if(c == '\n')
      break;
    s += c;
  }
  return d::Token::make(d::T_COMMENT, s, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//handle numeric stuff
d::DToken Reader::number(){
  auto ch= d::peek(_ctx);
  bool minus=0;
  if(ch == '-' || ch == '+'){
    minus= (ch == '-'); d::advance(_ctx); }
  auto res= d::numeric(_ctx);
  auto ds= _1(res);
  if(minus)
    ds = "-" + ds;
  auto cs= ds.c_str();
  return ::strchr(cs, '.') ? d::Token::make(ds, _2(res), ::atof(cs))
                           : d::Token::make(ds, _2(res), (llong) ::atol(cs)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//handle strings
d::DToken Reader::string(){
  auto res= d::str(_ctx);
  return d::Token::make(_1(res), _2(res)); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//true if part of a keyword
bool isKeywdChar(Tchar ch){
  return ch == '_' || ch == '-' || ch == '/' || ::isalpha(ch) || ::isdigit(ch); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//TODO: handle scoped keyword
d::DToken Reader::keywd(){
  auto m= d::mark_advance(_ctx); // skip the colon
  stdstr res;
  while(!_ctx.eof){
    auto ch=peek(_ctx);
    if(!isKeywdChar(ch))
      break;
    res += ch;
    advance(_ctx);
  }
  return d::Token::make(T_KEYWORD, res, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//handle symbols
d::DToken Reader::id(){
  static stdstr bad("{}[]()'\"\\`@~^,.;");
  auto m= _ctx.mark();
  int t;
  stdstr res;
  while(!_ctx.eof){
    auto ch=peek(_ctx);
    auto pos= bad.find(ch);
    if(::isspace(ch) ||
       pos != STDS_NPOS)
      break;
    res += ch;
    d::advance(_ctx);
  }
  t= -1;
  if(res == "false") t= T_FALSE;
  if(res == "true") t=T_TRUE;
  if(res == "nil") t=T_NIL;
  if(t != -1)
    return d::Token::make(t, res, m);
   DEBUG("::id = %s", res.c_str());
   return d::Token::make(d::T_IDENT, res, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Reader::skipCommas(){
  while(!_ctx.eof && (peek(_ctx) == ',')) advance(_ctx); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DToken Reader::getNextToken(){
  auto& S= d::getStrTokens();
  while(!_ctx.eof){
    auto ch= d::peek(_ctx);
    //ORDER IS IMPORTANT!
    if(::isspace(ch)){
      d::skipWhitespace(_ctx);
    }else if(ch == ','){
      skipCommas();
    }else if(ch == ';'){
      skipComment();
    }else if(ch == '~' && d::peekAhead(_ctx) == '@'){
      return d::Token::make(T_SPLICE_UNQUOTE, "~@", d::mark_advance(_ctx, 2));
    }else if(ch == '\'' || ch == '`' ||
             ch == '~' || ch == '^' || ch == '@'){
      return d::Token::make(S.at(stdstr(1,ch)), ch,d::mark_advance(_ctx));
    }else if((ch == '-' || ch == '+') &&
             '.'==d::peekAhead(_ctx) &&
             ::isdigit(d::peekAhead(_ctx,2))){
      return number();
    }else if((ch == '-' || ch == '+') &&
             ::isdigit(d::peekAhead(_ctx))){
      return number();
    }else if(ch=='.' &&
             ::isdigit(d::peekAhead(_ctx))){
      return number();
    }else if(::isdigit(ch)){
      return number();
    }else if(ch == '"'){
      return string();
    }else if(ch == '#' && d::peekAhead(_ctx) == '{'){
      return d::Token::make(T_SET, "#{", d::mark_advance(_ctx, 2));
    }else if(ch == '#' && d::peekAhead(_ctx) == '('){
      return d::Token::make(T_ANONFN,
                            "#(", d::mark_advance(_ctx, 2));
    }else if(ch == '('){
      return d::Token::make(d::T_LPAREN,
                            ch, d::mark_advance(_ctx));
    }else if(ch == ')'){
      return d::Token::make(d::T_RPAREN,
                            ch, d::mark_advance(_ctx));
    }else if(ch == '{'){
      return d::Token::make(d::T_LBRACE,
                            ch, d::mark_advance(_ctx));
    }else if(ch == '}'){
      return d::Token::make(d::T_RBRACE,
                            ch, d::mark_advance(_ctx));
    }else if(ch == '['){
      return d::Token::make(d::T_LBRACKET,
                            ch, d::mark_advance(_ctx));
    }else if(ch == ']'){
      return d::Token::make(d::T_RBRACKET,
                            ch, d::mark_advance(_ctx));
    }else if(ch == ':'){
      if(auto nx= d::peekAhead(_ctx); isKeywdChar(nx))
        return keywd();
      else
        return d::Token::make(d::T_COLON,
                              ch, d::mark_advance(_ctx));
    }else if(ch == '.'){
      return d::Token::make(d::T_DOT,
                            ch, d::mark_advance(_ctx));
    }else if(ch){
      return id();
    }else{
      return d::Token::make(d::T_ROGUE,
                            ch, d::mark_advance(_ctx));
    }
  }
  return d::Token::make(d::T_EOF, "<eof>", _ctx.mark());
}






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


