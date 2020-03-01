#pragma once

#include "../dsl/dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {
namespace d=czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

enum TokenType {
  T_PROGRAM,
  T_VAR,
  T_PROCEDURE,
  T_BEGIN,
  T_END,
  T_IF,
  T_ELSE,
  T_ENDIF,
  T_WHILE,
  T_ENDWHILE,
  T_FOR,
  T_ENDFOR,
  T_REPEAT,
  T_UNTIL,
  T_READ,
  T_READLN,
  T_WRITE,
  T_WRITELN,

  T_NOT,
  T_AND,
  T_OR,
  T_XOR,
  T_NOTEQ,
  T_GTEQ,
  T_LTEQ,
  T_ASSIGN
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Token : public d::IToken {

  Token(int line, int col, int type);
  virtual ~Token();

  std::string toString();
  int type();
  double getLiteralAsReal();
  long getLiteralAsInt();
  const char* getLiteralAsStr();

  d::Lexeme impl;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lexer : public d::IScanner {

  bool isKeyword(const char* s);
  d::IToken* getNextToken();
  void skipComment();
  d::IToken* number();
  d::IToken* id();
  d::IToken* string();

  Lexer(const char* src);
  virtual ~Lexer();
  d::Context ctx;
};






//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

