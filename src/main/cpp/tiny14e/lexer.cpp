#include <cstring>

int Lookup;

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Table lookup
int lookup(TabPtr T, const char* s, int n) {
  auto i=n;
  while (i > 0) {
    if (0 == ::strcmp(s, T[i]))
    break;
    --i;
  }
  return i;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Locate a symbol in table
//Returns the index of the entry.  Zero if not present.
int locate(Symbol* s) {
  return lookup(@ST, s, NEntry);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Look for symbol in table
bool inTable(Symbol* s) {
  return lookup(@ST, s, NEntry) != 0;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Check to see if an identifier is in the symbol table
//Report an error if it's not.
void checkTable(Symbol* s) {
  if (!inTable(s)) undefined(s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Check the symbol table for a duplicate identifier
//Report an error if identifier is already in table.
void checkDup(Symbol* s) {
  if (inTable(s)) duplicate(s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Add a new entry to symbol table
void addEntry(Symbol* s, TChar ch) {
  checkDup(s);
  if (NEntry == MaxEntry)
    abort("Symbol Table Full");
  ++NEntry;
  ST[NEntry] = s;
  SType[NEntry] = ch;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Find the parameter number
int paramNumber(const char* s) {
  return lookup(@Params, s, NumParams);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//See if an identifier is a parameter
bool isParam(const char* s) {
  return paramNumber(s) != 0;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Add a new parameter to table
void addParam(const char* s) {
  if (isParam(s)) duplicate(s);
  ++NumParams;
  Params[NumParams] = s;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Get type of symbol
TChar typeOf(const char* n) {
  return isParam(n) ? 'F' : SType[locate(s)];
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize an alpha character
bool isAlpha(TChar c) {
  auto x= ::toupper(c);
  return x >= 'A' && x <= 'Z';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize a decimal digit
bool isDigit(TChar c) {
  return ::isdigit(c);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize an alphanumeric character
bool isAlNum(TChar c) {
  return isAlpha(c) || isDigit(c);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize an addop
bool isAddop(TChar c) {
  return c == '+' || c ==  '-';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize a mulop
bool isMulop(TChar c) {
  return  c == '*' || c ==  '/';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize a Boolean orop
bool isOrop(TChar c) {
  return  c == '|' || c == '~';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize a relop
bool isRelop(TChar c) {
  return c ==  '=' || c == '#' || c == '<' || c == '>';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Recognize white space
boolean isWhite(TChar c) {
  return ::isspace(c);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Skip a comment field
void skipComment() {
  while (Look != '}') {
    getChar();
    if (Look == '{')
      skipComment();
  }
  getChar();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Skip over leading white space
void skipWhite() {
  while (isWhite(Look)) {
    if (Look == '{')
      skipComment();
    else
      getChar();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Match a semicolon
void semi() {
  if (Token == ';') next();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Get an identifier
void getName() {
  if (!isAlpha(Look))
    expected("Identifier");

  auto i=0;
  ::strcpy(Token, "x");
  do {
    Value[i++] =  upCase(Look);
    getChar();
  } while isAlNum(Look);
  Value[i]='\0';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Get a number
void getNum() {
  if (!isDigit(Look))
    expected("Number");

  ::strcpy(Token, "#");
  auto i=0;
  do {
    Value[i++]= Look;
    getChar();
  } while (isDigit(Look));
  Value[i]= '\0';
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Get an operator
void getOp() {
  Token[0] = Look;
  Token[1]= '\0';
  Value[0] = Look;
  Value[1]='\0';
  getChar();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Get the next input token
void next() {
  skipWhite();
  if (isAlpha(Look)) {
    getName();
  }
  else if (isDigit(Look)) {
    getNum();
  }
  else {
    getOp();
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Scan the current identifier for keywords
void scan() {
  if (::strcmp(Token, "x")==0)
    ::strcpy(Token, KWcode[lookup(addr(KWlist), Value, NKW) + 1]);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Match a specific input string
void matchString(const char* x) {
  if (::strcmp(Value,x) != 0) {
    ::sprintf(MSGBUF, "`%s`", x);
    expected(MSGBUF);
  }
  next();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//Initialize parameter table to null
void clearParams() {
  for (auto i=0; i < MaxParams; ++i) {
    Params[i] = nullptr;
  }
  NumParams = 0;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

