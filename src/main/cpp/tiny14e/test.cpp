#include "interpreter.h"

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
var \n\
    poo : integer;\n\
 flag : integer;\n\
procedure Alpha(a : integer; b : integer);\n\
var x : integer;\n\
begin\n\
   x := (a + b ) * 2;\n\
   poo := x;\n\
end;\n\
\n\
begin { Main }\n\
    flag := 0;\n\
    if (flag) \n\
      flag := flag + 1;\n\
    else \n\
      flag := 999;\n\
      flag := flag + 1;\n\
    endif; \n\
   Alpha(3 + 5, 7);  { procedure call }\n\
end.  { Main }\n\
";
namespace czlab::tiny14e {
namespace a=czlab::aeon;

int main(int argc, char* argv[]) {
  try {
    auto src= a::read_file("/Users/kenl/Desktop/pas_triangle.pas");
    Interpreter i(src.c_str());
    i.interpret();
    //::printf("result = %s\n", r.pr_str().c_str());
  } catch ( const d::SyntaxError& e) {
    ::printf("%s", e.what().c_str());
  }
  return 0;
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF
#if 1
using namespace czlab::aeon;
using namespace czlab::tiny14e;
namespace d=czlab::dsl;

int main(int argc, char* argv[]) {
  try {
    auto src= read_file("/Users/kenl/Desktop/pas_triangle.pas");
    Interpreter i(src.c_str());
    i.interpret();
    //::printf("result = %s\n", r.get()->pr_str().c_str());
  } catch ( const a::Exception& e) {
    ::printf("%s", e.what().c_str());
  }
  catch (const std::exception& e) {
    ::printf("%s", e.what());
  }
  return 0;
}
#endif

