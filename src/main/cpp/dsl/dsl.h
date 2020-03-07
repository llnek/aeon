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

#include "../aeon/aeon.h"

#define dsl_error(EXP, fmt, ...) \
  do { char buf[1024]; \
  ::sprintf(buf, (const char*)fmt, __VA_ARGS__); throw EXP(buf);} while (0)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::dsl {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a=czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Data;
struct Frame;
typedef a::ManagedPtr<Data> DslValue;
typedef a::ManagedPtr<Frame> DslFrame;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum {
  T_INTEGER = 1000000,
  T_REAL,
  T_STRING,
  T_IDENT,
  T_COMMA,
  T_COLON,
  T_LPAREN,
  T_RPAREN,
  T_LBRACE,
  T_RBRACE,
  T_EQ,
  T_MINUS,
  T_PLUS,
  T_MULT,
  T_DIV,
  T_GT,
  T_LT,
  T_SEMI,
  T_DOT,
  T_TILDA,
  T_BACKTICK,
  T_BANG,
  T_AMPER,
  T_AT,
  T_PERCENT,
  T_QUESTION,
  T_HAT,
  T_LBRACKET,
  T_RBRACKET,

  T_EOF
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IToken {
  virtual std::string getLiteralAsStr()=0;
  virtual double getLiteralAsReal()=0;
  virtual long getLiteralAsInt()=0;
  virtual std::string toString()=0;
  virtual int type()=0;
  virtual ~IToken() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IScanner {
  virtual bool isKeyword(const std::string&) = 0;
  virtual IToken* getNextToken()=0;
  virtual IToken* number()=0;
  virtual IToken* id()=0;
  virtual IToken* string()=0;
  virtual void skipComment() = 0;
  virtual ~IScanner() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ValueSlot {
  std::shared_ptr<a::CString> cs;
  union {
    llong n;
    double r;
  } u;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lexeme {
  std::string text;
  int line;
  int col;
  int type;
  ValueSlot value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct TokenInfo {
  TokenInfo(int line, int col) : line (line), col(col) {}
  int line,col;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Context {
  TokenInfo mark();
  ~Context() {}
  Context();
  //
  const char* src;
  size_t len;
  int line;
  int col;
  int pos;
  bool eof;
  IToken* cur;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SyntaxError {
  const std::string what() const { return msg; }
  SyntaxError(const std::string&);
  SyntaxError(const char*);
  private:
  std::string msg;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SemanticError {
  const std::string what() const { return msg; }
  SemanticError(const std::string&);
  SemanticError(const char*);
  private:
  std::string msg;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skipWhitespace(Context&);
bool advance(Context&);
char peek(Context&);
char peekNext(Context&);
std::string str(Context&);
std::string numeric(Context&);
std::string identifier(Context&);
Data* nothing();

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Symbol {

  Symbol(const std::string& n, Symbol* t) : Symbol(n) { type=t; }
  Symbol(const std::string& n) : name(n) { S_NIL(type); }
  ~Symbol() {}

  Symbol* type;
  std::string name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SymbolTable {

  Symbol* lookup(const std::string& s, bool traverse=true);
  void insert(Symbol*);

  SymbolTable(const std::string&, SymbolTable* outer);
  SymbolTable(const std::string&);
  ~SymbolTable();

  SymbolTable* enclosing;
  std::string name;
  std::map<std::string, Symbol*> symbols;
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Data : public a::Counted {
  virtual bool equals(const Data*) const = 0;
  virtual std::string toString() const = 0;
  virtual ~Data() {}
  Data() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Nothing : public Data {
  virtual std::string toString() const { return "nothing";}
  virtual bool equals(const Data* rhs) const {
    Nothing X;
    return X_NIL(rhs) && typeid(*rhs) == typeid(X);
  }
  virtual ~Nothing() {}
  Nothing() {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IEvaluator {
  virtual void setValue(const std::string&, const DslValue&, bool localOnly) = 0;
  virtual DslValue getValue(const std::string&) = 0;
  virtual DslFrame push(const std::string& name)=0;
  virtual DslFrame pop()=0;
  virtual DslFrame peek()=0;
  virtual ~IEvaluator() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IAnalyzer {
  virtual Symbol* lookup(const std::string&, bool traverse=true) = 0;
  virtual void pushScope(const std::string& name) =0;
  virtual SymbolTable* popScope()=0;
  virtual void define(Symbol*)=0;
  virtual ~IAnalyzer() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IAst {
  virtual DslValue eval(IEvaluator*)=0;
  virtual ~IAst() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Frame : public a::Counted {

  Frame(const std::string&, const DslFrame& outer);
  Frame(const std::string&);

  std::string toString();
  ~Frame();

  void set(const std::string& sym, const DslValue&, bool localOnly);
  DslValue get(const std::string& sym);
  std::set<std::string> keys();

  //DslFrame find(const std::string& sym);
  DslFrame getOuter();

  private:

  std::map<std::string,DslValue> slots;
  std::string name;
  DslFrame prev;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct TypeSymbol : public Symbol {
  TypeSymbol(const std::string& n) : Symbol(n) {}
  ~TypeSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ParamSymbol : public Symbol {
  ParamSymbol(const std::string& n, Symbol* t) : Symbol(n,t) {}
  ~ParamSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarSymbol : public Symbol {
  VarSymbol(const std::string& n, Symbol* t) : Symbol(n,t) {}
  ~VarSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FunctionSymbol : public Symbol {
  FunctionSymbol(const std::string& name, TypeSymbol* t) : FunctionSymbol(name) {
    result=t;
  }
  FunctionSymbol(const std::string& name) : Symbol(name) {
    S_NIL(block);
    S_NIL(result);
  }
  ~FunctionSymbol() {}

  TypeSymbol* result;
  IAst* block;
  std::vector<ParamSymbol*> params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IParser {
  virtual IToken* eat(int wantedToken)=0;
  virtual IAst* parse()=0;
  virtual ~IParser() {}
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





