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

#define DSL_ERROR(EXP, fmt, ...) \
  do { char buf[1024]; \
  ::sprintf(buf, (const char*)fmt, __VA_ARGS__); throw EXP(buf);} while (0)

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::dsl {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a=czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
typedef s::vector<s::string> StrVec;
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

  protected:

  ~IToken() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IScanner {

  virtual bool isKeyword(const std::string&) = 0;
  virtual IToken* getNextToken()=0;
  virtual IToken* number()=0;
  virtual IToken* id()=0;
  virtual IToken* string()=0;
  virtual void skipComment() = 0;

  protected:

  ~IScanner() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ValueSlot {
  std::shared_ptr<a::CString> cs;
  union {
    long n;
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
  TokenInfo(int line, int col) : line (line), col(col) {
  }
  int line,col;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Context {
  ~Context() {}
  Context();

  TokenInfo mark();

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
  SyntaxError(const std::string&);
  SyntaxError(const char*);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct EvalError {
  EvalError(const std::string&);
  EvalError(const char*);
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skipWhitespace(Context&);
bool advance(Context&);
char peek(Context&);
char peekNext(Context&);
std::string str(Context&);
std::string numeric(Context&);
std::string identifier(Context&);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SValue : public a::Counted {
  virtual ExprValue eval(IEvaluator*, malEnvPtr env) = 0;
  virtual SString toString(bool prettyPrint) = 0;
  virtual bool equals(const SValue*) = 0;
  virtual ~SValue() {}
  SValue() {}
};
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SInt : public SValue {
  virtual std::string toString(bool prettyPrint);
  virtual bool equals(const SValue* rhs);
  SInt(long n);
  long impl();
  private:
  long value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct StringBase : public SValue {
  StringBase(const StringBase& that, ExprValue meta);
  StringBase(const std::string& token);
  virtual std::string toString(bool pretty);
  std::string impl();
  private:
  std::string value;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SString : public StringBase {
  SString(const std::string& token);
  SString(const SString& that, ExprValue meta);
  virtual std::string toString(bool pretty);
  std::string escapedValue();
  virtual bool equals(const SValue* rhs);
  virtual ExprValue doWithMeta(ExprValue meta) {
    return new SString(*this, meta);
  };
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SKeyword : public StringBase {
  SKeyword(const st::string& token);
  SKeyword(const SKeyword& that, ExprValue meta);
  virtual bool equals(const SValue* rhs);
  virtual ExprValue doWithMeta(ExprValue meta) {
    return new SKeyword(*this, meta);
  };
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SSymbol : public StringBase {
  SSymbol(const std::string& token);
  SSymbol(const SSymbol& that, ExprValue meta);
  virtual bool equals(const SValue* rhs);
  virtual ExprValue doWithMeta(ExprValue meta) {
    return new SSymbol(*this, meta);
  };
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SSeq : public SValue {
  SSeq(malValueVec* items);
  SSeq(malValueIter begin, malValueIter end);
  SSeq(const SSeq& that, ExprValue meta);
  virtual ~SSeq();
  virtual std::string toString(bool pretty);

  malValueVec* evalItems(malEnvPtr env);
  int count();
  bool isEmpty();
  malValuePtr at(int index);

  malValueIter begin();
  malValueIter end();

  virtual bool equals(const SValue* rhs);
  virtual ExprValue conj(malValueIter argsBegin,
                              malValueIter argsEnd) = 0;

  ExprValue first();
  virtual ExprValue rest();

  private:

  malValueVec* const m_items;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SList : public SSeq {
  SList(malValueVec* items);
  SList(malValueIter begin, malValueIter end);
  SList(const malList& that, malValuePtr meta);
  virtual std::string toString(bool pretty);
  virtual malValuePtr eval(malEnvPtr env);
  virtual ExprValue conj(malValueIter argsBegin,
                             malValueIter argsEnd);
  virtual ExprValue doWithMeta(ExprValue meta) {
    return new SList(*this, meta);
  };
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SVec : public SSeq {

  SVec(malValueVec* items);
  SVec(malValueIter begin, malValueIter end);
  SVec(const malVector& that, malValuePtr meta);

  virtual malValuePtr eval(malEnvPtr env);
  virtual std::string toString(bool pretty);

  virtual ExprValue conj(malValueIter argsBegin,
                             malValueIter argsEnd);
  virtual ExprValue doWithMeta(ExprValue meta) {
    return new SVec(*this, meta);
  };

};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SApplicable : public SValue {

  SApplicable(ExprValue meta);
  SApplicable();

  virtual ExprValue apply(malValueIter argsBegin,
                               malValueIter argsEnd) = 0;
};

typedef std::map<std::string, ExprValue> Map;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SMap : public SValue {

  SMap(malValueIter argsBegin, malValueIter argsEnd, bool isEvaluated);
  SMap(const Map& map);
  SMap(const SMap& that, ExprValue meta);

  ExprValue assoc(malValueIter argsBegin, malValueIter argsEnd) const;
  ExprValue dissoc(malValueIter argsBegin, malValueIter argsEnd) const;
  bool contains(ExprValue key) const;
  ExprValue eval(malEnvPtr env);
  ExprValue get(ExprValue key) const;
  ExprValue keys() const;
  ExprValue values() const;

  virtual std::string toString(bool pretty);

  virtual bool equals(const SValue* rhs);

  virtual ExprValue doWithMeta(ExprValue meta) {
    return new SMap(*this, meta);
  };

  private:

  Map m_map;
  bool m_isEvaluated;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
class malBuiltIn : public malApplicable {
public:
    typedef malValuePtr (ApplyFunc)(const String& name,
                                    malValueIter argsBegin,
                                    malValueIter argsEnd);

    malBuiltIn(const String& name, ApplyFunc* handler)
    : m_name(name), m_handler(handler) { }

    malBuiltIn(const malBuiltIn& that, malValuePtr meta)
    : malApplicable(meta), m_name(that.m_name), m_handler(that.m_handler) { }

    virtual malValuePtr apply(malValueIter argsBegin,
                              malValueIter argsEnd) const;

    virtual String print(bool readably) const {
        return STRF("#builtin-function(%s)", m_name.c_str());
    }

    virtual bool doIsEqualTo(const malValue* rhs) const {
        return this == rhs; // these are singletons
    }

    String name() const { return m_name; }

    WITH_META(malBuiltIn);

private:
    const String m_name;
    ApplyFunc* m_handler;
};
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
class malLambda : public malApplicable {
public:
    malLambda(const StringVec& bindings, malValuePtr body, malEnvPtr env);
    malLambda(const malLambda& that, malValuePtr meta);
    malLambda(const malLambda& that, bool isMacro);

    virtual malValuePtr apply(malValueIter argsBegin,
                              malValueIter argsEnd) const;

    malValuePtr getBody() const { return m_body; }
    malEnvPtr makeEnv(malValueIter argsBegin, malValueIter argsEnd) const;

    virtual bool doIsEqualTo(const malValue* rhs) const {
        return this == rhs; // do we need to do a deep inspection?
    }

    virtual String print(bool readably) const {
        return STRF("#user-%s(%p)", m_isMacro ? "macro" : "function", this);
    }

    bool isMacro() const { return m_isMacro; }

    virtual malValuePtr doWithMeta(malValuePtr meta) const;

private:
    const StringVec   m_bindings;
    const malValuePtr m_body;
    const malEnvPtr   m_env;
    const bool        m_isMacro;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct SAtom : public SValue {

  SAtom(const SAtom& that, ExprValue meta);
  SAtom(ExprValue value);

  virtual bool equals(const ExprValue* rhs);
  virtual std::string toString(bool pretty);

  ExprValue deref();
  ExprValue reset(ExprValue value);

  virtual ExprValue doWithMeta(ExprValue meta) {
    return new SAtom(*this, meta);
  }

  private:

  ExprValue m_value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
enum ExprValueType {
  EXPR_INT = 2000000,
  EXPR_REAL,
  EXPR_STR,
  EXPR_NULL
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IEvaluator {
/*
  virtual void setValue(const std::string&, const ExprValue&, bool localOnly=true) = 0;
  virtual ExprValue getValue(const std::string&) = 0;
  virtual Frame* push(const std::string& name)=0;
  virtual Frame* pop()=0;
  virtual Frame* peek()=0;

  virtual std::string readString()=0;
  virtual double readFloat()=0;
  virtual long readInt()=0;

  virtual void writeString(const std::string&)=0;
  virtual void writeFloat(double)=0;
  virtual void writeInt(long)=0;
  virtual void writeln()=0;

  protected:
*/
  ~IEvaluator() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IAnalyzer {
/*
  virtual Symbol* lookup(const std::string&, bool traverse=true) = 0;
  virtual void pushScope(const std::string& name) =0;
  virtual SymbolTable* popScope()=0;
  virtual void define(Symbol*)=0;

  protected:
*/
  ~IAnalyzer() {}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IValue {
  virtual std::string toString() = 0;
  virtual ~IValue() {}
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct IAst {
  virtual void eval(IEvaluator*)=0;
  virtual ~IAst() {}
};



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ExprValue : public IValue {

  ExprValue(ExprValueType t, const std::string&);
  ExprValue(ExprValueType t, long v);
  ExprValue(ExprValueType t, double v);

  virtual std::string toString();
  bool isNull();
  ExprValue();
  ~ExprValue();

  ExprValue& operator=(const ExprValue&);
  ExprValue(const ExprValue&);
  //ExprValue(ExprValue&&);
  //ExprValue& operator=(ExprValue&&);

  ExprValueType type;
  ValueSlot value;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Frame {

  void set(const std::string&, const ExprValue&, bool localOnly=true);
  ExprValue get(const std::string& n);
  std::set<std::string> keys();
  std::string toString();

  Frame(const std::string& name, Frame* outer);
  Frame(const std::string& name);
  ~Frame();

  std::string name;
  Frame* prev;

  protected:

  std::map<std::string,ExprValue> slots;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct CallStack {

  Frame* push(const std::string& name);
  Frame* pop();
  Frame* peek();

  CallStack();
  ~CallStack();

  protected:

  Frame* top;
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct Symbol {

  Symbol(const std::string& n, Symbol* t) : Symbol(n) { type=t; }
  Symbol(const std::string& n) : name(n) { type=nullptr; }
  ~Symbol() {}

  Symbol* type;
  std::string name;
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
    block=nullptr;
    result=nullptr;
  }
  ~FunctionSymbol() {}

  TypeSymbol* result;
  IAst* block;
  std::vector<ParamSymbol*> params;
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
struct IParser {

  virtual IToken* eat(int wantedToken)=0;
  virtual IAst* parse()=0;

  protected:

  ~IParser() {}
};



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





