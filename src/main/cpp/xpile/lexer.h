#pragma once
#include <string>
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon::lexer {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum TokenType {
  T_PLUS,
  T_MINUS,
  T_MULT,
  T_DIV,
  T_LPAREN,
  T_RPAREN,
  T_LBRACE,
  T_RBRACE,
  T_SEMI,
  T_DOT,
  T_COLON,
  T_COMMA,

  T_INT,
  T_REAL,

  T_PROGRAM,
  T_INT_DIV,
  T_VAR,
  T_PROCEDURE,
  T_BEGIN,
  T_END,

  T_ID,
  T_INTEGER,
  T_FLOAT,
  T_ASSIGN,
  T_EOF
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum class Reserved {
  Opcode,
  Keyword
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Token {
  Token(int type);
  Token(int type, const std::string& s);
  Token(Reserved type, int t);
  Token(int type, long n);
  Token(int type, double d);
  Token(int type, const char c);
  ~Token() {}
  int line;
  int col;
  int type;
  union {
    double real;
    long num;
    char name[512];
  } value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Context {
  Context(const char* file, const char* src);
  ~Context() {}
  const char* file;
  const char* src;
  size_t len;
  int line;
  int col;
  int pos;
  bool eof;
  Token* cur;
private:
  void init();
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Context lexer(const char* src, const char* filename);
char get_ch(const Context&);
void next_token(Context&);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

