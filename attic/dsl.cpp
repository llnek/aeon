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

#include "dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::dsl {
namespace a=czlab::aeon;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, std::string> TOKENS {
  {T_INTEGER, "long"},
  {T_REAL, "double"},
  {T_STRING, "string"},
  {T_IDENT, "id"},
  {T_COMMA, ","},
  {T_COLON, ":"},
  {T_LPAREN, "("},
  {T_RPAREN, ")"},
  {T_LBRACE, "{"},
  {T_RBRACE, "}"},
  {T_EQ, "="},
  {T_MINUS, "-"},
  {T_PLUS, "+"},
  {T_MULT, "*"},
  {T_DIV, "/"},
  {T_GT, ">"},
  {T_LT, "<"},
  {T_SEMI, ";"},
  {T_TILDA, "~"},
  {T_BACKTICK, "`"},
  {T_BANG, "!"},
  {T_AMPER, "&"},
  {T_AT, "@"},
  {T_PERCENT, "%"},
  {T_QUESTION, "?"},
  {T_HAT, "^"},
  {T_LBRACKET, "["},
  {T_RBRACKET, "]"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SyntaxError::SyntaxError(const std::string& x)
  : std::logic_error(x){
}
SyntaxError::SyntaxError(const char* x)
  :std::logic_error(x) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
EvalError::EvalError(const std::string& x)
  : std::logic_error(x){
}
EvalError::EvalError(const char* x)
  :std::logic_error(x) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char peek(Context& ctx) {
  return ctx.src[ctx.pos];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char peekNext(Context& ctx) {
  auto nx = ctx.pos + 1;
  return (nx >= ctx.len) ? '\0': ctx.src[nx];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool advance(Context& ctx) {
  if (ctx.eof) { return false; }
  if (peek(ctx) == '\n') {
    ++ctx.line;
    ctx.col=0;
  }
  ++ctx.pos;
  if (ctx.pos >= ctx.len) {
    ctx.eof=true;
  } else {
    ++ctx.col;
  }
  return !ctx.eof;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void skipWhitespace(Context& ctx) {
  while (!ctx.eof &&
         ::isspace(peek(ctx))) advance(ctx);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string digits(Context& ctx) {
  std::string res;
  while (!ctx.eof && ::isdigit(peek(ctx))) {
    res += peek(ctx);
    advance(ctx);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string numeric(Context& ctx) {
  auto res = digits(ctx);
  if (!ctx.eof && peek(ctx) == '.') {
    res += peek(ctx);
    advance(ctx);
    res += digits(ctx);
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char escSeq(char c) {
  switch (c) {
    case 'a': return '\a'; //	07	Alert (Beep, Bell) (added in C89)[1]
    case 'b': return '\b'; //	08	Backspace
    case 'e': return '\e'; // 1B	Escape character
    case 'f': return '\f'; //	0C	Formfeed Page Break
    case 'n': return '\n'; //	0A	Newline (Line Feed)
    case 'r': return '\r'; //	0D	Carriage Return
    case 't': return '\t'; //	09	Horizontal Tab
    case 'v': return '\v'; //	0B	Vertical Tab
    case '\\': return '\\'; //	5C	Backslash
    case '\'': return '\''; //	27	Apostrophe or single quotation mark
    case '"': return '"'; //	22	Double quotation mark
    case '?': return '\?'; //3F	Question mark (used to avoid trigraphs)
  }
  return c;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string str(Context& ctx) {

  std::string res;
  char ch;

  if (!ctx.eof &&
      peek(ctx) == '"') {
    advance(ctx);
    while (!ctx.eof) {
      ch= peek(ctx);
      if (ch == '"') {
        break;
      }
      if (ch == '\\') {
        if (!advance(ctx)) {
          throw SyntaxError("Malformed string value, bad escaped char.");
        }
        ch=escSeq(peek(ctx));
      }
      res += ch;
      advance(ctx);
    }
    if (ctx.eof || ch != '"') {
      throw SyntaxError("Malformed string value, missing dquote.");
    }
    // good, got the end dquote
    advance(ctx);
  }

  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string identifier(Context& ctx) {
  std::string res;
  char ch;
  while (!ctx.eof) {
    ch=peek(ctx);
    if (res.empty()) {
      if (ch == '_' || ::isalpha(ch)) {
        res += ch;
        advance(ctx);
      } else {
        break;
        //error(ctx);
      }
    } else {
      if (ch == '_' || ::isalpha(ch) || ::isdigit(ch)) {
        res += ch;
        advance(ctx);
      } else {
        break;
        //error(ctx);
      }
    }
  }
  return res;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Context::Context() {
  S_NIL(src);
  len=0;
  line=0;
  col=0;
  pos=0;
  eof=false;
  S_NIL(cur);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TokenInfo Context::mark() {
  return TokenInfo(line,col);
}


////kenl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprInt::ExprInt(long n) : value(n) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String ExprInt::toString(bool prettyPrint) {
  return std::to_string(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
long ExprInt::value() { return value; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool ExprInt::equals(const ExprValue* rhs) {
  return (X_NIL(rhs) && typeid(*this) == typeid(*rhs))
    ? value == static_cast<ExprInt*>(rhs)->value
    : false;
}

class malStringBase : public malValue {
public:
    malStringBase(const String& token)
        : m_value(token) { }
    malStringBase(const malStringBase& that, malValuePtr meta)
        : malValue(meta), m_value(that.value()) { }

    virtual String print(bool readably) const { return m_value; }

    String value() const { return m_value; }

private:
    const String m_value;
};

class malString : public malStringBase {
public:
    malString(const String& token)
        : malStringBase(token) { }
    malString(const malString& that, malValuePtr meta)
        : malStringBase(that, meta) { }

    virtual String print(bool readably) const;

    String escapedValue() const;

    virtual bool doIsEqualTo(const malValue* rhs) const {
        return value() == static_cast<const malString*>(rhs)->value();
    }

    WITH_META(malString);
};

class malKeyword : public malStringBase {
public:
    malKeyword(const String& token)
        : malStringBase(token) { }
    malKeyword(const malKeyword& that, malValuePtr meta)
        : malStringBase(that, meta) { }

    virtual bool doIsEqualTo(const malValue* rhs) const {
        return value() == static_cast<const malKeyword*>(rhs)->value();
    }

    WITH_META(malKeyword);
};

class malSymbol : public malStringBase {
public:
    malSymbol(const String& token)
        : malStringBase(token) { }
    malSymbol(const malSymbol& that, malValuePtr meta)
        : malStringBase(that, meta) { }

    virtual malValuePtr eval(malEnvPtr env);

    virtual bool doIsEqualTo(const malValue* rhs) const {
        return value() == static_cast<const malSymbol*>(rhs)->value();
    }

    WITH_META(malSymbol);
};



class malSequence : public malValue {
public:
    malSequence(malValueVec* items);
    malSequence(malValueIter begin, malValueIter end);
    malSequence(const malSequence& that, malValuePtr meta);
    virtual ~malSequence();

    virtual String print(bool readably) const;

    malValueVec* evalItems(malEnvPtr env) const;
    int count() const { return m_items->size(); }
    bool isEmpty() const { return m_items->empty(); }
    malValuePtr item(int index) const { return (*m_items)[index]; }

    malValueIter begin() const { return m_items->begin(); }
    malValueIter end()   const { return m_items->end(); }

    virtual bool doIsEqualTo(const malValue* rhs) const;

    virtual malValuePtr conj(malValueIter argsBegin,
                              malValueIter argsEnd) const = 0;

    malValuePtr first() const;
    virtual malValuePtr rest() const;

private:
    malValueVec* const m_items;
};

class malList : public malSequence {
public:
    malList(malValueVec* items) : malSequence(items) { }
    malList(malValueIter begin, malValueIter end)
        : malSequence(begin, end) { }
    malList(const malList& that, malValuePtr meta)
        : malSequence(that, meta) { }

    virtual String print(bool readably) const;
    virtual malValuePtr eval(malEnvPtr env);

    virtual malValuePtr conj(malValueIter argsBegin,
                             malValueIter argsEnd) const;

    WITH_META(malList);
};

class malVector : public malSequence {
public:
    malVector(malValueVec* items) : malSequence(items) { }
    malVector(malValueIter begin, malValueIter end)
        : malSequence(begin, end) { }
    malVector(const malVector& that, malValuePtr meta)
        : malSequence(that, meta) { }

    virtual malValuePtr eval(malEnvPtr env);
    virtual String print(bool readably) const;

    virtual malValuePtr conj(malValueIter argsBegin,
                             malValueIter argsEnd) const;

    WITH_META(malVector);
};


class malApplicable : public malValue {
public:
    malApplicable() { }
    malApplicable(malValuePtr meta) : malValue(meta) { }

    virtual malValuePtr apply(malValueIter argsBegin,
                               malValueIter argsEnd) const = 0;
};

class malHash : public malValue {
public:
    typedef std::map<String, malValuePtr> Map;

    malHash(malValueIter argsBegin, malValueIter argsEnd, bool isEvaluated);
    malHash(const malHash::Map& map);
    malHash(const malHash& that, malValuePtr meta)
    : malValue(meta), m_map(that.m_map), m_isEvaluated(that.m_isEvaluated) { }

    malValuePtr assoc(malValueIter argsBegin, malValueIter argsEnd) const;
    malValuePtr dissoc(malValueIter argsBegin, malValueIter argsEnd) const;
    bool contains(malValuePtr key) const;
    malValuePtr eval(malEnvPtr env);
    malValuePtr get(malValuePtr key) const;
    malValuePtr keys() const;
    malValuePtr values() const;

    virtual String print(bool readably) const;

    virtual bool doIsEqualTo(const malValue* rhs) const;

    WITH_META(malHash);

private:
    const Map m_map;
    const bool m_isEvaluated;
};

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

class malAtom : public malValue {
public:
    malAtom(malValuePtr value) : m_value(value) { }
    malAtom(const malAtom& that, malValuePtr meta)
        : malValue(meta), m_value(that.m_value) { }

    virtual bool doIsEqualTo(const malValue* rhs) const {
        return this->m_value->isEqualTo(rhs);
    }

    virtual String print(bool readably) const {
        return "(atom " + m_value->print(readably) + ")";
    };

    malValuePtr deref() const { return m_value; }

    malValuePtr reset(malValuePtr value) { return m_value = value; }

    WITH_META(malAtom);

private:
    malValuePtr m_value;
};



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue(ExprValueType t, long v) : type(t) {
  value.u.n=v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue(ExprValueType t, double v) : type(t) {
  value.u.r=v;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue(ExprValueType t, const std::string& s) : type(t) {
  auto len=s.length();
  value.cs=std::make_shared<a::CString>(len);
  value.cs.get()->copy(s.c_str());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue() {
  type=EXPR_NULL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool ExprValue::isNull() {
  return type == EXPR_NULL;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::~ExprValue() {
  //if (type == EXPR_STR) { del_array(value.s); }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue::ExprValue(const ExprValue& src) {
  value=src.value;
  type=src.type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue& ExprValue::operator=(const ExprValue& src) {
  value=src.value;
  type=src.type;
  return *this;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string ExprValue::toString() {
  switch (type) {
    case EXPR_REAL: return std::to_string(value.u.r);
    case EXPR_INT: return std::to_string(value.u.n); break;
    case EXPR_STR: return std::string(value.cs.get()->get()); break;
    default: return "null";
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const std::string& name, Frame* outer) : name(name) {
  prev=outer;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::Frame(const std::string& name) : name(name) {
  S_NIL(prev);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame::~Frame() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Frame::toString() {
  std::string b(40,'=');
  std::string z(40,'+');
  std::string out;
  char buf[1024];
  ::sprintf(buf, "%s\nName: %s\n%s\n", b.c_str(), name.c_str(),b.c_str());
  out += buf;
  for (auto& x : slots) {
    ::sprintf(buf, "%s = %s\n", x.first.c_str(), x.second.toString().c_str());
    out += buf;
  }
  out += z;
  out += "\n";
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::set<std::string> Frame::keys() {
  std::set<std::string> out;
  for (auto &x : slots) {
    out.insert(x.first);
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ExprValue Frame::get(const std::string& key) {
  auto x= slots.find(key);
  return x != slots.end() ? x->second : (prev ? prev->get(key) : ExprValue());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Frame::set(const std::string& key, const ExprValue& v, bool localOnly) {
  auto x= slots.find(key);


  if (x != slots.end() || localOnly) {
    slots[key]=v;
  } else if (prev) {
    prev->set(key,v,localOnly);
  }
}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CallStack::~CallStack() {
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CallStack::CallStack() {
  S_NIL(top);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::push(const std::string& name) {
  return (top= new Frame(name,top));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::pop() {
  auto x= top;
  if (top) {
    top = top->prev;
  }
  return x;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Frame* CallStack::peek() {
  return top;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::SymbolTable(const std::string& n, SymbolTable* outer)
  : SymbolTable(n) {
  enclosing=outer;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::SymbolTable(const std::string& n) {
  name=n;
  enclosing=nullptr;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymbolTable::~SymbolTable() {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void SymbolTable::insert(Symbol* s) {
  if (s) symbols[s->name] = s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Symbol* SymbolTable::lookup(const std::string& name, bool traverse) {
  auto s = symbols.find(name);
  if (s != symbols.end()) {
    return s->second;
  } else {
    return (!traverse || !enclosing)
      ? nullptr : enclosing->lookup(name);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF





