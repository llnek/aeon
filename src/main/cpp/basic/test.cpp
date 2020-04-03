/* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright © 2013-2020, Kenneth Leung. All rights reserved. */

#include <iostream>
#include "interpreter.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic {
using namespace czlab::aeon;

}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#if 1
int main(int argc, char* argv[]) {
  using namespace czlab::basic;
  try {
    BasicParser p("100 print 1,(4+3),b,c\n300 END\n200 rem poo\nRUN");
        //"100 goto sin(3+4*5^2+4^3*8)\n200 gosub 777\n300 return");
    //100 let a=2*3^4*5\nREM poo poo\r\n200 b=5\n500 DIM xx$(3,4):a=7");
    p.parse();
    std::cout << "result = " << "yo"  << "\n";
  } catch (const d::SyntaxError& e) {
    std::cout << e.what() << "\n";
  }
  return 0;
}
#endif

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF
