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
#define DMARK(l,p) s__pair(int,int,l,p)
#define DCAST(T,x) s__cast(T,x.get())

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define WRAP_SYM(x) czlab::dsl::DslSymbol(x)
#define WRAP_VAL(x) czlab::dsl::DslValue(x)
#define WRAP_AST(x) czlab::dsl::DslAst(x)
#define WRAP_TKN(x) czlab::dsl::DslToken(x)
#define WRAP_ENV(x) czlab::dsl::DslFrame(x)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef bool (*IdPredicate)(Tchar, bool); // for tokenizeing an identifier
typedef std::pair<int,int> Mark; // line & col info

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoSuchVar : public a::Exception {
  NoSuchVar(cstdstr& m) : a::Exception (m) {}
};
struct BadArg : public a::Exception {
  BadArg(cstdstr& m) : a::Exception(m) {}
};
struct DivByZero : public a::Exception {
  DivByZero(cstdstr& m) : a::Exception(m) {}
};
struct BadArity : public a::Exception {
  BadArity(cstdstr& m) : a::Exception(m) {}
  BadArity(int wanted, int got)
    : a::Exception("Expected " + N_STR(wanted) +
                   " args, got " + N_STR(got) + ".") {}
};
struct BadEval : public a::Exception {
  BadEval(cstdstr& m) : a::Exception(m) {}
};
struct IndexOOB : public a::Exception {
  IndexOOB(cstdstr& m) : a::Exception(m) {}
};
struct Unsupported : public a::Exception {
  Unsupported(cstdstr& m) : a::Exception(m) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Table;
struct Symbol;
struct Node;
struct Data;
struct Frame;
struct Lexeme;
typedef std::shared_ptr<Data> DslValue;
typedef std::shared_ptr<Node> DslAst;
typedef std::shared_ptr<Frame> DslFrame;
typedef std::shared_ptr<Symbol> DslSymbol;
typedef std::shared_ptr<Table> DslTable;
typedef std::shared_ptr<Lexeme> DslToken;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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
int preEqual(int wanted, int got, cstdstr&);
int preMin(int min, int got, cstdstr&);
int preMax(int max, int got, cstdstr&);
int preNonZero(int c, cstdstr& fn);
int preEven(int c, cstdstr& fn);

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
  virtual double getFloat() const =0;
  virtual stdstr getStr() const =0;
  virtual llong getInt() const =0;
  virtual stdstr pr_str() const =0;
  virtual ~Lexeme() {}

  Mark marker() const { return info; }
  int type() const { return ttype;}

  protected:

  Lexeme(int t) { ttype=t; }
  Mark info;
  int ttype;

};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IScanner {
  // Interface for a Lexical scanner.
  virtual bool isKeyword(cstdstr&) const = 0;
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
  DslToken cur;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SemanticError : public a::Exception { SemanticError(cstdstr&); };

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SyntaxError : public a::Exception { SyntaxError(cstdstr&); };

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool is_same(const Data* x, const Data* y);
bool is_same(DslValue, const Data* y);

Tchar peekAhead(Context&, int offset=1);
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

  static DslSymbol make(cstdstr& n, DslSymbol t) {
    return WRAP_SYM(new Symbol(n, t));
  }

  static DslSymbol make(cstdstr& n) {
    return WRAP_SYM(new Symbol(n));
  }

  DslSymbol type() const { return _type; }
  stdstr name() const { return _name; }

  ~Symbol() {}

  protected:

  Symbol(cstdstr& n, DslSymbol t) : Symbol(n) { _type=t; }
  Symbol(cstdstr& n) : _name(n) { }

  private:

  stdstr _name;
  DslSymbol _type;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef std::map<stdstr,DslSymbol> SymbolMap;
struct Table {
  // A symbol table (with hierarchy)

  static DslTable make(cstdstr&, const SymbolMap&);
  static DslTable make(cstdstr&, DslTable);
  static DslTable make(cstdstr&);

  DslSymbol search(cstdstr&) const;
  DslSymbol find(cstdstr&) const;

  DslTable outer() const { return enclosing; }
  stdstr name() const { return _name; }
  void insert(DslSymbol);

  ~Table() {}

  protected:

  Table(cstdstr&, const SymbolMap&);
  Table(cstdstr&, DslTable);
  Table(cstdstr&);

  stdstr _name;
  SymbolMap symbols;
  DslTable enclosing;

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

  static DslValue make() { return WRAP_VAL(new Nothing()); }

  stdstr pr_str(bool b=0) const { return "nothing"; }

  virtual bool equals(DslValue rhs) const {
    ASSERT1(rhs);
    return is_same(rhs.get(),this);
  }

  virtual int compare(DslValue rhs) const {
    ASSERT1(rhs);
    return is_same(rhs.get(),this)
           ? 0 : pr_str().compare(rhs->pr_str());
  }

  virtual ~Nothing() {}

  protected:

  Nothing() {}

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IEvaluator {
  // Interface support for parser evaluation.
  virtual DslValue setValueEx(cstdstr&, DslValue) = 0;
  virtual DslValue setValue(cstdstr&, DslValue) = 0;
  virtual DslValue getValue(cstdstr&) const = 0;
  virtual DslFrame pushFrame(cstdstr&)=0;
  virtual DslFrame popFrame()=0;
  virtual DslFrame peekFrame() const =0;
  virtual ~IEvaluator() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IAnalyzer {
  // Interface support for abstract syntax tax analysis.
  virtual DslSymbol search(cstdstr&) const = 0;
  virtual DslSymbol find(cstdstr&) const = 0;

  virtual DslTable pushScope(cstdstr&) = 0;
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

  static DslFrame search(cstdstr&, DslFrame);
  static DslFrame make(cstdstr&, DslFrame);
  static DslFrame make(cstdstr&);
  static DslFrame getRoot(DslFrame);

  ~Frame() {}

  stdstr name() const { return _name; }
  stdstr pr_str() const;

  DslValue setEx(cstdstr&, DslValue);
  DslValue set(cstdstr&, DslValue);
  DslValue get(cstdstr&) const;

  bool contains(cstdstr&) const;
  std::set<stdstr> keys() const;

  DslFrame getOuter() const;

  protected:

  Frame(cstdstr&, DslFrame);
  Frame(cstdstr&);

  private:

  std::map<stdstr,DslValue> slots;
  stdstr _name;
  DslFrame prev;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct TypeSymbol : public Symbol {

  static DslSymbol make(cstdstr& n) {
    return WRAP_SYM(new TypeSymbol(n));
  }

  ~TypeSymbol() {}

  protected:

  TypeSymbol(cstdstr& n) : Symbol(n) {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarSymbol : public Symbol {

  static DslSymbol make(cstdstr& n, DslSymbol t) {
    return WRAP_SYM(new VarSymbol(n,t));
  }

  ~VarSymbol() {}

  protected:

  VarSymbol(cstdstr& n, DslSymbol t) : Symbol(n,t) {}

};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FnSymbol : public Symbol {

  static DslSymbol make(cstdstr& name, DslSymbol t) {
    return WRAP_SYM(new FnSymbol(name, t));
  }

  static DslSymbol make(cstdstr& name) {
    return WRAP_SYM(new FnSymbol(name));
  }

  DslSymbol returnType() const { return result; }
  SymbolVec& params() { return _params; }
  DslAst body() const { return block; }
  void setBody(DslAst b) { block=b;}

  ~FnSymbol() {}

  protected:

  FnSymbol(cstdstr& name, DslSymbol t) : FnSymbol(name) { result=t; }
  FnSymbol(cstdstr& name) : Symbol(name) {}

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
struct Token : public Lexeme {

  virtual double getFloat() const;
  virtual stdstr getStr() const;
  virtual llong getInt() const;
  virtual stdstr pr_str() const;
  virtual ~Token() {}

  protected:

  explicit Token(int, cstdstr&, Mark);
  explicit Token(int, Tchar, Mark);

  stdstr lexeme;
  union {
    llong n; double r; } number;
};





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





