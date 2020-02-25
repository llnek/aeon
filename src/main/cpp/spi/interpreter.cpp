#include "interpreter.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::spi {
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Interpreter::Interpreter(const char* src) {
  SimplePascalParser p(src);
  eval((Ast*) p.parse());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Interpreter::Interpreter(Ast* tree) {
  eval(tree);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::eval(Ast* tree) {
  auto f= new d::Frame();
  stack.push(f);
  auto res= tree->eval(this);
  if (res.type == d::EXPR_INT)
    ::printf("result = %ld\n\n", res.value.u.n);
  else
    ::printf("result = %lf\n\n", res.value.u.r);
  auto env= stack.pop();
  auto& m= env->slots;
  ::printf("slots cont = %d\n", (int)m.size());
  for (auto it = m.begin(); it != m.end(); ++it) {
    auto x = it->second;
    if (x.type == d::EXPR_INT)
      ::printf("key = %s, value = %ld\n", it->first.c_str(), x.value.u.n);
    if (x.type == d::EXPR_REAL)
      ::printf("key = %s, value = %lf\n", it->first.c_str(), x.value.u.r);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Interpreter::setValue(const char* name, const d::ExprValue& v) {
  auto x = stack.peek();
  if (x) x->set(name, v);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Interpreter::getValue(const char* name) {
  auto x = stack.peek();
  if (x)
    return x->get(name);
  else
    return d::ExprValue();
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

