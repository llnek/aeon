#include <iostream>
#include "kirby.h"

namespace k = czlab::kirby;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int main(int argc, char* argv[]) {

  auto s = k::repl("(* 2 3)");

  std::cout << s << "\n";
#if 0
  std::string input;

  while (1) {
    std::cout << "user>\n";
    std::cin >> input;
    k::repl(input);
  }
#endif
  return 0;
}
