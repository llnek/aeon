#include <cstring>

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
void assignOrProc() {
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
//Parse and translate a block of statements.
void block() {
  scan();
  while (Token[0] != 'e' &&
         Token[0] != 'l' && Token[0] != 'u') {
    switch (Token[0]) {
      case 'i' : doIf(); break;
      case 'w' : doWhile(); break;
      case 'r' : doRepeat(); break;
      case 'f' : doFor(); break;
      case 'R' : doRead(); break;
      case 'L' : doReadLn(); break;
      case 'W' : doWrite(); break;
      case 'N' : doWriteLn(); break;
      default:
      assignOrProc();
    }
    semi();
    scan();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Allocate storage for a variable.
void alloc() {
  next();
  if (::strcmp(Token, "x") != 0) {
    expected("Variable Name");
  }
  checkDup(Value);
  addEntry(Value, 'v');
  allocate(Value, '0');
  next();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate a procedure declaration.
void doProc() {
  next();
  if (::strcmp(Token,"x") != 0) {
    expected("Procedure Name");
  }
  checkDup(Value);
  addEntry(Value, 'p');

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
// Parse and translate main program
void doMain() {
  next();
  scan();

  if (::strcmp(Token, "x") != 0) {
    expected("Program Name");
  }

  checkDup(Value);
  //Put in symbol table to prevent identifiers with same name
  addEntry(Value, 'P');
  semi();
  next();
  topDecls();
  codeProlog();

  while (::strcmp(Token, "p")==0) {
    doProc();
  }

  matchString("BEGIN");
  prolog();
  block();
  matchString("END");
  epilog();
  closeFile(SourceFile);
  closefile(AsmFile);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Parse and translate global declarations
void topDecls() {
  scan();
  while (::strcmp(Token, "v")==0) {
    alloc();
  }
  while (::strcmp(Token, ",")==0) {
    alloc();
  }
  semi();
  scan();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

