#include <iostream>
#include "kirby.h"

namespace k = czlab::kirby;
namespace a = czlab::aeon;
#if 0
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int main(int argc, char* argv[]) {

  try {
    auto f= a::read_file("/tmp/poo.clj");
    auto s = k::repl(f);
    //"`(+ 1 2)");
    //"~eee");
    //"@abc");
    //"'abc");
        //"^{:a 1} bbb");
    //"~@(+ 1 (* 2 3 (/ 4 5)))");//"[1 2 [3  4 [ 5 6]");//"{:a [1 2 3], :b {:z 5} }");//"(* 2 3 \"a\nb\tc\")");
    std::cout << s << "\n";
  } catch (a::Exception& e) {
    std::cout << e.what() << "\n";
  }
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
#endif

