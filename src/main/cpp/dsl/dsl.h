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

#include "../aeon/smptr.h"

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
struct Number;
struct Table;
struct Symbol;
struct Node;
struct Data;
struct Frame;
struct AbstractToken;
typedef a::RefPtr<Data> DslValue;
typedef a::RefPtr<Node> DslAst;
typedef a::RefPtr<Frame> DslFrame;
typedef a::RefPtr<Symbol> DslSymbol;
typedef a::RefPtr<Table> DslTable;
typedef a::RefPtr<AbstractToken> DslToken;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::vector<Number> NumberVec;

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
  T_QUOTE,
  T_DQUOTE,
  T_BANG,
  T_AMPER,
  T_AT,
  T_PERCENT,
  T_QMARK,
  T_HAT,
  T_LBRACKET,
  T_RBRACKET,

  T_COMMENT,
  T_EOF,
  T_ETHEREAL
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct AbstractToken : public a::Counted {

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
struct Symbol : public a::Counted {

  // int a = 3; a is a symbol, int is a symbol(type), 3 is a value

  Symbol(const stdstr& n, DslSymbol t) : Symbol(n) { _type=t; }
  Symbol(const stdstr& n) : _name(n) { }
  stdstr name() const { return _name; }
  DslSymbol type() const { return _type; }
  ~Symbol() {}

  private:

  DslSymbol _type;
  stdstr _name;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Table : public a::Counted {

  // A symbol table (with hierarchy)

  DslSymbol search(const stdstr&) const;
  DslSymbol find(const stdstr&) const;

  DslTable outer() const { return enclosing; }
  stdstr name() const { return _name; }
  void insert(DslSymbol);

  Table(const stdstr&, const std::map<stdstr,DslSymbol>& root);
  Table(const stdstr&, DslTable outer);
  Table(const stdstr&);
  ~Table() {}

  protected:

  DslTable enclosing;
  stdstr _name;
  std::map<stdstr, DslSymbol> symbols;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Data : public a::Counted {
  // Abstract class to store data value in parsers.
  virtual bool equals(const Data*) const = 0;
  virtual int compare(const Data*) const = 0;
  virtual stdstr pr_str(bool p = 0) const = 0;
  virtual ~Data() {}
  Data() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Nothing : public Data {
  stdstr pr_str(bool b=0) const { return "nothing"; }
  virtual bool equals(const Data* rhs) const {
    return X_NIL(rhs) &&
           typeid(*rhs) == typeid(*this);
  }
  virtual int compare(const Data* rhs) const {
    return E_NIL(rhs)
        ? 1 : (typeid(*rhs) == typeid(*this) ? 0 : pr_str().compare(rhs->pr_str()));
  }
  virtual ~Nothing() {}
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
struct Node : public a::Counted {
  // Abstract node used in the building of a syntax tree.
  virtual DslValue eval(IEvaluator*)=0;
  virtual void visit(IAnalyzer*)=0;
  virtual ~Node() {}
  protected:
  Node() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Frame : public a::Counted {

  // A stack frame used during language evaluation.

  Frame(const stdstr&, DslFrame outer);
  Frame(const stdstr&);
  ~Frame() {}

  stdstr name() const { return _name; }
  stdstr pr_str() const;

  DslValue setEx(const stdstr& sym, DslValue);
  DslValue set(const stdstr& sym, DslValue);
  DslValue get(const stdstr& sym) const;

  DslFrame search(const stdstr& sym) const;

  bool contains(const stdstr&) const;
  std::set<stdstr> keys() const;

  DslFrame getOuterRoot() const;
  DslFrame getOuter() const;

  private:

  std::map<stdstr,DslValue> slots;
  stdstr _name;
  DslFrame prev;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct TypeSymbol : public Symbol {
  TypeSymbol(const stdstr& n) : Symbol(n) {}
  ~TypeSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarSymbol : public Symbol {
  VarSymbol(const stdstr& n, DslSymbol t) : Symbol(n,t) {}
  ~VarSymbol() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FnSymbol : public Symbol {
  FnSymbol(const stdstr& name, DslSymbol t) : FnSymbol(name) { result=t; }
  FnSymbol(const stdstr& name) : Symbol(name) {}
  ~FnSymbol() {}

  DslSymbol returnType() const { return result; }
  DslAst body() const { return block; }
  std::vector<DslSymbol>& params() { return _params; }
  void setBody(DslAst b) { block=b;}

  private:

  DslSymbol result;
  DslAst block;
  std::vector<DslSymbol> _params;
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





