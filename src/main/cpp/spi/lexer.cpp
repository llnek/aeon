#include <string>
#include <map>
#include "lexer.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::spi {
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, std::string> TOKENS {
  {T_PROCEDURE, "PROCEDURE"},
  {T_PROGRAM, "PROGRAM"},
  {T_VAR, "VAR"},
  {T_REAL, "REAL"},
  {T_INT, "INTEGER"},
  {T_INT_DIV, "DIV"},
  {T_BEGIN, "BEGIN"},
  {T_ASSIGN, ":="},
  {T_END, "END"}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<std::string,int> KEYWORDS {
  {TOKENS.find(T_BEGIN)->second, T_BEGIN},
  {TOKENS.find(T_END)->second, T_END},
  {TOKENS.find(T_PROGRAM)->second, T_PROGRAM},
  {TOKENS.find(T_PROCEDURE)->second, T_PROCEDURE},
  {TOKENS.find(T_VAR)->second, T_VAR},
  {TOKENS.find(T_INT)->second, T_INT},
  {TOKENS.find(T_REAL)->second, T_REAL},
  {TOKENS.find(T_INT_DIV)->second, T_INT_DIV}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int line, int col, int type) {
  impl.line=line;
  impl.col=col;
  impl.type=type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::~Token() {
  if (impl.type == d::T_IDENT || impl.type == d::T_STRING) {
    //del_array(impl.value.s);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int Token::type() {
  return impl.type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Token::getLiteralAsReal() {
  return (impl.type == d::T_REAL) ? impl.value.r : 0;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
long Token::getLiteralAsInt() {
  return (impl.type == d::T_INTEGER) ? impl.value.n : 0;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
const char* Token::getLiteralAsStr() {
  return nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* token(int line, int pos, int type, const char* s) {
  auto t= new Token(line,pos,type);
  t->impl.value.s = new char[::strlen(s)+1];
  ::strcpy(t->impl.value.s, s);
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* token(int line, int col, int type, long n) {
  auto t= new Token(line, col,type);
  t->impl.value.n=n;
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* token(int line, int col, int type, double d) {
  auto t= new Token(line,col,type);
  t->impl.value.r=d;
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Token::toString() {
  auto r= TOKENS.find(impl.type);
  auto s= "";
  char buf[1024];
  if (r != TOKENS.end()) {
    s= r->second.c_str();
  } else if (impl.type == d::T_IDENT) {
    s= impl.value.s;
  } else if (impl.type == d::T_INTEGER) {
    s= std::to_string(impl.value.n).c_str();
  } else if (impl.type == d::T_REAL) {
    s= std::to_string(impl.value.r).c_str();
  } else if (impl.type == d::T_STRING) {
    s= impl.value.s;
  }
  ::sprintf(buf, "Token#{type = %d, value = %s}", impl.type, s);
  return std::string(buf);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::Lexer(const char* src) {
  ctx.len= ::strlen(src);
  ctx.eof=false;
  ctx.src=src;
  ctx.line=0;
  ctx.col=0;
  ctx.pos=0;
  ctx.cur=nullptr;
  //
  ctx.cur= getNextToken();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::~Lexer() {

}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lexer::isKeyword(const char* s) {
  return KEYWORDS.find(std::string(s)) != KEYWORDS.end();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Lexer::skipComment() {
  while (!ctx.eof) {
    d::advance(ctx);
    if (d::peek(ctx) == '}') {
      d::advance(ctx);
      break;
    }
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Lexer::number() {
  auto s = d::numeric(ctx).c_str();
  return ::strchr(s, '.')
    ? token(ctx.line, ctx.col, d::T_REAL, ::atof(s))
    : token(ctx.line, ctx.col, d::T_INTEGER, ::atol(s));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Lexer::id() {
  auto s= d::identifier(ctx);
  auto c=s.c_str();
  return isKeyword(c)
    ? new Token(ctx.line, ctx.col, KEYWORDS.at(s))
    : token(ctx.line, ctx.col, d::T_IDENT, c);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* Lexer::getNextToken() {
  char ch;
  while (!ctx.eof) {
    ch= d::peek(ctx);
    if (::isspace(ch)) {
      d::skipWhitespace(ctx);
    }
    else
    if (::isdigit(ch)) {
      return number();
    }
    else
    if (ch == '*') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_MULT);
    }
    else
    if (ch == '/') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_DIV);
    }
    else
    if (ch == '+') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_PLUS);
    }
    else
    if (ch == '-') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_MINUS);
    }
    else
    if (ch == '(') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_LPAREN);
    }
    else
    if (ch == ')') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_RPAREN);
    }
    else
    if (ch == '_' || ::isalpha(ch)) {
      return id();
    }
    else
    if (ch== ':' && '=' == d::peekNext(ctx)) {
      d::advance(ctx);
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, T_ASSIGN);
    }
    else
    if (ch == '{') {
      d::advance(ctx);
      skipComment();
    }
    else
    if (ch == ';') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_SEMI);
    }
    else
    if (ch == ':') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_COLON);
    }
    else
    if (ch == ',') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_COMMA);
    }
    else
    if (ch == '.') {
      d::advance(ctx);
      return new Token(ctx.line, ctx.col, d::T_DOT);
    }
    else {
      return nullptr;//error();
    }
  }
  return new Token(ctx.line, ctx.col, d::T_EOF);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

