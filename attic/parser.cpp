#include "../dsl/dsl.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

//Check to make sure the current token is an identifier
void checkIdent() {
  if (::strcmp(Token, "x") != 0) {
    expected("Identifier");
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Store primary to parameter or variable
void store(const char* n) {
  if (isParam(n)) {
    storeParam(paramNumber(n))
  } else {
    checkTable(n);
    storeVar(n);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Load primary from parameter or variable
void load(const char* n) {
  if (isParam(n)) {
    loadParam(paramNumber(n));
  } else {
    checkTable(n);
    loadVar(n);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Increment parameter or variable
void increment(const char* n) {
  if (isParam(n)) {
    incParam(paramNumber(n));
  } else {
    checkTable(n);
    incVar(n);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Increment parameter or variable
void decrement(const char* n) {
  if (isParam(n)) {
    decParam(paramNumber(n));
  } else {
    checkTable(n);
    decVar(n);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Compare parameter or variable with primary
void compare(const char* n) {
  if (isParam(n)) {
    compareParam(paramNumber(n));
  } else {
    checkTable(n);
    compareVar(n);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate a math factor
void factor() {
  if (::strcmp(Token, "(")==0) {
    next();
    boolExpr();
    matchString(")");
  } else {
    if (::strcmp(Token, "x")==0) {
      load(Value);
    } else if (::strcmp(Token, "#")==0) {
      loadConst(Value);
    } else {
      expected("Math Factor");
    }
    next();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a multiply
void multiply() {
  next();
  factor();
  popMul();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a divide
void divide() {
  next();
  factor();
  popDiv();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate a maths term
void term() {
  factor();
  while (isMulop(Token)) {
    push();
    switch (Token[0]) {
      case '*' : multiply(); break;
      case '/' : divide(); break;
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate an add
void add() {
  next();
  term();
  popAdd();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a subtract
void subtract() {
  next();
  term();
  popSub();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate an expression
void expr() {
  if (isAddop(Token)) {
    clear();
  } else {
    term();
  }
  while (isAddop(Token)) {
    push();
    switch (Token[0]) {
      case '+' : add(); break;
      case '-' : subtract(); break;
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Get another expression and compare
void compareExpr() {
  expr();
  popCompare();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Get the next expression and compare
void nextExpr() {
  next();
  compareExpr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a relational "Equals"
void equal() {
  nextExpr();
  setEqual();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a relational "Less Than or Equal"
void lessOrEqual() {
  nextExpr();
  setLessOrEqual();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a relational "Not Equals"
void notEqual() {
  nextExpr();
  setNEqual();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Recognize and translate a relational "Less Than"
void less() {
  next();
  switch (Token[0]) {
    case '=' : lessOrEqual(); break;
    case '>' : notEqual(); break;
    default:
      compareExpr();
      setLess();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize and translate a relational "Greater Than"
void greater() {
  next();
  if (::strcmp(Token, "=")==0) {
    nextExpr();
    setGreaterOrEqual();
  } else {
    compareExpr();
    setGreater();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a relation
void relation() {
  expr();
  if (isRelop(Token)) {
    push();
    switch (Token[0]) {
      case '=' : equal(); break;
      case '<' : less(); break;
      case '>' : greater(); break;
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate a Boolean factor with leading NOT
void notFactor() {
  if (::strcmp(Token, "!")==0) {
    next();
    relation();
    notIt();
  } else {
    relation();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate a Boolean term
void boolTerm() {
  notFactor();
  while (::strcmp(Token, "&")==0) {
    push();
    next();
    notFactor();
    popAnd();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Recognize and translate a Boolean OR
void boolOr() {
  next();
  boolTerm();
  popOr();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Recognize and translate an exclusive Or
void boolXor() {
  next();
  boolTerm();
  popXor();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a Boolean expression
void boolExpr() {
  boolTerm();
  while (isOrOp(Token)) {
    push();
    switch (Token[0]) {
      case '|' : boolOr(); break;
      case '~' : boolXor(); break;
    }
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate an assignment statement
void assignment(const char* n) {
  next();
  matchString("=");
  boolExpr();
  store(n);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Recognize and translate an IF construct
void doIf() {
  next();
  boolExpr();

  auto s1 = newLabel();
  auto s2 = s1;
  branchFalse(s1);
  block();

  if (::strcmp(Token, "l")==0) {
    next();
    s2 = newLabel();
    branch(s2);
    postLabel(s1);
    block();
  }

  postLabel(s2);
  matchString("ENDIF");
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Parse and translate a WHILE statement
void doWhile() {
  next();
  auto s1 = newLabel();
  auto s2 = newLabel();
  postLabel(s1);
  boolExpr();
  branchFalse(s2);
  block();
  matchString("ENDWHILE");
  branch(s1);
  postLabel(s2);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a REPEAT statement
void doRepeat() {
  matchString("REPEAT");
  auto s = newLabel();
  postLabel(s);
  block();
  matchString("UNTIL");
  boolExpr();
  branchFalse(s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a FOR statement
void doFor() {
  matchString("FOR");
  auto s1 = newLabel();
  auto s2 = newLabel();
  char name[256];
  ::strcpy(name, Value);
  next();
  matchString("=");
  expr();
  store(name);
  decrement(name);
  expr();
  push();
  postLabel(s1);
  increment(name);
  pop();
  compare(name);
  branchGreater(s2);
  push();
  block();
  matchString("ENDFOR");
  branch(s1);
  postLabel(s2);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Process a read statement
void doRead() {
  next();
  matchString("(");
  if (::strcmp(Token, ")")==0) {
    readKey();
  } else {
    readAndStore(Value);
    while (::strcmp(Token, ",")==0) {
      next();
      readAndStore(Value);
    }
  }
  matchString(")");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Same as DoRead, compatability with Pascal
void doReadLn() {
  doRead();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void stringOrVar(const char* os) {
  if (*os) {
    writeString(os);
  } else {
    expr();
    writeIt();
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Process a write statement.
void doWrite() {
  next();
  matchString("(");
  if (::strcmp(Token, ")") != 0) {
    stringOrVar(getString());
    while (::strcmp(Token, ",")==0) {
      next();
      stringOrVar(getString());
    }
  }
  matchString(")");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void doWriteLn() {
  doWrite();
  newLine();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process a formal parameter.
void formalParam() {
  addParam(Value);
  next();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process the formal parameter list of a procedure.
void formalList() {
  next();
  matchString("(");
  if (::strcmp(Token, ")") != 0) {
    formalParam();
    while (::strcmp(Token, ",")==0) {
      next();
      formalParam();
    }
  }
  matchString(")");
  Base = NumParams;
  NumParams += 2;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process an actual parameter.
void param() {
  expr();
  push();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process the parameter list for a procedure call.
int paramList() {
  auto n = 0;
  next();
  matchString("(");
  if (::strcmp(Token, ")") != 0) {
    param();
    inc(n);
    while (::strcmp(Token, ",")==0) {
      matchString(",");
      param();
      inc(n);
    }
  }
  matchString(")");
  ParamList = 4 * n;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Process a procedure call.
void callProc(const char* n) {
  auto n = paramList();
  call(n);
  cleanStack(n);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a local data declaration.
void locDecl() {
  next();
  addParam(Value);
  next();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate local declarations.
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

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void doAssignOrProc() {
  auto x= typeOf(Value);
  switch (x[0]) {
    case ' ' : undefined(Value); break;
    case 'v':
    case'F' : assignment(Value); break;
    case 'p' : callProc(Value); break;
    default:
    ::sprintf(MSGBUF, "Identifier %s cannot be used here.", Value);
    abort(MSGBUF);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* statement(CrenshawParser* ps) {
  Ast* node;
  switch (ps->lex->ctx.cur->type()) {
    case T_BEGIN:
      node = compound_statement(ps);
      break;


    case d::T_IDENT:
      if (d::peek(ps->lex->ctx) == '(') {
        node = proccall_statement(ps);
      } else {
        node = assignment_statement(ps);
      }
      break;
    default:
      node = empty(ps);
      break;
  }
  return node;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool xblock(int type) {
  switch (type) {
    case T_ENDIF:
    case T_UNTIL:
    case T_ENDFOR:
    case T_END:
    case T_ELSE:
    case T_ENDWHILE: return true;
    default: return false;
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Ast*> statement_list(CrenshawParser* ps) {

  while (!xblock(ps->lex->ctx.cur->type())) {
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
        doAssignOrProc(ps); break;
    }
    ps->eat(d::T_SEMI);
  }

  std::vector<Ast*> results {
    statement(ps)
  };

  while (ps->lex->ctx.cur->type() == d::T_SEMI) {
    ps->eat(d::T_SEMI);
    results.push_back(statement(ps));
  }

  if (ps->lex->ctx.cur->type() == d::T_IDENT) {
    ::sprintf(MSGBUF, "Unexpected identifier `%s`", ps->lex->ctx.cur->getLiteralAsStr());
    throw d::SyntaxError(MSGBUF);
  }

  return results;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound* compound_statement(CrenshawParser* ps) {
  ps->eat(T_BEGIN);
  auto nodes = statement_list(ps);
  ps->eat(T_END);
  auto root= new Compound();
  for (auto& node : nodes) {
    root->children.push_back(node);
  }
  return root;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl* procedure_declaration(CrenshawParser* ps) {

  ps->eat(T_PROCEDURE);

  auto proc_name = ps->lex->ctx.cur->getLiteralAsStr();
  std::vector<Param*> params;

  ps->eat(d::T_IDENT);

  if (ps->lex->ctx.cur->type() == d::T_LPAREN) {
    ps->eat(d::T_LPAREN);
    params = formal_parameter_list(ps);
    ps->eat(d::T_RPAREN);
  }

  ps->eat(d::T_SEMI);

  auto block_node = block(ps);
  auto proc_decl = new ProcedureDecl(proc_name, params, block_node);
  ps->eat(d::T_SEMI);

  return proc_decl;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Ast*> declarations(CrenshawParser* ps) {
  std::vector<Ast*> ds;

  if (ps->lex->ctx.cur->type() == T_VAR) {
    ps->eat(T_VAR);
    while (ps->lex->ctx.cur->type() == d::T_IDENT) {
      auto vs = variable_declaration(ps);
      ds.insert(ds.end(),vs.begin(),vs.end());
      ps->eat(d::T_SEMI);
    }
  }

  while (ps->lex->ctx.cur->type() == T_PROCEDURE) {
    ds.push_back(procedure_declaration(ps));
  }

  return ds;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block* block(CrenshawParser* ps) {
  auto decls = declarations(ps);
  auto cs = compound_statement(ps);
  return new Block(decls, cs);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program* program(CrenshawParser* ps) {
  ps->eat(T_PROGRAM);
  auto var_node = variable(ps);
  auto prog_name = var_node->name();
  ps->eat(d::T_SEMI);
  auto block_node = block(ps);
  auto program_node = new Program(prog_name.c_str(), block_node);
  ps->eat(d::T_DOT);
  return program_node;
}

kenl
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
//EOF
/*
<program> ::= PROGRAM <ident> SEMI <top-level decl> <main> '.'

<top-level decls> ::= ( <data declaration> )*

<data declaration> ::= VAR <var-list>

<var-list> ::= <var> ( <var> )*
<var> ::= <ident> [ = <integer> ]


<main> ::= BEGIN <block> END

<block> ::= ( <statement> )*
<statement> ::= <if> | <while> | <assignment>

<assignment> ::= <ident> = <expression>

<expression> ::= <first term> ( <addop> <term> )*

<first term> ::= <first factor> <rest>

<term> ::= <factor> <rest>

<rest> ::= ( <mulop> <factor> )*

<first factor> ::= [ <addop> ] <factor>

<factor> ::= <var> | <number> | ( <expression> )

<bool-expr> ::= <bool-term> ( <orop> <bool-term> )*

<bool-term> ::= <not-factor> ( <andop> <not-factor> )*

<not-factor> ::= [ '!' ] <relation>

     <relation> ::= <expression> [ <relop> <expression> ]


<if> ::= IF <bool-expression> <block> [ ELSE <block>] ENDIF

     <while> ::= WHILE <bool-expression> <block> ENDWHILE



 */
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

