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
 * Copyright © 2013-2021, Kenneth Leung. All rights reserved. */

#include <iostream>
#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::basic{
using namespace czlab::aeon;

}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#if 0
int main(int argc, char* argv[]) {
  using namespace czlab::basic;
  namespace a=czlab::aeon;
  try {
    auto f= a::read_file("/tmp/test.bas");
    Basic p(f.c_str());
    p.interpret();
    std::cout << "done." << "\n";
  } catch (const a::Error& e) {
    std::cout << e.what() << "\n";
  } catch (...) {
    std::exception_ptr p = std::current_exception();
    std::cout << "Error!!!" << "\n";
  }
  return 0;
}
#endif

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

