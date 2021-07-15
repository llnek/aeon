#include <iostream>
#include "otto.h"

namespace k = czlab::otto;
namespace a = czlab::aeon;
#if 0
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int main(int argc, char* argv[]) {
#if 0
  try {
    auto f= a::read_file("/tmp/test.clj");
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
#endif
  std::string input;

  while(1){
    try{
      std::cout << "user> ";
      getline(std::cin, input);
      if(input=="xxx" || input == "qqq"){
        std::cout << "bye!\n";
        break;
      }
      std::cout << k::repl(input);
      std::cout << "\n";
    }catch(a::Error& e){
      std::cout << e.what() << "\n";
    }
  }

  return 0;
}
#endif

