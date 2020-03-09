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

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::dsl {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a=czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SymbolTable;
struct Symbol;
struct Chunk;
struct Node;
struct Data;
struct Frame;
typedef a::ManagedPtr<Frame> DslFrame;
typedef a::ManagedPtr<Data> DslValue;
typedef a::ManagedPtr<Node> DslAst;
typedef a::ManagedPtr<Chunk> DslToken;
typedef a::ManagedPtr<Symbol> DslSymbol;
typedef std::vector<DslToken> TokenVec;
typedef std::vector<DslAst> AstVec;
typedef std::vector<DslSymbol> SymbolVec;
typedef a::ManagedPtr<SymbolTable> DslSymbolTable;
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

  T_EOF,
  T_ETHEREAL
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::pair<int,int> SrcInfo;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Chunk : public a::Counted {
  virtual stdstr getLiteralAsStr()=0;
  virtual double getLiteralAsReal()=0;
  virtual llong getLiteralAsInt()=0;
  virtual stdstr toString()=0;
  virtual ~Chunk() {}
  int type() { return ttype;}
  protected:
  Chunk(int t) { ttype=t; }
  int ttype;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IScanner {
  virtual bool isKeyword(const stdstr&) = 0;
  virtual DslToken getNextToken()=0;
  virtual DslToken number()=0;
  virtual DslToken id()=0;
  virtual DslToken string()=0;
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
  int line;
  int col;
  stdstr text;
  ValueSlot value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//struct TokenInfo {
//  TokenInfo(int line, int col) : line (line), col(col) {}
//  int line,col;
//};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Context {
  SrcInfo mark();
  ~Context() {}
  Context();
  //
  const char* src;
  size_t len;
  int line;
  int col;
  int pos;
  bool eof;
  DslToken cur;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SyntaxError {
  const stdstr what() const { return msg; }
  SyntaxError(const stdstr&);
  SyntaxError(const char*);
  private:
  stdstr msg;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SemanticError {
  const stdstr what() const { return msg; }
  SemanticError(const stdstr&);
  SemanticError(const char*);
  private:
  stdstr msg;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skipWhitespace(Context&);
bool advance(Context&);
char peek(Context&);
char peekNext(Context&);
stdstr str(Context&);
stdstr numeric(Context&);
stdstr identifier(Context&);
Data* nothing();

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Symbol : public a::Counted {

  Symbol(const stdstr& n, DslSymbol t) : Symbol(n) { type=t; }
  Symbol(const stdstr& n) : name(n) { }
  ~Symbol() {}

  DslSymbol type;
  stdstr name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SymbolTable : public a::Counted {

  DslSymbol lookup(const stdstr& s, bool traverse);
  void insert(DslSymbol);

  SymbolTable(const stdstr&, const std::map<stdstr,DslSymbol>& root);
  SymbolTable(const stdstr&, DslSymbolTable outer);
  SymbolTable(const stdstr&);
  ~SymbolTable();

  DslSymbolTable enclosing;
  stdstr name;
  std::map<stdstr, DslSymbol> symbols;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Data : public a::Counted {
  virtual bool equals(const Data*) const = 0;
  virtual stdstr toString() const = 0;
  virtual ~Data() {}
  Data() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Nothing : public Data {
  virtual stdstr toString() const { return "nothing";}
  virtual bool equals(const Data* rhs) const {
    return X_NIL(rhs) && typeid(*rhs) == typeid(*this);
  }
  virtual ~Nothing() {}
  Nothing() {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IEvaluator {
  virtual DslValue setValue(const stdstr&, DslValue, bool localOnly) = 0;
  virtual DslValue getValue(const stdstr&) = 0;
  virtual DslFrame push(const stdstr& name)=0;
  virtual DslFrame pop()=0;
  virtual DslFrame peek()=0;
  virtual ~IEvaluator() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IAnalyzer {
  virtual DslSymbol lookup(const stdstr&, bool traverse=true) = 0;
  virtual void pushScope(const stdstr& name) =0;
  virtual DslSymbolTable popScope()=0;
  virtual void define(DslSymbol)=0;
  virtual ~IAnalyzer() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Node : public a::Counted {
  virtual DslValue eval(IEvaluator*)=0;
  virtual void visit(IAnalyzer*)=0;
  virtual ~Node() {}
  protected:
  Node() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Frame : public a::Counted {

  Frame(const stdstr&, DslFrame outer);
  Frame(const stdstr&);

  stdstr toString();
  ~Frame();

  void set(const stdstr& sym, DslValue, bool localOnly);
  DslValue get(const stdstr& sym);
  std::set<stdstr> keys();

  //DslFrame find(const stdstr& sym);
  DslFrame getOuter();

  private:

  std::map<stdstr,DslValue> slots;
  stdstr name;
  DslFrame prev;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct TypeSymbol : public Symbol {
  TypeSymbol(const stdstr& n) : Symbol(n) {}
  ~TypeSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/*
struct ParamSymbol : public Symbol {
  ParamSymbol(const stdstr& n, DslSymbol t) : Symbol(n,t) {}
  ~ParamSymbol() {}
};*/

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarSymbol : public Symbol {
  VarSymbol(const stdstr& n, DslSymbol t) : Symbol(n,t) {}
  ~VarSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FnSymbol : public Symbol {
  FnSymbol(const stdstr& name, DslSymbol t)
    : FnSymbol(name) {
    result=t;
  }
  FnSymbol(const stdstr& name) : Symbol(name) {
  }
  ~FnSymbol() {}

  DslSymbol result;
  DslAst block;
  SymbolVec params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IParser {
  virtual DslToken eat(int wantedToken)=0;
  virtual DslAst parse()=0;
  virtual ~IParser() {}
};




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





