#include <iostream>
#include "mal.h"

using namespace czlab::mal;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int main(int argc, char* argv[]) {

  std::string input;

  while (1) {
    std::cout << "user>\n";
    std::cin >> input;
    repl(input);
  }


  return 0;
}
