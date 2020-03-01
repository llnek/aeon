#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <stdexcept>

#include "interpreter.h"

using namespace czlab::aeon;
using namespace czlab::spi;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
const char* XARG="BEGIN\n\
\n\
BEGIN\n\
        number := 2;\n\
  a := number;\n\
         b := 10 * a + 10 * number / 4;\n\
         c := a - - b\n\
     END;\n\
\n\
     x := 11;\n\
 END.\n";

const char* XXARG="\n\
PROGRAM Part10AST;\n\
VAR\n\
  number: INTEGER;\n\
   a, b : INTEGER;\n\
   y    : REAL;\n\
\n\
BEGIN {Part10AST}\n\
   number := 2;\n\
   a := number;\n\
   b := 10 * a + 10 * a DIV 4;\n\
   y := 20 / 7 + 3.14;\n\
END.  {Part10AST}\n\
";

const char* ARG = "\n\
PROGRAM Part12;\n\
VAR\n\
   a : INTEGER;\n\
\n\
PROCEDURE P1;\n\
VAR\n\
   a : REAL;\n\
   k : INTEGER;\n\
\n\
   PROCEDURE P2;\n\
   VAR\n\
      a, z : INTEGER;\n\
   BEGIN {P2}\n\
      z := 777;\n\
   END;  {P2}\n\
\n\
BEGIN {P1}\n\
\n\
END;  {P1}\n\
\n\
BEGIN {Part12}\n\
   a := 10;\n\
END.  {Part12}\n\
";

const char* zARG= "\n\
program Main;\n\
var\n\
    x, y : integer;\n\
    s : STRING;\n\
begin { Main }\n\
  s := \"he\tllo\";\n\
  y := 7;\n\
   x := (y + 3) * 3;\n\
END.  { Main }\n\
";

const char* ARG18= "\n\
program Main;\n\
var poo : integer;\n\
procedure Alpha(a : integer; b : integer);\n\
var x : integer;\n\
begin\n\
   x := (a + b ) * 2;\n\
    poo := x;\n\
end;\n\
\n\
begin { Main }\n\
   Alpha(3 + 5, 7);  { procedure call }\n\
end.  { Main }\n\
";

int main(int argc, char* argv[]) {
  try {
    //"5 - - - + - (3 + 4) - +2");//" 2 + ((5 + 4) * 3)");
    Interpreter i(ARG18);
    auto r= i.interpret();
    ::printf("result = %s\n", r.toString().c_str());
    //Analyzer z(ARG);

  } catch ( const d::SyntaxError& e) {
    ::printf("%s", e.what());
  }
  return 0;
}
