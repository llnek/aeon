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
typedef std::pair<int,int> SrcInfo;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Table;
struct Symbol;
struct Chunk;
struct Node;
struct Data;
struct Frame;
typedef a::RefPtr<Data> DslValue;
typedef a::RefPtr<Node> DslAst;
typedef a::RefPtr<Frame> DslFrame;
typedef a::RefPtr<Chunk> DslToken;
typedef a::RefPtr<Symbol> DslSymbol;
typedef a::RefPtr<Table> DslTable;

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
struct Chunk : public a::Counted {
  // A chunk of text - a sequence of chars.
  virtual stdstr getLiteralAsStr() const =0;
  virtual double getLiteralAsReal() const =0;
  virtual llong getLiteralAsInt() const =0;
  virtual stdstr toString() const =0;
  int type() const { return ttype;}
  virtual ~Chunk() {}
  protected:
  Chunk(int);
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
  int line, col;
  stdstr text;
  struct {
    Number num;
    std::shared_ptr<a::CString> cs; } value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Context {
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
bool advance(Context&);
Tchar peek(Context&);
stdstr str(Context&);
stdstr numeric(Context&);
stdstr identifier(Context&);
Data* nothing();

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

  DslSymbol lookup(const stdstr& s, bool traverseOuterScope) const;
  DslTable outer() const { return enclosing; }
  stdstr name() const { return _name; }
  void insert(DslSymbol);

  Table(const stdstr&, const std::map<stdstr,DslSymbol>& root);
  Table(const stdstr&, DslTable outer);
  Table(const stdstr&);
  ~Table() {}

  private:
  DslTable enclosing;
  stdstr _name;
  std::map<stdstr, DslSymbol> symbols;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Data : public a::Counted {
  // Abstract class to store data value in parsers.
  virtual stdstr toString(bool prettyPrint=false) const = 0;
  virtual bool equals(const Data*) const = 0;
  virtual ~Data() {}
  Data() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Nothing : public Data {
  stdstr toString(bool b) const { return "nothing"; }
  virtual bool equals(const Data* rhs) const {
    return X_NIL(rhs) &&
           typeid(*rhs) == typeid(*this);
  }
  virtual ~Nothing() {}
  Nothing() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IEvaluator {
  // Interface support for parser evaluation.
  virtual DslValue setValue(const stdstr&, DslValue, bool localOnly) = 0;
  virtual DslValue getValue(const stdstr&) const = 0;
  virtual bool containsSymbol(const stdstr&) const = 0;
  virtual DslFrame pushFrame(const stdstr& name)=0;
  virtual DslFrame popFrame()=0;
  virtual DslFrame peekFrame()=0;
  virtual ~IEvaluator() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IAnalyzer {
  // Interface support for abstract syntax tax analysis.
  virtual DslSymbol lookup(const stdstr&, bool traverseOuterScope=true) const = 0;
  virtual void pushScope(const stdstr& name) =0;
  virtual DslTable popScope()=0;
  virtual void define(DslSymbol)=0;
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
  stdstr toString() const;

  DslValue set(const stdstr& sym, DslValue, bool localOnly);
  DslValue get(const stdstr& sym) const;
  DslFrame find(const stdstr& sym) const;

  bool containsKey(const stdstr&) const;
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





