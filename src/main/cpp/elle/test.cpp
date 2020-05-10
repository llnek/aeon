#include <iostream>
#include "elle.h"

namespace k = czlab::elle;
namespace a = czlab::aeon;
#if 1
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int main(int argc, char* argv[]) {

  try {
    auto f= a::read_file("/tmp/poo.scm");
    auto s = k::repl(f);
    //"`(+ 1 2)");
    //"~eee");
    //"@abc");
    //"'abc");
        //"^{:a 1} bbb");
    //"~@(+ 1 (* 2 3 (/ 4 5)))");//"[1 2 [3  4 [ 5 6]");//"{:a [1 2 3], :b {:z 5} }");//"(* 2 3 \"a\nb\tc\")");
    std::cout << s << "\n";
  } catch (a::Error& e) {
    std::cout << e.what() << "\n";
  } catch (...) {
    std::cout << "Core dumped!\n";
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

