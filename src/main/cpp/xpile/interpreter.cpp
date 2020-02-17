#include "interpreter.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::aeon::interpreter {
namespace l= czlab::aeon::lexer;
namespace p= czlab::aeon::parser;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Interpreter::Interpreter(const char* src) {
  auto c= l::lexer(src, "");
  auto t= p::parser(c);
  eval(t);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Interpreter::Interpreter(p::Ast* tree) {
  eval(tree);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::eval(p::Ast* tree) {
  auto f= new p::Frame(tree->name().c_str(),0,1);
  stack.push(f);
  auto res= tree->eval(this);
  if (res.type == l::T_INT)
    ::printf("result = %ld\n\n", res.value.num);
  else
    ::printf("result = %lf\n\n", res.value.real);
  auto env= stack.pop();
  auto& m= env->slots;
  ::printf("slots cont = %d\n", (int)m.size());
  for (auto it = m.begin(); it != m.end(); ++it) {
    auto x = it->second;
    if (x.type == l::T_INT)
      ::printf("key = %s, value = %ld\n", it->first.c_str(), x.value.num);
    if (x.type == l::T_REAL)
      ::printf("key = %s, value = %lf\n", it->first.c_str(), x.value.real);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::setValue(const std::string& name, const p::DataValue& v) {
  auto x = stack.peek();
  if (x) x->set(name, v);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
p::DataValue Interpreter::getValue(const std::string& name) {
  auto x = stack.peek();
  if (x)
    return x->get(name);
  else
    return p::DataValue();
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

