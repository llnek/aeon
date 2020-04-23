#pragma once
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
#define DMARK(l,c) s__pair(int,int,l,c)
#define DCAST(T,x) s__cast(T,x.get())
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define WRAP_SYM(T,...) czlab::dsl::DSymbol(new T(__VA_ARGS__))
#define WRAP_VAL(T,...) czlab::dsl::DValue(new T(__VA_ARGS__))
#define WRAP_AST(T,...) czlab::dsl::DAst(new T(__VA_ARGS__))
#define WRAP_TKN(T,...) czlab::dsl::DToken(new T(__VA_ARGS__))
#define WRAP_ENV(T,...) czlab::dsl::DFrame(new T(__VA_ARGS__))
#define DVAL_NIL czlab::dsl::DValue(P_NIL)
#define DTKN_NIL czlab::dsl::DToken(P_NIL)
#define DENV_NIL czlab::dsl::DFrame(P_NIL)
#define DSYM_NIL czlab::dsl::DSymbol(P_NIL)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define E_SEMANTIC(fmt,...) RAISE(czlab::dsl::SemanticError, fmt, __VA_ARGS__)
#define E_SYNTAX(fmt,...) RAISE(czlab::dsl::SyntaxError, fmt, __VA_ARGS__)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef bool (*IdPredicate)(Tchar, bool); // for tokenizeing an identifier
typedef std::pair<int,int> Mark; // line & col info

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoSuchVar : public a::Error {
  NoSuchVar(cstdstr& m) : a::Error (m) {}
};

struct BadArg : public a::Error {
  BadArg(cstdstr& m) : a::Error(m) {}
};

struct DivByZero : public a::Error {
  DivByZero(cstdstr& m) : a::Error(m) {}
};

struct BadArity : public a::Error {
  BadArity(cstdstr& m) : a::Error(m) {}
  BadArity(int wanted, int got)
    : a::Error("Expected " + N_STR(wanted) +
                   " args, got " + N_STR(got) + ".") {}
};

struct BadEval : public a::Error {
  BadEval(cstdstr& m) : a::Error(m) {}
};

struct IndexOOB : public a::Error {
  IndexOOB(cstdstr& m) : a::Error(m) {}
};

struct Unsupported : public a::Error {
  Unsupported(cstdstr& m) : a::Error(m) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Symbol;
struct Table;
struct Node;
struct Data;
struct Frame;
struct Lexeme;
typedef std::shared_ptr<Lexeme> DToken;
typedef std::shared_ptr<Data> DValue;
typedef std::shared_ptr<Node> DAst;
typedef std::shared_ptr<Frame> DFrame;
typedef std::shared_ptr<Table> DTable;
typedef std::shared_ptr<Symbol> DSymbol;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::vector<DSymbol> SymbolVec;
typedef std::vector<DToken> TokenVec;
typedef std::vector<DAst> AstVec;
typedef std::vector<DValue> ValVec;

typedef TokenVec::iterator TokenIter;
typedef AstVec::iterator AstIter;
typedef ValVec::iterator ValIter;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VSlice {
  VSlice(ValVec& v) { begin=v.begin(); end=v.end(); }
  VSlice(ValIter b, ValIter e) : begin(b), end(e) {}
  int size() { return std::distance(begin, end); }
  ValIter begin, end;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEqual(int wanted, int got, cstdstr&);
int preMin(int min, int got, cstdstr&);
int preMax(int max, int got, cstdstr&);
int preNonZero(int c, cstdstr&);
int preEven(int c, cstdstr&);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum TokenType {
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
  T_EQ, //10
  T_MINUS,
  T_PLUS,
  T_MULT,
  T_DIV,
  T_GT,
  T_LT,
  T_SEMI,
  T_DOT,
  T_TILDA,
  T_BACKTICK, //20
  T_QUOTE,
  T_DQUOTE,
  T_BANG,
  T_AMPER,
  T_AT,
  T_PERCENT,
  T_QMARK,
  T_HAT,
  T_LBRACKET,
  T_RBRACKET, //30

  T_COMMENT,
  T_EOF,
  T_ROGUE,
  T_ETHEREAL
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lexeme {
  // A chunk of text - a sequence of chars.

  Mark marker() const { return info; }
  int type() const { return ttype;}

  virtual double getFloat() const =0;
  virtual stdstr getStr() const =0;
  virtual llong getInt() const =0;
  virtual stdstr pr_str() const =0;
  virtual ~Lexeme() {}

  protected:

  Mark info;
  int ttype;

  Lexeme(int t) { ttype=t; }
  Lexeme(int t, Mark m) { ttype=t;  info=m; }
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IScanner {

  // Interface for a Lexical scanner.
  virtual bool isKeyword(cstdstr&) const = 0;
  virtual DToken skipComment() = 0;
  virtual DToken getNextToken()=0;

  virtual DToken number()=0;
  virtual DToken id()=0;
  virtual DToken string()=0;

  virtual ~IScanner() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/*
struct Number {

  // A gnerialized number object.
  explicit Number(double d) : type(T_REAL) { u.r=d; }
  explicit Number(llong n) : type(T_INTEGER) { u.n=n; }
  Number(int n) : type(T_INTEGER) { u.n=n; }
  Number() : type(T_INTEGER) { u.n=0; }

  bool isZero() const;
  bool isInt() const;

  void setFloat(double);
  void setInt(llong);
  void setInt(int);

  double getFloat() const;
  llong getInt() const;

  private:

  int type;
  union { llong n; double r; } u;
};
*/
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Context {
  /////////////////////////////////////////////
  // For lexer, holds all the key attributes.
  Mark mark() { return DMARK(line,col); }
  ~Context() {}
  Context();
  /////////////////////////////////////////////
  int line, col, pos;
  const Tchar* src;
  size_t len;
  bool eof;
  DToken cur;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SemanticError : public a::Error {
  SemanticError(cstdstr& m) : a::Error(m) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SyntaxError : public a::Error {
  SyntaxError(cstdstr& m) : a::Error(m) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool is_same(const Data* x, const Data* y);
bool is_same(DValue, const Data* y);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<stdstr,Mark> identifier(Context&, IdPredicate);
std::pair<stdstr,Mark> numeric(Context&);
std::pair<stdstr,Mark> str(Context&);
Tchar peekAhead(Context&, int offset=1);
Tchar peek(Context&);
void skipWhitespace(Context&);
bool advance(Context&, int steps=1);
Mark mark_advance(Context&, int steps=1);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Symbol {
  // int a = 3; a is a symbol, int is a symbol(type), 3 is a value
  static DSymbol make(cstdstr& n, DSymbol t) {
    return WRAP_SYM(Symbol, n, t);
  }
  static DSymbol make(cstdstr& n) {
    return WRAP_SYM(Symbol, n);
  }
  DSymbol type() const { return _type; }
  stdstr name() const { return _name; }

  ~Symbol() {}

  protected:

  Symbol(cstdstr& n, DSymbol t) : Symbol(n) { _type=t; }
  Symbol(cstdstr& n) : _name(n) { }

  private:

  stdstr _name;
  DSymbol _type;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::map<stdstr,DSymbol> SymbolMap;
struct Table {
  // A symbol table (with hierarchy)

  static DTable make(cstdstr&, const SymbolMap&);
  static DTable make(cstdstr&, DTable);
  static DTable make(cstdstr&);

  DTable outer() const { return enclosing; }
  stdstr name() const { return _name; }
  void insert(DSymbol);

  DSymbol search(cstdstr&) const;
  DSymbol find(cstdstr&) const;

  ~Table() {}

  protected:

  Table(cstdstr&, const SymbolMap&);
  Table(cstdstr&, DTable);
  Table(cstdstr&);

  stdstr _name;
  SymbolMap symbols;
  DTable enclosing;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Data {
  // Abstract class to store data value in parsers.
  virtual stdstr pr_str(bool p = 0) const = 0;
  virtual bool equals(DValue) const = 0;
  virtual int compare(DValue) const = 0;
  virtual ~Data() {}

  protected:

  Data() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Nothing : public Data {

  static DValue make() { return WRAP_VAL(Nothing); }
  stdstr pr_str(bool b=0) const { return "nothing"; }

  virtual bool equals(DValue rhs) const {
    ASSERT1(rhs);
    return is_same(rhs,this);
  }

  virtual int compare(DValue rhs) const {
    ASSERT1(rhs);
    return is_same(rhs,this)
           ? 0 : pr_str().compare(rhs->pr_str());
  }

  virtual ~Nothing() {}

  protected:

  Nothing() {}

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IEvaluator {
  // Interface support for parser evaluation.
  virtual DValue setValueEx(cstdstr&, DValue) = 0;
  virtual DValue setValue(cstdstr&, DValue) = 0;
  virtual DValue getValue(cstdstr&) const = 0;
  virtual DFrame pushFrame(cstdstr&)=0;
  virtual DFrame popFrame()=0;
  virtual DFrame peekFrame() const =0;
  virtual ~IEvaluator() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IAnalyzer {
  // Interface support for abstract syntax tax analysis.
  virtual DSymbol search(cstdstr&) const = 0;
  virtual DSymbol find(cstdstr&) const = 0;

  virtual DTable pushScope(cstdstr&) = 0;
  virtual DTable popScope()=0;
  virtual DSymbol define(DSymbol)=0;

  virtual ~IAnalyzer() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Node {
  // Abstract node used in the building of a syntax tree.
  virtual DValue eval(IEvaluator*)=0;
  virtual void visit(IAnalyzer*)=0;
  virtual ~Node() {}
  protected:
  Node() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Frame {
  // A stack frame used during language evaluation.

  static DFrame search(cstdstr&, DFrame);
  static DFrame make(cstdstr&, DFrame);
  static DFrame make(cstdstr&);
  static DFrame getRoot(DFrame);

  ~Frame() {}

  stdstr name() const { return _name; }
  stdstr pr_str() const;

  DValue setEx(cstdstr&, DValue);
  DValue set(cstdstr&, DValue);
  DValue get(cstdstr&) const;

  bool contains(cstdstr&) const;
  std::set<stdstr> keys() const;

  DFrame getOuter() const;

  protected:

  Frame(cstdstr&, DFrame);
  Frame(cstdstr&);

  private:

  stdstr _name;
  DFrame prev;
  std::map<stdstr,DValue> slots;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct TypeSymbol : public Symbol {

  static DSymbol make(cstdstr& n) {
    return WRAP_SYM(TypeSymbol, n);
  }

  ~TypeSymbol() {}

  protected:

  TypeSymbol(cstdstr& n) : Symbol(n) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarSymbol : public Symbol {

  static DSymbol make(cstdstr& n, DSymbol t) {
    return WRAP_SYM(VarSymbol, n,t);
  }

  ~VarSymbol() {}

  protected:

  VarSymbol(cstdstr& n, DSymbol t) : Symbol(n,t) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FnSymbol : public Symbol {

  static DSymbol make(cstdstr& name, DSymbol t) {
    return WRAP_SYM(FnSymbol, name, t);
  }

  static DSymbol make(cstdstr& name) {
    return WRAP_SYM(FnSymbol, name);
  }

  DSymbol returnType() const { return result; }
  SymbolVec& params() { return _params; }
  DAst body() const { return block; }
  void setBody(DAst b) { block=b;}

  ~FnSymbol() {}

  protected:

  FnSymbol(cstdstr& name, DSymbol t) : FnSymbol(name) { result=t; }
  FnSymbol(cstdstr& name) : Symbol(name) {}

  DSymbol result;
  DAst block;
  SymbolVec _params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IParser {
  // Interface for a parser.
  virtual DToken eat(int wantedToken)=0;
  virtual DToken eat()=0;
  virtual bool isEof() const =0;
  virtual ~IParser() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/*
struct Token : public Lexeme {

  virtual double getFloat() const;
  virtual stdstr getStr() const;
  virtual llong getInt() const;
  virtual stdstr pr_str() const;
  virtual ~Token() {}

  protected:

  Token(int, cstdstr&, Mark);
  Token(int, Tchar, Mark);

  stdstr lexeme;
  union { llong n; double r; } number;
};
*/




//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





