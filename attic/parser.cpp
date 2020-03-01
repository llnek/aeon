#include "parser.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {
namespace d = czlab::dsl;
BoolExpr* boolExpr(CrenshawParser*);
Compound* code(CrenshawParser*);

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void optional(CrenshawParser* ps, int t) {
  if (ps->isCur(t)) {
    ps->eat();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool isMulop(d::IToken* t) {
  auto n=t->type();
  return n== d::T_MULT || n == d::T_DIV;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool isAddop(d::IToken* t) {
  auto n=t->type();
  return n== d::T_PLUS || n == d::T_MINUS;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool isOrop(d::IToken* t) {
  auto n=t->type();
  return n== T_OR || n == T_XOR;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool isRelop(d::IToken* t) {
  auto n=t->type();
  return n== d::T_GT || n == d::T_LT || n == d::T_EQUALS || n == T_NOTEQ;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast(d::IToken* t) : Ast() {
  token=t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast() : token(nullptr) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(const char* name,
    std::vector<VarDecl*>& d, std::vector<ProcDecl*>& p, Block* block) : Ast() {
  this->block=block;
  _name=name;
  gvars.insert(gvars.end(), d.begin(), d.end());
  procs.insert(procs.end(), p.begin(), p.end());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Program::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(d::IAnalyzer* a) {
  for (auto& v : gvars) {
    v->visit(a);
  }
  //codeProlog();
  for (auto& p : procs) {
    p->visit(a);
  }
  //prolog();
  block->visit(a);
  //epilog();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Program::eval(d::IEvaluator* e) {
  return d::ExprValue();
}


/*
//Store primary to parameter or variable
void store(const char* n) {
  if (isParam(n)) {
    storeParam(paramNumber(n))
  } else {
    checkTable(n);
    storeVar(n);
  }
}
//Load primary from parameter or variable
void load(const char* n) {
  if (isParam(n)) {
    loadParam(paramNumber(n));
  } else {
    checkTable(n);
    loadVar(n);
  }
}
//Increment parameter or variable
void increment(const char* n) {
  if (isParam(n)) {
    incParam(paramNumber(n));
  } else {
    checkTable(n);
    incVar(n);
  }
}
//Increment parameter or variable
void decrement(const char* n) {
  if (isParam(n)) {
    decParam(paramNumber(n));
  } else {
    checkTable(n);
    decVar(n);
  }
}
// Compare parameter or variable with primary
void compare(const char* n) {
  if (isParam(n)) {
    compareParam(paramNumber(n));
  } else {
    checkTable(n);
    compareVar(n);
  }
}
*/

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* factor(CrenshawParser* ps) {

  if (ps->isCur(d::T_LPAREN)) {
    ps->eat();
    auto b= boolExpr(ps);
    ps->eat(d::T_RPAREN);
    return b;
  }

  if (ps->isCur(d::T_IDENT)) {
    return new Var(ps->eat()->getLiteralAsStr());
    //load(Value);
  }

  if (ps->isCur(d::T_REAL) ||
      ps->isCur(d::T_INTEGER)) {
    return new Num(ps->eat());
    //loadConst(Value);
  }

  throw d::SyntaxError("Math Factor");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* multiply(CrenshawParser* ps) {
  auto f= factor(ps);
  //popMul();
  return f;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a divide
Ast* divide(CrenshawParser* ps) {
  auto f= factor(ps);
  //popDiv();
  return f;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate a maths term
Term* term(CrenshawParser* ps) {
  std::vector<Ast*> nodes;
  auto f= factor(ps);

  while (isMulop(ps->token())) {
    //push();
    switch (ps->cur()) {
      case d::T_MULT:
        s__conj(nodes, (ps->eat(d::T_MULT), multiply(ps)));
        break;
      case d::T_DIV:
        s__conj(nodes, (ps->eat(d::T_DIV), divide(ps)));
        break;
    }
  }

  return new Term(f,nodes);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* add(CrenshawParser* ps) {
  auto t= term(ps);
  //popAdd();
  return new Term(t);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* subtract(CrenshawParser* ps) {
  auto t= term(ps);
  //popSub();
  return new Term(t);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Expr* expr(CrenshawParser* ps) {
  std::vector<Ast*> nodes;
  Ast* t= nullptr;

  if (isAddop(ps->token())) {
    //clear();
  } else {
    t=term(ps);
  }

  while (isAddop(ps->token())) {
    //push();
    switch (ps->cur()) {
      case d::T_PLUS:
        s__conj(nodes, (ps->eat(d::T_PLUS), add(ps)));
        break;
      case d::T_MINUS:
        s__conj(nodes, (ps->eat(d::T_MINUS), subtract(ps)));
        break;
    }
  }

  if (t) {
    return new Expr(t, nodes);
  } else {
    return new Expr(nodes);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Get another expression and compare
Ast* compareExpr(CrenshawParser* ps) {
  auto e= expr(ps);
  //popCompare();
  return e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Ast* nextExpr(CrenshawParser* ps) { return compareExpr(ps); }

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a relational "Equals"
Ast* equal(CrenshawParser* ps) {
  ps->eat();
  auto e= compareExpr(ps);
  //setEqual();
  return e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void greaterOrEqual(CrenshawParser* ps) {
  ps->eat();
  compareExpr(ps);
  //setGreaterOrEqual();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void lessOrEqual(CrenshawParser* ps) {
  ps->eat();
  compareExpr(ps);
  //setLessOrEqual();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a relational "Not Equals"
void notEqual(CrenshawParser* ps) {
  ps->eat();
  compareExpr(ps);
  //setNEqual();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Recognize and translate a relational "Less Than"
void less(CrenshawParser* ps) {
  ps->eat();
  compareExpr(ps);
  //setLess();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a relational "Greater Than"
void greater(CrenshawParser* ps) {
  ps->eat();
  compareExpr(ps);
  //setGreater();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a relation
Ast* relation(CrenshawParser* ps) {
  auto e= expr(ps);
  Ast* rhs=nullptr;
  d::IToken* t= ps->token();
  if (isRelop(t)) {
    //push();
    switch (t->type()) {
      case d::T_EQUALS: rhs=equal(ps); break;
      case T_NOTEQ: rhs=notEqual(ps); break;
      case d::T_GT: rhs=greater(ps); break;
      case d::T_LT: rhs=less(ps); break;
      case T_GTEQ: rhs=greaterOrEqual(ps); break;
      case T_LTEQ: rhs=lessOrEqual(ps); break;
    }
  }
  if (rhs) {
    return new Relation(e, t, rhs);
  } else {
    return e;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate a Boolean factor with leading NOT
NotFactor* notFactor(CrenshawParser* ps) {
  bool _not=false;
  Ast* a;
  if (ps->isCur(d::T_BANG)) {
    ps->eat();
    _not=true;
    a= relation(ps);
    //notIt(ps);
  } else {
    a= relation(ps);
  }
  return new NotFactor(_not, a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate a Boolean term
BoolTerm* boolTerm(CrenshawParser* ps) {
  std::vector<Ast*> nodes;
  auto f1= notFactor(ps);
  while (ps->isCur(T_AND)) {
    //push();
    ps->eat(T_AND);
    s__conj(nodes, notFactor(ps));
    //popAnd();
  }
  return new BoolTerm(f1, nodes);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Recognize and translate a Boolean OR
Ast* boolOr(CrenshawParser* ps) {
  auto t= boolTerm(ps);
  //popOr();
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Recognize and translate an exclusive Or
Ast* boolXor(CrenshawParser* ps) {
  auto t= boolTerm(ps);
  //popXor();
  return t;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BoolExpr* boolExpr(CrenshawParser* ps) {
  std::vector<Ast*> nodes;
  auto t1= boolTerm(ps);

  while (isOrop(ps->token())) {
    //push();
    switch (ps->cur()) {
      case T_OR:
        s__conj(nodes, (ps->eat(), boolOr(ps)));
        break;
      case T_XOR:
        s__conj(nodes, (ps->eat(), boolXor(ps)));
        break;
    }
  }

  return new BoolExpr(t1, nodes);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment* assignment(CrenshawParser* ps) {
  auto t= ps->eat(d::T_IDENT);
  auto n= t->getLiteralAsStr();
  auto a = (ps->eat(T_ASSIGN), boolExpr(ps));
  //store(n);
  return new Assignment(new Var(n), a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Recognize and translate an IF construct
If* doIf(CrenshawParser* ps) {
  auto c = (ps->eat(T_IF), boolExpr(ps));
  //auto s1 = newLabel();
  //auto s2 = s1;
  //branchFalse(s1);
  auto t= code(ps);
  Ast* z= nullptr;
  if (ps->isCur(T_ELSE)) {
    ps->eat();
    //s2 = newLabel();
    //branch(s2);
    //postLabel(s1);
    z=code(ps);
  }
  //postLabel(s2);
  ps->eat(T_ENDIF);

  return new If(c, t, z);
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate a WHILE statement
While* doWhile(CrenshawParser* ps) {
  ps->eat(T_WHILE);
  //auto s1 = newLabel();
  //auto s2 = newLabel();
  //postLabel(s1);
  auto c= boolExpr(ps);
  //branchFalse(s2);
  auto e= code(ps);
  ps->eat(T_ENDWHILE);
  //branch(s1);
  //postLabel(s2);

  return new While(c, e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a REPEAT statement
Repeat* doRepeat(CrenshawParser* ps) {
  ps->eat(T_REPEAT);
  //auto s = newLabel();
  //postLabel(s);
  auto e= code(ps);
  auto c = (ps->eat(T_UNTIL),boolExpr(ps));
  //branchFalse(s);

  return new Repeat(e,c);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a FOR statement
For* doFor(CrenshawParser* ps) {
  // FOR x = 1+3
  //auto s1 = newLabel();
  //auto s2 = newLabel();
  auto t = (ps->eat(T_FOR), ps->eat(d::T_IDENT));
  auto name= t->getLiteralAsStr();
  ps->eat(d::T_EQUALS);
  auto i= expr(ps);
  //store(name);
  //decrement(name);
  auto z= expr(ps);
  //push();
  //postLabel(s1);
  //increment(name);
  //pop();
  //compare(name);
  //branchGreater(s2);
  //push();
  auto e= code(ps);
  ps->eat(T_ENDFOR);
  //branch(s1);
  //postLabel(s2);

  return new For(new Var(name), i,z,e);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Process a read statement
void doRead(CrenshawParser* ps) {
  ps->eat(d::T_LPAREN);
  if (ps->isCur(d::T_RPAREN)) {
    //readKey();
  } else {
    //readAndStore(ps);
    while (ps->isCur(d::T_COMMA)) {
      ps->eat();
      //readAndStore(ps);
    }
  }
  ps->eat(d::T_RPAREN);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Same as DoRead, compatability with Pascal
void doReadLn(CrenshawParser* ps) {
  doRead(ps);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void stringOrVar(const char* os) {
  if (*os) {
    //writeString(os);
  } else {
    expr(ps);
    //writeIt();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Process a write statement.
void doWrite(CrenshawParser* ps) {
  ps->eat(d::T_LPAREN);
  if (!ps->isCur(d::T_RPAREN)) {
    //stringOrVar(getString(ps));
    while (ps->isCur(d::T_COMMA)) {
      ps->eat();
      //stringOrVar(getString());
    }
  }
  ps->eat(d::T_RPAREN);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void doWriteLn(CrenshawParser* ps) {
  doWrite(ps);
  //newLine();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process a formal parameter.
void formalParam() {
  addParam(Value);
  next();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process the formal parameter list of a procedure.
void formalList(CrenshawParser* ps) {
  ps->eat(d::T_LPAREN);
  if (!ps->isCur(d::T_RPAREN)) {//::strcmp(Token, ")") != 0) {
    formalParam(ps);
    while (ps->isCur(d::T_COMMA)) {
      ps->eat();
      formalParam(ps);
    }
  }
  ps->eat(d::T_RPAREN);
//  Base = NumParams;
  //NumParams += 2;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process an actual parameter.
Expr* param(CrenshawParser* ps) {
  auto e= expr(ps);
  //push();
  return e;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process the parameter list for a procedure call.
int paramList(CrenshawParser* ps) {
  auto n = 0;
  ps->eat(d::T_LPAREN);
  if (!ps->isCur(d::T_RPAREN)) {//::strcmp(Token, ")") != 0) {
    param(ps);
    //inc(n);
    while (ps->isCur(d::T_COMMA)) {//::strcmp(Token, ",")==0) {
      ps->eat();
      param(ps);
      //inc(n);
    }
  }
  ps->eat(d::T_RPAREN);
//  ParamList = 4 * n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process a procedure call.
void callProc(CrenshawParser* ps) {
  auto p=ps->eat(d::T_IDENT);
  std::vector<Ast*> pms;

  ps->eat(d::T_LPAREN);

  if (!ps->isCur(d::T_RPAREN)) {
    pms.push_back( expr(ps));
  }

  while (ps->isCur(d::T_COMMA)) {
    ps->eat();
    pms.push_back(expr(ps));
  }

  ps->eat(d::T_RPAREN);

/////
  auto n = paramList(ps);
  //call(n);
  //cleanStack(n);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a local data declaration.
/*
void locDecl() {
  next();
  addParam(Value);
  next();
}
*/
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate local declarations.
/*
int locDecls() {
  auto n = 0;
  scan();
  while (::strcmp(Token, "v")==0) {
    locDecl();
    inc(n);
  }
  while (::strcmp(Token, ",")==0) {
    locDecl();
    inc(n);
  }
  return n;
}
*/
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* assignOrProc(CrenshawParser* ps) {

  auto t= ps->token();

  if (t->type() == d::T_IDENT) {
    if (ps->peek() == '(') {
      return callProc(ps);
    } else {
      return assignment(ps);
    }
  } else {
    throw d::SyntaxError("");
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool xblock(d::IToken* t) {
  switch (t->type()) {
    case T_ENDIF:
    case T_UNTIL:
    case T_ENDFOR:
    case T_END:
    case T_ELSE:
    case T_ENDWHILE: return false;
    default: return true;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//A block of statements.
Compound* code(CrenshawParser* ps) {
  while (xblock(ps->lex->ctx.cur)) {
    switch (ps->lex->ctx.cur->type()) {
      case T_IF: doIf(ps); break;
      case T_WHILE: doWhile(ps); break;
      case T_REPEAT: doRepeat(ps); break;
      case T_FOR: doFor(ps); break;
      case T_READ: doRead(ps); break;
      case T_READLN: doReadLn(ps); break;
      case T_WRITE: doWrite(ps); break;
      case T_WRITELN: doWriteLn(ps); break;
      default:
      assignOrProc(ps);
    }
    semi(ps);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Allocate storage for a variable.


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/* // Parse and translate a procedure declaration.
void decl_proc(CrenshawParser* ps) {
  auto t = (ps->eat(T_PROCEDURE), ps->eat(d::T_IDENT));
  auto pn= t->getLiteralAsStr()();
  //checkDup(pn);
  //addEntry(Value, 'p');

  char n[256];
  ::strcpy(n,Value);
  formalList();
  semi();
  auto k = locDecls();
  procProlog(n, k);

  auto len = Base + 2 + k;
  for (auto i= Base+3; i < len; ++i) { //must init locals only
    clear();
    storeParam(i);
  }

  semi();
  matchString("BEGIN");
  block();
  matchString("END");
  semi();
  procEpilog();
  clearParams();
  scan();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void gvar(CrenshawParser* ps) {
  auto t= ps->eat(T_IDENT);
  auto n= t->getLiteralAsStr();
  ps->checkDup(n);
  addEntry(Value, 'v'); allocate(Value, '0');
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void topDecls(CrenshawParser* ps) {
  while (ps->lex->ctx.cur->type() == T_VAR) {
    gvar(ps);
  }
  while (ps->lex->ctx.cur->type() == T_COMMA) {
    gvar(ps);
  }
  semi();
}
*/
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcCall* proccall(CrenshawParser* ps) {
  auto n= ps->eat(d::T_IDENT);
  std::vector<Expr*> pms;
  ps->eat(d::T_LPAREN);

  if (!ps->isCur(d::T_RPAREN)) {
    // 1st param
    s__conj(pms, expr(ps));
  }

  while (ps->isCur(d::T_COMMA)) {
    ps->eat(d::T_COMMA);
    s__conj(pms, expr(ps));
  }

  ps->eat(d::T_RPAREN);

  return new ProcCall(new Var(n->getLiteralAsStr()), pms);
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Param*> formal_parameters(CrenshawParser* ps) {

  std::vector<d::IToken*> param_tokens {ps->eat(d::T_IDENT)};
  std::vector<Param*> pnodes;

  while (ps->isCur(d::T_COMMA)) {
    ps->eat(d::T_COMMA);
    s__conj(param_tokens, ps->eat(d::T_IDENT));
  }

  for (auto& t : param_tokens) {
    s__conj(pnodes, (new Param(new Var(t), new Type())));
  }

  return pnodes;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Param*> formal_parameter_list(CrenshawParser* ps) {
  std::vector<Param*> out;

  if (!ps->isCur(d::T_IDENT)) {
    return out;
  }

  auto pnodes = formal_parameters(ps);
  while (ps->isCur(d::T_SEMI)) {
    ps->eat(d::T_SEMI);
    auto pms = formal_parameters(ps);
    pnodes.insert(pnodes.end(), pms.begin(), pms.end());
  }

  return pnodes;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcDecl* decl_proc(CrenshawParser* ps, bool global) {
  auto t= (ps->eat(T_PROCEDURE), ps->eat(d::T_IDENT));
  auto pname = t->getLiteralAsStr();
  std::vector<Param*> params;

  if (ps->isCur(d::T_LPAREN)) {
    ps->eat();
    params = formal_parameter_list(ps);
    ps->eat(d::T_RPAREN);
  }

  optional(ps, d::T_SEMI);
  auto p = new ProcDecl(pname, params, code(ps,false), global);
  optional(ps, d::T_SEMI);

  return p;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type* type_spec(CrenshawParser* ps) {
  auto tkn = ps->token();
  switch (tkn->type()) {
    //case T_STR: ps->eat(T_STR); break;
    //case T_INT: ps->eat(T_INT); break;
    case T_REAL: ps->eat(T_REAL); break;
    default:
      ::sprintf(MSGBUF, "Unsupported type %s.", to_string(tkn));
      throw d::SyntaxError(MSGBUF);
  }
  return new Type(tkn);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<VarDecl*> var_decl(CrenshawParser* ps, bool global) {
  // deal with the 1st var
  std::vector<Var*> vars { new Var(ps->token()) };
  ps->eat(d::T_IDENT);
  // if more,...
  while (ps->isCur(d::T_COMMA)) {
    ps->eat(d::T_COMMA);
    s__conj(vars, (new Var(ps->token())));
    ps->eat(d::T_IDENT);
  }

  // expect the type next
  ps->eat(d::T_COLON);

  auto type = type_spec(ps);
  std::vector<VarDecl*> out;
  for (auto &x : vars) {
    s__conj(out, (new VarDecl(x, type, global)));
  }

  return out;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<VarDecl*> decl_vars(CrenshawParser* ps, bool global) {
  std::vector<VarDecl*> ds;

  if (ps->isCur(T_VAR)) {
    ps->eat();
    while (ps->isCur(d::T_IDENT)) {
      auto vs = var_decl(ps, global);
      ds.insert(ds.end(),vs.begin(),vs.end());
      ps->eat(d::T_SEMI);
    }
  }
  optional(ps, d::SEMI);
  return ds;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<ProcDecl*> decl_procs(CrenshawParser* ps, bool global) {
  std::vector<ProcDecl*> ps;

  while (ps->isCur(T_PROCEDURE)) {
    s__conj(ds, (decl_proc(ps, global)));
  }

  return ds;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block* block(CrenshawParser* ps, bool global) {
  return new Block(decls(ps, global), code(ps));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program* program(CrenshawParser* ps) {
  auto pname= (ps->eat(T_PROGRAM), ps->eat(T_IDENT));
  p->eat(T_SEMI);
  auto d= decl_vars(ps,true);
  auto p= decl_procs(ps, true);
  return new Program(pname->getLiteralAsStr(), d, p, code(ps,true));
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CrenshawParser::~CrenshawParser() {
  delete lex;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CrenshawParser::CrenshawParser(const char* src) {
  lex = new Lexer(src);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IAst* CrenshawParser::parse() {
  return program(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* CrenshawParser::eat(int wanted) {
  auto t= lex->ctx.cur;
  if (t->type() != wanted) {
    ::sprintf(MSGBUF, "Expecting token[%d], got[%d] instead.", wanted, t->type());
    throw d::SyntaxError(MSGBUF);
  }
  lex->ctx.cur=lex->getNextToken();
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* CrenshawParser::eat() {
  // just eat it
  auto t= lex->ctx.cur;
  lex->ctx.cur=lex->getNextToken();
  return t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool CrenshawParser::isCur(int token_type) {
  return lex->ctx.cur->type() == token_type;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int CrenshawParser::cur() {
  return lex->ctx.cur->type();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
char CrenshawParser::peek() {
  return d::peek(lex->ctx);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IToken* CrenshawParser::token() {
  return lex->ctx.cur;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF
/*
<b-expression> ::= <b-term> [<orop> <b-term>]*
 <b-term>       ::= <not-factor> [AND <not-factor>]*
 <not-factor>   ::= [NOT] <b-factor>
 <b-factor>     ::= <b-literal> | <b-variable> | <relation>
 <relation>     ::= | <expression> [<relop> <expression]
 <expression>   ::= <term> [<addop> <term>]*
 <term>         ::= <signed factor> [<mulop> factor]*
 <signed factor>::= [<addop>] <factor>
 <factor>       ::= <integer> | <variable> | (<b-expression>)

 */


