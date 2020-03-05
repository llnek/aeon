#pragma once

#include "../dsl/dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::mal {
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum TokenType {
  T_UNQUOTE_SPLICE,
  T_TRUE,
  T_FALSE,
  T_NIL,
  T_COMMENT
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Token : public d::IToken {

  static std::string typeToString(int);

  Token(int type, const std::string&, d::TokenInfo);
  Token(int type, const char, d::TokenInfo);

  virtual ~Token();

  std::string getLiteralAsStr();
  double getLiteralAsReal();
  long getLiteralAsInt();
  std::string toString();
  int type();

  d::Lexeme impl;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Reader : public d::IScanner {

  bool isKeyword(const std::string&);
  d::IToken* getNextToken();
  d::IToken* skipComment();
  d::IToken* number();
  d::IToken* id();
  d::IToken* string();

  Reader(const char* src);
  virtual ~Reader();

  d::Context ctx;

  protected:
  void skipCommas();

};





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

