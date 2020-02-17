#include "ubasic.h"

static const char program[] =
"10 gosub 100\n\
20 for i = 1 to 10\n\
30 print i\n\
40 next i\n\
50 print \"end\"\n\
60 end\n\
100 print \"subroutine\"\n\
110 return\n";

/*
x == number statement



*/
/*---------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {

  ubasic_init(program);

  do {
    ubasic_run();
  } while (!ubasic_finished());

  return 0;
}
/*---------------------------------------------------------------------------*/
