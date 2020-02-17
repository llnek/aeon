#include <string>
#include <map>
#include "lexer.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon::lexer {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, std::string> TOKENS {
  {T_PROCEDURE, "PROCEDURE"},
  {T_PROGRAM, "PROGRAM"},
  {T_VAR, "VAR"},
  {T_REAL, "REAL"},
  {T_INT, "INTEGER"},
  {T_INT_DIV, "DIV"},
  {T_INTEGER,"integer"},
  {T_FLOAT,"float"},
  {T_MULT,"*"},
  {T_COLON,":"},
  {T_DIV,"/"},
  {T_PLUS, "+"},
  {T_MINUS, "-"},
  {T_LPAREN,"("},
  {T_RPAREN,")"},
  {T_LBRACE,"{"},
  {T_RBRACE,"}"},
  {T_BEGIN, "BEGIN"},
  {T_ASSIGN, ":="},
  {T_SEMI,";"},
  {T_DOT,"."},
  {T_COMMA, ","},
  {T_END, "END"},
  {T_EOF, "EOF"}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<std::string,int> KEYWORDS {
  {TOKENS.find(T_BEGIN)->second, T_BEGIN},
  {TOKENS.find(T_END)->second, T_END},
  {TOKENS.find(T_ASSIGN)->second, T_ASSIGN},
  {TOKENS.find(T_SEMI)->second, T_SEMI},
  {TOKENS.find(T_DOT)->second, T_DOT},
  {TOKENS.find(T_COLON)->second, T_COLON},
  {TOKENS.find(T_COMMA)->second, T_COMMA},
  {TOKENS.find(T_PROGRAM)->second, T_PROGRAM},
  {TOKENS.find(T_PROCEDURE)->second, T_PROCEDURE},
  {TOKENS.find(T_VAR)->second, T_VAR},
  {TOKENS.find(T_INT)->second, T_INT},
  {TOKENS.find(T_REAL)->second, T_REAL},
  {TOKENS.find(T_INT_DIV)->second, T_INT_DIV}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type) {
  this->type=type;
  this->line=0;
  this->col=0;
  if (type == T_ID) {
  } else {
    ::printf("token=%s\n", TOKENS[type].c_str());
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const std::string& s) : Token(type) {
  ::strcpy(this->value.name, s.c_str());
  if (type == T_ID) {
    ::printf("var=%s\n", s.c_str());
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(Reserved type, int t) : Token(t) {
  ::strcpy(this->value.name, TOKENS.find(t)->second.c_str());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, long n) : Token(type) {
  this->value.num=n;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, double d) : Token(type) {
  this->value.real=d;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const char c) : Token(type) {
  this->value.name[0]=c;
  this->value.name[1]='\0';
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token* error(Context& ctx) {
  std::string s= "Error parsing input, pos= ";
  s += ctx.pos;
  throw new std::runtime_error(s);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Context::Context(const char* file, const char* src)
  : file(file), src(src), line(0), col(0), pos(0), eof(false) {
  this->len = ::strlen(src);
  init();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Context::init() {
  next_token(*this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char get_ch(const Context& ctx) {
  return ctx.src[ctx.pos];
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char peek(Context& ctx) {
  auto nx = ctx.pos + 1;
  if (nx >= ctx.len) {
    return '\0';
  } else {
    return ctx.src[nx];
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void advance(Context& ctx) {
  if (ctx.eof) {}
  else {
    if (get_ch(ctx) == '\n') {
      ++ctx.line;
      ctx.col=0;
    }
    ++ctx.pos;
    if (ctx.pos >= ctx.len) {
      ctx.eof=true;
    } else {
      ++ctx.col;
    }
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int is_keyword(const std::string& s) {
  auto it= KEYWORDS.find(s);
  return it != KEYWORDS.end() ? it->second : -1;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
inline void skip(Context& ctx) {
  while (!ctx.eof &&
         ::isspace(get_ch(ctx))) advance(ctx);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string digits(Context& ctx) {
  std::string res;
  while (!ctx.eof && ::isdigit(get_ch(ctx))) {
    res.push_back(get_ch(ctx));
    advance(ctx);
  }
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token* number(Context& ctx) {
  std::string res = digits(ctx);
  Token* t;
  if (!ctx.eof && get_ch(ctx) == '.') {
    res.push_back(get_ch(ctx));
    advance(ctx);
    res.append(digits(ctx));
    t = new Token(T_FLOAT, std::stod(res));
  } else {
    t = new Token(T_INTEGER, std::stol(res));
  }
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token* _id(Context& ctx) {
  std::string res;
  char ch;
  while (!ctx.eof) {
    ch=get_ch(ctx);
    if (res.empty()) {
      if (ch== '_' || ::isalpha(ch)) {
        res.push_back(ch);
        advance(ctx);
      } else {
        break;
        //error(ctx);
      }
    } else {
      if (ch== '_' || ::isalpha(ch) || ::isdigit(ch)) {
        res.push_back(ch);
        advance(ctx);
      } else {
        break;
        //error(ctx);
      }
    }
  }
  auto rc= is_keyword(res);
  if (rc < 0) {
    return new Token(T_ID,res);
  } else {
    return new Token(Reserved::Keyword,rc);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skip_comment(Context& ctx) {
  while (!ctx.eof) {
    advance(ctx);
    if (get_ch(ctx) == '}') {
      advance(ctx);
      break;
    }
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token* get_next_token(Context& ctx) {
  char ch;
  while (!ctx.eof) {
    ch= get_ch(ctx);
    if (::isspace(ch)) {
      skip(ctx);
    }
    else
    if (::isdigit(ch)) {
      return number(ctx);
    }
    else
    if (ch == '*') {
      advance(ctx);
      return new Token(T_MULT, '*');
    }
    else
    if (ch == '/') {
      advance(ctx);
      return new Token(T_DIV, '/');
    }
    else
    if (ch == '+') {
      advance(ctx);
      return new Token(T_PLUS, '+');
    }
    else
    if (ch == '-') {
      advance(ctx);
      return new Token(T_MINUS, '-');
    }
    else
    if (ch == '(') {
      advance(ctx);
      return new Token(T_LPAREN, '(');
    }
    else
    if (ch == ')') {
      advance(ctx);
      return new Token(T_RPAREN, ')');
    }
    else
    if (ch == '_' || ::isalpha(ch)) {
      return _id(ctx);
    }
    else
    if (ch== ':' && '=' == peek(ctx)) {
      advance(ctx);
      advance(ctx);
      return new Token(Reserved::Opcode, T_ASSIGN);
    }
    else
    if (ch == '{') {
      advance(ctx);
      skip_comment(ctx);
    }
    else
    if (ch == '}') {
      advance(ctx);
      return new Token(Reserved::Opcode, T_RBRACE);
    }
    else
    if (ch == ';') {
      advance(ctx);
      return new Token(Reserved::Opcode, T_SEMI);
    }
    else
    if (ch == ':') {
      advance(ctx);
      return new Token(Reserved::Opcode, T_COLON);
    }
    else
    if (ch == ',') {
      advance(ctx);
      return new Token(Reserved::Opcode, T_COMMA);
    }
    else
    if (ch == '.') {
      advance(ctx);
      return new Token(Reserved::Opcode, T_DOT);
    }
    else {
      return error(ctx);
    }
  }
  return new Token(T_EOF);
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void next_token(Context& ctx) {
  ctx.cur= get_next_token(ctx);
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Context lexer(const char* src, const char* filename) {
  return Context(filename,src);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

