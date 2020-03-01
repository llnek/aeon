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
  {T_EQ, "="},
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
    s__conj(res,peek(ctx));
    advance(ctx);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string numeric(Context& ctx) {
  auto res = digits(ctx);
  if (!ctx.eof && peek(ctx) == '.') {
    s__conj(res,peek(ctx));
    advance(ctx);
    res += digits(ctx);
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
      if (ch == '"') {
        break;
      }
      if (ch == '\\') {
        if (!advance(ctx)) {
          throw SyntaxError("Malformed string value, bad escaped char.");
        }
        ch=escSeq(peek(ctx));
      }
      res += ch;
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
        res += ch;
        advance(ctx);
      } else {
        break;
        //error(ctx);
      }
    } else {
      if (ch == '_' || ::isalpha(ch) || ::isdigit(ch)) {
        res += ch;
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
ExprValue::ExprValue(ExprValueType t, const std::string& s) : type(t) {
  auto len=s.length();
  value.cs=std::make_shared<a::CString>(len);
  value.cs.get()->copy(s.c_str());
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
std::string ExprValue::toString() {
  switch (type) {
    case EXPR_REAL: return std::to_string(value.u.r);
    case EXPR_INT: return std::to_string(value.u.n); break;
    case EXPR_STR: return std::string(value.cs.get()->get()); break;
    default: return "null";
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const std::string& name, Frame* outer) : name(name) {
  prev=outer;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const std::string& name) : name(name) {
  S_NIL(prev);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::~Frame() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Frame::toString() {
  std::string b(40,'=');
  std::string z(40,'+');
  std::string out;
  char buf[1024];
  ::sprintf(buf, "%s\nName: %s\n%s\n", b.c_str(), name.c_str(),b.c_str());
  out += buf;
  for (auto& x : slots) {
    ::sprintf(buf, "%s = %s\n", x.first.c_str(), x.second.toString().c_str());
    out += buf;
  }
  out += z;
  out += "\n";
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::set<std::string> Frame::keys() {
  std::set<std::string> out;
  for (auto &x : slots) {
    out.insert(x.first);
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue Frame::get(const std::string& key) {
  auto x= slots.find(key);
  return x != slots.end() ? x->second : (prev ? prev->get(key) : ExprValue());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Frame::set(const std::string& key, const ExprValue& v, bool localOnly) {
  auto x= slots.find(key);

  if (x != slots.end() || localOnly) {
    slots[key]=v;
  } else if (prev) {
    prev->set(key,v);
  }
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CallStack::~CallStack() {
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CallStack::CallStack() {
  S_NIL(top);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::push(const std::string& name) {
  return (top= new Frame(name,top));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::pop() {
  auto x= top;
  if (top) {
    top = top->prev;
  }
  return x;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::peek() {
  return top;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::SymbolTable(const std::string& n, SymbolTable* outer)
  : SymbolTable(n) {
  enclosing=outer;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::SymbolTable(const std::string& n) {
  name=n;
  enclosing=nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::~SymbolTable() {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void SymbolTable::insert(Symbol* s) {
  if (s) symbols[s->name] = s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Symbol* SymbolTable::lookup(const std::string& name, bool traverse) {
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





