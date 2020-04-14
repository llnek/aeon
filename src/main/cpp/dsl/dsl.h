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
typedef bool (*IdPredicate)(Tchar, bool); // for tokenizeing an identifier
typedef std::pair<int,int> SrcInfo; // line & col info

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoSuchVar : public a::Exception {
  NoSuchVar(const stdstr& m) : a::Exception (m) {}
};
struct BadArg : public a::Exception {
  BadArg(const stdstr& m) : a::Exception(m) {}
};
struct DivByZero : public a::Exception {
  DivByZero(const stdstr& m) : a::Exception(m) {}
};
struct BadArity : public a::Exception {
  BadArity(int wanted, int got)
    : a::Exception("Expected " + std::to_string(wanted) + " args, got " + std::to_string(got) + ".\n") {}
  BadArity(const stdstr& m) : a::Exception(m) {}
};
struct BadEval : public a::Exception {
  BadEval(const stdstr& m) : a::Exception(m) {}
};
struct IndexOOB : public a::Exception {
  IndexOOB(const stdstr& m) : a::Exception(m) {}
};
struct Unsupported : public a::Exception {
  Unsupported(const stdstr& m) : a::Exception(m) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Number;
struct Table;
struct Symbol;
struct Node;
struct Data;
struct Frame;
struct AbstractToken;
typedef std::shared_ptr<Data> DslValue;
typedef std::shared_ptr<Node> DslAst;
typedef std::shared_ptr<Frame> DslFrame;
typedef std::shared_ptr<Symbol> DslSymbol;
typedef std::shared_ptr<Table> DslTable;
typedef std::shared_ptr<AbstractToken> DslToken;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::vector<Number> NumberVec;

typedef std::vector<DslSymbol> SymbolVec;
typedef std::vector<DslToken> TokenVec;
typedef std::vector<DslAst> AstVec;
typedef std::vector<DslValue> ValVec;

typedef TokenVec::iterator TokenIter;
typedef AstVec::iterator AstIter;
typedef ValVec::iterator ValIter;

struct VSlice {
  VSlice(ValVec& v) { begin=v.begin(); end=v.end(); }
  VSlice(ValIter b, ValIter e) : begin(b), end(e) {}
  int size() { return std::distance(begin, end); }
  ValIter begin, end;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int preEqual(int wanted, int got, const stdstr& fn);
int preMin(int min, int got, const stdstr& fn);
int preMax(int max, int got, const stdstr& fn);
int preNonZero(int c, const stdstr& fn);
int preEven(int c, const stdstr& fn);

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
  T_ETHEREAL
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct AbstractToken {

  // A chunk of text - a sequence of chars.
  virtual stdstr getLiteralAsStr() const =0;
  virtual double getLiteralAsReal() const =0;
  virtual llong getLiteralAsInt() const =0;
  virtual stdstr pr_str() const =0;
  virtual ~AbstractToken() {}

  SrcInfo srcInfo() const { return info; }
  int type() const { return ttype;}

  protected:

  AbstractToken(int t ) { ttype=t; }
  SrcInfo info;
  int ttype;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IScanner {
  // Interface for a Lexical scanner.
  virtual bool isKeyword(const stdstr&) const = 0;
  virtual DslToken getNextToken()=0;
  virtual DslToken number()=0;
  virtual DslToken id()=0;
  virtual DslToken string()=0;
  virtual DslToken skipComment() = 0;
  virtual ~IScanner() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Number {
  // A gnerialized number object.
  explicit Number(double d) : type(T_REAL) { u.r=d; }
  Number(llong n) : type(T_INTEGER) { u.n=n; }
  Number() : type(T_INTEGER) { u.n=0; }

  bool isZero() const;
  bool isInt() const;

  void setFloat(double);
  void setInt(llong);

  double getFloat() const;
  llong getInt() const;

  private:

  int type;
  union { llong n; double r; } u;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Lexeme {
  stdstr txt;
  Number num;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Context {
  // For lexer, holds all the key attributes.
  SrcInfo mark();
  ~Context() {}
  Context();
  /////////////////////////////////////////////
  int line, col, pos;
  const char* src;
  size_t len;
  bool eof;
  DslToken cur;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SemanticError : public a::Exception { SemanticError(const stdstr&); };

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SyntaxError : public a::Exception { SyntaxError(const stdstr&); };

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Tchar peekNext(Context&, int offset=1);
void skipWhitespace(Context&);
bool advance(Context&, int steps=1);
Tchar peek(Context&);
stdstr str(Context&);
stdstr numeric(Context&);
stdstr line(Context&);
Data* nothing();
stdstr identifier(Context& ctx, IdPredicate pred);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Symbol {

  // int a = 3; a is a symbol, int is a symbol(type), 3 is a value

  static DslSymbol make(const stdstr& n, DslSymbol t) {
    return DslSymbol(new Symbol(n, t));
  }

  static DslSymbol make(const stdstr& n) {
    return DslSymbol(new Symbol(n));
  }

  DslSymbol type() const { return _type; }
  stdstr name() const { return _name; }

  ~Symbol() {}

  protected:

  Symbol(const stdstr& n, DslSymbol t) : Symbol(n) { _type=t; }
  Symbol(const stdstr& n) : _name(n) { }

  private:

  DslSymbol _type;
  stdstr _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::map<stdstr,DslSymbol> SymbolMap;
struct Table {
  // A symbol table (with hierarchy)

  static DslTable make(const stdstr&, const SymbolMap&);
  static DslTable make(const stdstr&, DslTable outer);
  static DslTable make(const stdstr&);

  DslSymbol search(const stdstr&) const;
  DslSymbol find(const stdstr&) const;

  DslTable outer() const { return enclosing; }
  stdstr name() const { return _name; }
  void insert(DslSymbol);

  ~Table() {}

  protected:

  Table(const stdstr&, const SymbolMap&);
  Table(const stdstr&, DslTable outer);
  Table(const stdstr&);

  DslTable enclosing;
  stdstr _name;
  SymbolMap symbols;

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Data {
  // Abstract class to store data value in parsers.
  virtual stdstr pr_str(bool p = 0) const = 0;
  virtual bool equals(DslValue) const = 0;
  virtual int compare(DslValue) const = 0;
  virtual ~Data() {}

  protected:

  Data() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Nothing : public Data {

  static DslValue make() { return DslValue(new Nothing()); }

  stdstr pr_str(bool b=0) const { return "nothing"; }

  virtual bool equals(DslValue rhs) const {
    ASSERT1(rhs);
    auto p= rhs.get();
    return typeid(*p) == typeid(*this);
  }

  virtual int compare(DslValue rhs) const {
    ASSERT1(rhs);
    auto p= rhs.get();
    return typeid(*p) == typeid(*this)
           ? 0 : pr_str().compare(rhs->pr_str());
  }

  virtual ~Nothing() {}

  protected:

  Nothing() {}

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IEvaluator {
  // Interface support for parser evaluation.
  virtual DslValue setValueEx(const stdstr&, DslValue) = 0;
  virtual DslValue setValue(const stdstr&, DslValue) = 0;
  virtual DslValue getValue(const stdstr&) const = 0;
  //virtual bool contains(const stdstr&) const = 0;
  virtual DslFrame pushFrame(const stdstr& name)=0;
  virtual DslFrame popFrame()=0;
  virtual DslFrame peekFrame() const =0;
  virtual ~IEvaluator() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IAnalyzer {
  // Interface support for abstract syntax tax analysis.
  virtual DslSymbol search(const stdstr&) const = 0;
  virtual DslSymbol find(const stdstr&) const = 0;

  virtual DslTable pushScope(const stdstr& name) = 0;
  virtual DslTable popScope()=0;
  virtual DslSymbol define(DslSymbol)=0;

  virtual ~IAnalyzer() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Node {
  // Abstract node used in the building of a syntax tree.
  virtual DslValue eval(IEvaluator*)=0;
  virtual void visit(IAnalyzer*)=0;
  virtual ~Node() {}
  protected:
  Node() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Frame {
  // A stack frame used during language evaluation.

  static DslFrame search(const stdstr& sym, DslFrame from);
  static DslFrame make(const stdstr&, DslFrame outer);
  static DslFrame make(const stdstr&);
  static DslFrame getRoot(DslFrame from);

  ~Frame() {}

  stdstr name() const { return _name; }
  stdstr pr_str() const;

  DslValue setEx(const stdstr& sym, DslValue);
  DslValue set(const stdstr& sym, DslValue);
  DslValue get(const stdstr& sym) const;

  bool contains(const stdstr&) const;
  std::set<stdstr> keys() const;

  DslFrame getOuter() const;

  protected:

  Frame(const stdstr&, DslFrame outer);
  Frame(const stdstr&);

  private:

  std::map<stdstr,DslValue> slots;
  stdstr _name;
  DslFrame prev;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct TypeSymbol : public Symbol {

  static DslSymbol make(const stdstr& n) {
    return DslSymbol(new TypeSymbol(n));
  }

  ~TypeSymbol() {}

  protected:

  TypeSymbol(const stdstr& n) : Symbol(n) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarSymbol : public Symbol {

  static DslSymbol make(const stdstr& n, DslSymbol t) {
    return DslSymbol(new VarSymbol(n,t));
  }

  ~VarSymbol() {}

  protected:

  VarSymbol(const stdstr& n, DslSymbol t) : Symbol(n,t) {}

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FnSymbol : public Symbol {

  static DslSymbol make(const stdstr& name, DslSymbol t) {
    return DslSymbol(new FnSymbol(name, t));
  }

  static DslSymbol make(const stdstr& name) {
    return DslSymbol(new FnSymbol(name));
  }

  DslSymbol returnType() const { return result; }
  SymbolVec& params() { return _params; }
  DslAst body() const { return block; }
  void setBody(DslAst b) { block=b;}

  ~FnSymbol() {}

  protected:

  FnSymbol(const stdstr& name, DslSymbol t) : FnSymbol(name) { result=t; }
  FnSymbol(const stdstr& name) : Symbol(name) {}

  DslSymbol result;
  DslAst block;
  SymbolVec _params;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IParser {
  // Interface for a parser.
  virtual DslToken eat(int wantedToken)=0;
  virtual DslToken eat()=0;
  virtual bool isEof() const =0;
  virtual ~IParser() {}
};








//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





