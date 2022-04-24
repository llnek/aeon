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
 * Copyright © 2013-2022, Kenneth Leung. All rights reserved. */

#include "aeon/aeon.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::dsl{
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a=czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//creates an address (line,col)
#define DMARK(l,c) s__pair(int,int,l,c)
#define DMARK_00 s__pair(int,int,0,0)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//cast a shared-ptr
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
//exceptions
#define E_SEMANTIC(fmt,...) RAISE(czlab::dsl::SemanticError, fmt, __VA_ARGS__)
#define E_SYNTAX(fmt,...) RAISE(czlab::dsl::SyntaxError, fmt, __VA_ARGS__)
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef bool (*IdPredicate)(Tchar, bool); // for tokenizeing an identifier
typedef std::pair<int,int> Addr; // line & col info
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct NoSuchVar : public a::Error{
  NoSuchVar(cstdstr& m) : a::Error (m) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BadArg : public a::Error{
  BadArg(cstdstr& m) : a::Error(m) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct DivByZero : public a::Error{
  DivByZero(cstdstr& m) : a::Error(m) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BadArity : public a::Error{
  BadArity(cstdstr& m) : a::Error(m){}
  BadArity(int wanted, int got)
    : a::Error("Expected " + N_STR(wanted) +
               " args, got " + N_STR(got) + ".") {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct BadEval : public a::Error{
  BadEval(cstdstr& m) : a::Error(m) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IndexOOB : public a::Error{
  IndexOOB(cstdstr& m) : a::Error(m) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Unsupported : public a::Error{
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
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef TokenVec::iterator TokenIter;
typedef AstVec::iterator AstIter;
typedef ValVec::iterator ValIter;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VSlice{
  VSlice(ValVec& v){ begin=v.begin(); end=v.end(); }
  VSlice(ValIter b, ValIter e) : begin(b), end(e) {}
  int size(){ return std::distance(begin, end); }
  ValIter begin, end;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//precond checkers
int preEqual(int wanted, int got, cstdstr&);
int preMin(int min, int got, cstdstr&);
int preMax(int max, int got, cstdstr&);
int preNonZero(int c, cstdstr&);
int preEven(int c, cstdstr&);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum TokenType{
  T_INT = 1000000,
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

  T_HASH,
  T_COMMENT,
  T_EOF,
  T_ROGUE,
  T_ETHEREAL
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//A chunk of text - a sequence of chars.
struct Lexeme{

  bool stringy() const { return ttype==T_IDENT || ttype==T_STRING; }
  bool type(int t) const { return t==ttype;}
  Addr addr() const { return info; }
  int type() const { return ttype;}

  virtual double getFloat() const =0;
  virtual stdstr getStr() const =0;
  virtual llong getInt() const =0;
  virtual stdstr pr_str() const =0;
  virtual ~Lexeme() {}

  protected:

  Addr info;
  int ttype;

  Lexeme(int t){ ttype=t; }
  Lexeme(int t, Addr m){ ttype=t;  info=m; }
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Interface for a Lexical scanner.
struct IScanner{

  virtual bool isKeyword(cstdstr&) const = 0;
  virtual DToken skipComment() = 0;
  virtual DToken getNextToken()=0;

  virtual DToken number()=0;
  virtual DToken id()=0;
  virtual DToken string()=0;

  virtual ~IScanner() {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//For lexer, holds all the key attributes.
struct Context{
  Addr mark(){ return DMARK(line,col); }
  ~Context(){}
  Context();
  /////////////////////////////////////////////
  int line, col, pos;
  const Tchar* src;
  size_t len;
  bool eof;
  DToken cur;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SemanticError : public a::Error{
  SemanticError(cstdstr& m) : a::Error(m) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SyntaxError : public a::Error{
  SyntaxError(cstdstr& m) : a::Error(m) {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool is_same(const Data* x, const Data* y);
bool is_same(DValue, const Data* y);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::pair<stdstr,Addr> identifier(Context&, IdPredicate);
std::pair<stdstr,Addr> digits(Context&);
std::pair<stdstr,Addr> numeric(Context&);
std::pair<stdstr,Addr> str(Context&);
Tchar peekAhead(Context&, int offset=1);
bool peekPattern(Context&,cstdstr&);
Tchar peek(Context&);
Tchar pop(Context&);
const std::map<stdstr,int>& getStrTokens();
const std::map<int,stdstr>& getIntTokens();
void skipWhitespace(Context&);
stdstr  pr_addr(Addr);
bool advance(Context&, int steps=1);
Addr mark_advance(Context&, int steps=1);
stdstr unescape(cstdstr&);
stdstr escape(cstdstr&);
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Symbol{
  // int a = 3; a is a symbol, int is a symbol(type), 3 is a value
  static DSymbol make(cstdstr& n, DSymbol t){
    return WRAP_SYM(Symbol, n, t);
  }
  static DSymbol make(cstdstr& n){
    return WRAP_SYM(Symbol, n);
  }
  DSymbol type() const { return _type; }
  stdstr name() const { return _name; }

  ~Symbol(){}

  protected:

  Symbol(cstdstr& n, DSymbol t) : Symbol(n) { _type=t; }
  Symbol(cstdstr& n) : _name(n) { }

  private:

  stdstr _name;
  DSymbol _type;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//A symbol table (with hierarchy)
typedef std::map<stdstr,DSymbol> SymbolMap;
struct Table{

  static DTable make(cstdstr&, const SymbolMap&);
  static DTable make(cstdstr&, DTable);
  static DTable make(cstdstr&);

  DTable outer() const{ return enclosing; }
  stdstr name() const{ return _name; }
  void insert(DSymbol);

  DSymbol search(cstdstr&) const;
  DSymbol find(cstdstr&) const;

  ~Table(){}

  protected:

  Table(cstdstr&, const SymbolMap&);
  Table(cstdstr&, DTable);
  Table(cstdstr&);

  stdstr _name;
  SymbolMap symbols;
  DTable enclosing;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Abstract class to store data value in parsers.
struct Data{
  virtual stdstr pr_str(bool p = 0) const = 0;
  virtual stdstr rtti() const=0;
  virtual bool equals(DValue) const = 0;
  virtual int compare(DValue) const = 0;
  virtual ~Data(){}

  protected:

  Data(){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//A generic number holding either an int or a float
struct Number : public Data{

  static DValue make(double d){
    return WRAP_VAL(Number,d);
  }

  static DValue make(int n){
    return WRAP_VAL(Number,n);
  }

  static DValue make(llong n){
    return WRAP_VAL(Number,n);
  }

  static DValue make(){
    return WRAP_VAL(Number);
  }

  double getFloat() const{ return isInt() ? (double)num.n : num.r; }
  llong getInt() const{ return isInt() ? num.n : (llong) num.r; }

  void setFloat(double d){ num.r=d; }
  void setInt(llong n){ num.n=n; }

  bool isInt() const{ return type== T_INT;}

  bool isZero() const{
    return type==T_INT ? getInt()==0 : a::fuzzy_zero(getFloat()); }

  bool isNeg() const{
    return type==T_INT ? getInt() < 0 : getFloat() < 0.0; }

  bool isPos() const{
    return type==T_INT ? getInt() > 0 : getFloat() > 0.0; }

  virtual stdstr pr_str(bool p=0) const{
    return isInt() ? N_STR(getInt()) : N_STR(getFloat());
  }

  virtual bool equals(DValue) const;
  virtual int compare(DValue) const;

  virtual stdstr rtti() const{ return "Number"; }
  Number() : type(T_INT) { num.n=0; }

  protected:

  explicit Number(double d) : type(T_REAL){ num.r=d; }
  explicit Number(int n) : type(T_INT){ num.n=n; }
  Number(llong n) : type(T_INT){ num.n=n; }

  bool match(const Number*) const;

  int type;
  union { llong n; double r; } num;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct String : public Data{

  virtual stdstr rtti() const{ return "String"; }

  virtual stdstr pr_str(bool p=0) const{
    return p ? "\"" + value + "\"" : value;
  }

  static DValue make(const Tchar* s){
    return WRAP_VAL(String,s);
  }

  static DValue make(cstdstr& s){
    return WRAP_VAL(String,s);
  }

  virtual bool equals(DValue) const;
  virtual int compare(DValue) const;

  stdstr impl() const { return value; }
  // internal use only
  String() {}

  protected:

  stdstr value;
  String(cstdstr& s) : value(s){}
  String(const Tchar* s) : value(s){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//a null or nil
struct Nothing : public Data{

  virtual stdstr pr_str(bool b=0) const{ return "nothing"; }
  virtual stdstr rtti() const{ return "Nothing"; }

  static DValue make(){ return WRAP_VAL(Nothing); }

  virtual bool equals(DValue rhs) const{
    ASSERT1(rhs);
    return is_same(rhs,this);
  }

  virtual int compare(DValue rhs) const{
    ASSERT1(rhs);
    return is_same(rhs,this)
           ? 0 : pr_str().compare(rhs->pr_str());
  }

  virtual ~Nothing() {}

  protected:

  Nothing() {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Interface support for parser evaluation.
struct IEvaluator{
  virtual DValue setValueEx(cstdstr&, DValue) = 0;
  virtual DValue setValue(cstdstr&, DValue) = 0;
  virtual DValue getValue(cstdstr&) const = 0;
  virtual DFrame pushFrame(cstdstr&)=0;
  virtual DFrame popFrame()=0;
  virtual DFrame peekFrame() const =0;
  virtual ~IEvaluator(){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Interface support for abstract syntax tree analysis.
struct IAnalyzer{
  virtual DSymbol search(cstdstr&) const = 0;
  virtual DSymbol find(cstdstr&) const = 0;

  virtual DTable pushScope(cstdstr&) = 0;
  virtual DTable popScope()=0;
  virtual DSymbol define(DSymbol)=0;

  virtual ~IAnalyzer() {}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Abstract node used in the building of a syntax tree.
struct Node{
  virtual DValue eval(IEvaluator*)=0;
  virtual void visit(IAnalyzer*)=0;
  virtual ~Node() {}
  protected:
  Node(){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//A stack frame used during language evaluation.
struct Frame{

  static DFrame search(cstdstr&, DFrame);
  static DFrame make(cstdstr&, DFrame);
  static DFrame make(cstdstr&);
  static DFrame getRoot(DFrame);

  ~Frame(){}

  stdstr name() const{ return _name; }
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
struct TypeSymbol : public Symbol{

  static DSymbol make(cstdstr& n){
    return WRAP_SYM(TypeSymbol, n);
  }

  ~TypeSymbol() {}

  protected:

  TypeSymbol(cstdstr& n) : Symbol(n){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct VarSymbol : public Symbol{

  static DSymbol make(cstdstr& n, DSymbol t){
    return WRAP_SYM(VarSymbol, n,t);
  }

  ~VarSymbol(){}

  protected:

  VarSymbol(cstdstr& n, DSymbol t) : Symbol(n,t){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct FnSymbol : public Symbol{

  static DSymbol make(cstdstr& name, DSymbol t){
    return WRAP_SYM(FnSymbol, name, t);
  }

  static DSymbol make(cstdstr& name){
    return WRAP_SYM(FnSymbol, name);
  }

  DSymbol returnType() const{ return result; }
  SymbolVec& params(){ return _params; }
  DAst body() const{ return block; }
  void setBody(DAst b){ block=b;}

  ~FnSymbol(){}

  protected:

  FnSymbol(cstdstr& name, DSymbol t) : FnSymbol(name){ result=t; }
  FnSymbol(cstdstr& name) : Symbol(name){}

  DSymbol result;
  DAst block;
  SymbolVec _params;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Interface for a parser.
struct IParser{
  virtual DToken eat(int wantedToken)=0;
  virtual DToken eat()=0;
  virtual bool isEof() const =0;
  virtual ~IParser(){}
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Token : public Lexeme{

  static DToken make(int t, cstdstr& s, Addr a){
    auto x=new Token(t,a);
    x->text=s;
    return DToken(x);
  }

  static DToken make(int t, Tchar c, Addr a){
    auto x=new Token(t,a);
    x->text=stdstr{c};
    return DToken(x);
  }

  static DToken make(cstdstr& s, Addr a, double d){
    auto x=new Token(T_REAL,a);
    x->num.r=d;
    x->text=s;
    return DToken(x);
  }

  static DToken make(cstdstr& s, Addr a, llong n){
    auto x=new Token(T_INT,a);
    x->num.n=n;
    x->text=s;
    return DToken(x);
  }

  static DToken make(cstdstr& s, Addr a){
    auto x=new Token(T_STRING,a);
    x->text=s;
    return DToken(x);
  }

  virtual double getFloat() const{
    return type()==T_REAL?num.r:num.n;
  }

  virtual llong getInt() const{
    return type()==T_REAL?num.r:num.n;
  }

  virtual stdstr getStr() const{
    return text;
  }

  virtual stdstr pr_str() const{
    return text;
  }

  virtual ~Token(){}

  protected:

  stdstr text;
  union { llong n; double r; } num;
  Token(int t, Addr m) : Lexeme(t,m){}
};





//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





