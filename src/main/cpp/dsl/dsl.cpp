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

#include "dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::dsl {
namespace a=czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, std::string> TOKENS {
  {T_INTEGER, "long"},
  {T_REAL, "double"},
  {T_STRING, "string"},
  {T_IDENT, "id"},
  {T_COMMA, ","},
  {T_COLON, ":"},
  {T_LPAREN, "("},
  {T_RPAREN, ")"},
  {T_LBRACE, "{"},
  {T_RBRACE, "}"},
  {T_EQUALS, "="},
  {T_MINUS, "-"},
  {T_PLUS, "+"},
  {T_MULT, "*"},
  {T_DIV, "/"},
  {T_GT, ">"},
  {T_LT, "<"},
  {T_SEMI, ";"},
  {T_TILDA, "~"},
  {T_BACKTICK, "`"},
  {T_BANG, "!"},
  {T_AMPER, "&"},
  {T_AT, "@"},
  {T_PERCENT, "%"},
  {T_QUESTION, "?"},
  {T_HAT, "^"},
  {T_LBRACKET, "["},
  {T_RBRACKET, "]"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SyntaxError::SyntaxError(const std::string& x)
  : std::logic_error(x){
}
SyntaxError::SyntaxError(const char* x)
  :std::logic_error(x) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
EvalError::EvalError(const std::string& x)
  : std::logic_error(x){
}
EvalError::EvalError(const char* x)
  :std::logic_error(x) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char peek(Context& ctx) {
  return ctx.src[ctx.pos];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char peekNext(Context& ctx) {
  auto nx = ctx.pos + 1;
  return (nx >= ctx.len) ? '\0': ctx.src[nx];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool advance(Context& ctx) {
  if (ctx.eof) { return false; }
  if (peek(ctx) == '\n') {
    ++ctx.line;
    ctx.col=0;
  }
  ++ctx.pos;
  if (ctx.pos >= ctx.len) {
    ctx.eof=true;
  } else {
    ++ctx.col;
  }
  return !ctx.eof;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skipWhitespace(Context& ctx) {
  while (!ctx.eof &&
         ::isspace(peek(ctx))) advance(ctx);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string digits(Context& ctx) {
  std::string res;
  while (!ctx.eof && ::isdigit(peek(ctx))) {
    res.push_back(peek(ctx));
    advance(ctx);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string numeric(Context& ctx) {
  auto res = digits(ctx);
  if (!ctx.eof && peek(ctx) == '.') {
    res.push_back(peek(ctx));
    advance(ctx);
    res.append(digits(ctx));
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char escSeq(char c) {
  switch (c) {
    case 'a': return '\a'; //	07	Alert (Beep, Bell) (added in C89)[1]
    case 'b': return '\b'; //	08	Backspace
    case 'e': return '\e'; // 1B	Escape character
    case 'f': return '\f'; //	0C	Formfeed Page Break
    case 'n': return '\n'; //	0A	Newline (Line Feed)
    case 'r': return '\r'; //	0D	Carriage Return
    case 't': return '\t'; //	09	Horizontal Tab
    case 'v': return '\v'; //	0B	Vertical Tab
    case '\\': return '\\'; //	5C	Backslash
    case '\'': return '\''; //	27	Apostrophe or single quotation mark
    case '"': return '"'; //	22	Double quotation mark
    case '?': return '\?'; //3F	Question mark (used to avoid trigraphs)
  }
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string str(Context& ctx) {

  std::string res;
  char ch;

  if (!ctx.eof &&
      peek(ctx) == '"') {
    advance(ctx);
    while (!ctx.eof) {
      ch= peek(ctx);
      ::printf("found str-char [%c]\n", ch);
      if (ch == '"') {
        break;
      }
      if (ch == '\\') {
        ::printf("found escape seq!\n");
        if (!advance(ctx)) {
          throw SyntaxError("Malformed string value, bad escaped char.");
        }
        ::printf("found escaped char [%c]\n", peek(ctx));
        ch=escSeq(peek(ctx));
      }
      res.push_back(ch);
      advance(ctx);
    }
    if (ctx.eof || ch != '"') {
      throw SyntaxError("Malformed string value, missing dquote.");
    }
    // good, got the end dquote
    advance(ctx);
  }

  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string identifier(Context& ctx) {
  std::string res;
  char ch;
  while (!ctx.eof) {
    ch=peek(ctx);
    if (res.empty()) {
      if (ch == '_' || ::isalpha(ch)) {
        res.push_back(ch);
        advance(ctx);
      } else {
        break;
        //error(ctx);
      }
    } else {
      if (ch == '_' || ::isalpha(ch) || ::isdigit(ch)) {
        res.push_back(ch);
        advance(ctx);
      } else {
        break;
        //error(ctx);
      }
    }
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue(ExprValueType t, long v) : type(t) {
  value.u.n=v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue(ExprValueType t, double v) : type(t) {
  value.u.r=v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue(ExprValueType t, const char* s) : type(t) {
  auto len=0;
  if (s) { len= ::strlen(s); }
  value.cs=std::make_shared<a::CString>(len);
  value.cs.get()->copy(s);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue() {
  type=EXPR_NULL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool ExprValue::isNull() {
  return type == EXPR_NULL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::~ExprValue() {
  //if (type == EXPR_STR) { del_array(value.s); }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue(const ExprValue& src) {
  value=src.value;
  type=src.type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue& ExprValue::operator=(const ExprValue& src) {
  value=src.value;
  type=src.type;
  return *this;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(FrameType type) : type(type) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::~Frame() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue Frame::get(const char* key) {
  auto x= slots.find(key);
  return x != slots.end() ? x->second : ExprValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Frame::set(const char* key, const ExprValue& v) {
  slots[key]=v;
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CallStack::~CallStack() {
  Frame* f= pop();
  while (f) {
    delete f;
    f=pop();
  }
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CallStack::CallStack() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void CallStack::push(Frame* f) {
  if (f) frames.push(f);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::pop() {
  auto x= peek();
  if (x) frames.pop();
  return x;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::peek() {
  return frames.empty() ? nullptr : frames.top();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::SymbolTable(SymbolTable* outer)
  : enclosing(outer) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::SymbolTable() : enclosing(nullptr) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::~SymbolTable() {
  for (auto x=symbols.begin(); x != symbols.end(); ++x) {
    delete x->second;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void SymbolTable::insert(Symbol* s) {
  if (s) symbols[s->name] = s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Symbol* SymbolTable::lookup(const char* name, bool traverse) {
  auto s = symbols.find(name);
  if (s != symbols.end()) {
    return s->second;
  } else {
    return (!traverse || !enclosing)
      ? nullptr : enclosing->lookup(name);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





