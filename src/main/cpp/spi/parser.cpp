#include "parser.h"
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::spi {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound* compound_statement(SimplePascalParser*);
Block* block(SimplePascalParser*);
Ast* expr(SimplePascalParser*);
char MSGBUF[1024];
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast(d::IToken* t) : Ast() {
  token=t;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast::Ast() : token(nullptr) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BinOp::BinOp(Ast* left, d::IToken* op, Ast* right)
  : Ast(op), lhs(left), rhs(right) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void BinOp::visit(d::IAnalyzer* a) {
  lhs->visit(a);
  rhs->visit(a);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string BinOp::name() {
  return "BinOp";
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue BinOp::eval(d::IEvaluator* e) {

  d::ExprValue lf = lhs->eval(e);
  d::ExprValue rt = rhs->eval(e);
  double x, y, z;

  if (lf.type == d::EXPR_INT)  {
    x= (double) lf.value.u.n;
  } else { x= lf.value.u.r; }

  if (rt.type == d::EXPR_INT)  {
    y= (double) rt.value.u.n;
  } else { y= rt.value.u.r; }

  switch (this->token->type()) {
    case d::T_MINUS: z = x - y; break;
    case d::T_PLUS: z = x + y; break;
    case d::T_MULT: z = x * y; break;
    case T_INT_DIV:
    case d::T_DIV: z = x / y; break;
    default: throw d::SyntaxError("Bad Binary Op");
  }

  switch (this->token->type()) {
    case T_INT_DIV:
      return d::ExprValue(d::EXPR_INT, (long) z);
    case d::T_DIV:
      return d::ExprValue(d::EXPR_REAL, z);
    default:
      return (lf.type == d::EXPR_REAL ||
              rt.type == d::EXPR_REAL)
        ? d::ExprValue(d::EXPR_REAL, z)
        : d::ExprValue(d::EXPR_INT, (long) z);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
String::String(d::IToken* t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void String::visit(d::IAnalyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string String::name() {
  return "string";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue String::eval(d::IEvaluator* e) {
  return d::ExprValue(d::EXPR_STR, token->getLiteralAsStr());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Num::Num(d::IToken* t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Num::visit(d::IAnalyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Num::name() {
  switch (token->type()) {
    case d::T_INTEGER: return "integer";
    case d::T_REAL: return "real";
    default: throw d::SyntaxError("Bad numeric type");
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Num::eval(d::IEvaluator* e) {
  switch (token->type()) {
    case d::T_INTEGER:
      return d::ExprValue(d::EXPR_INT, token->getLiteralAsInt());
    case d::T_REAL:
      return d::ExprValue(d::EXPR_REAL, token->getLiteralAsReal());
    default: throw d::SyntaxError("Bad numeric type");
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
UnaryOp::UnaryOp(d::IToken* t, Ast* expr) : Ast(t), expr(expr) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string UnaryOp::name() {
  switch (token->type()) {
    case d::T_PLUS: return "+";
    case d::T_MINUS: return "-";
    default: throw d::SyntaxError("Bad unary op.");
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void UnaryOp::visit(d::IAnalyzer* a) {
  expr->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue UnaryOp::eval(d::IEvaluator* e) {
  auto r = expr->eval(e);
  if (! (r.type == d::EXPR_REAL || r.type == d::EXPR_INT)) {
    throw d::SyntaxError("Expected numeric value for unary op.");
  }
  if (token->type() == d::T_MINUS) {
    if (r.type == d::EXPR_INT)
      return d::ExprValue(r.type, - r.value.u.n);
    if (r.type == d::EXPR_REAL)
      return d::ExprValue(r.type, - r.value.u.r);
  }
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound::Compound() : Ast() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Compound::name() {
  return "Compound";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Compound::visit(d::IAnalyzer* a) {
  for (auto& it : children) {
    it->visit(a);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Compound::eval(d::IEvaluator* e) {
  for (auto& it : children) {
    it->eval(e);
  }
  return d::ExprValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment::Assignment(Var* left, d::IToken* op, Ast* right)
  : Ast(op), lhs(left), rhs(right) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Assignment::name() {
  return ":=";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Assignment::visit(d::IAnalyzer* a) {
  auto v = lhs->token->getLiteralAsStr();
  if (! a->lookup(v)) {
    throw d::SyntaxError("Unknown symbol.");
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Assignment::eval(d::IEvaluator* e) {
  auto v = lhs->token->getLiteralAsStr();
  auto r= rhs->eval(e);
  ::printf("Assigning value to %s\n", v);
  e->setValue(v, r);
  return r;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var::Var(d::IToken* t) : Ast(t) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Var::name() {
  return token->getLiteralAsStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Var::visit(d::IAnalyzer* a) {
  auto n = token->getLiteralAsStr();
  if (! a->lookup(n)) {
    throw d::SyntaxError("Unknown var name.");
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Var::eval(d::IEvaluator* e) {
  auto n = token->getLiteralAsStr();
  return e->getValue(n);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type::Type(d::IToken* token) : Ast(token) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Type::name() {
  return token->getLiteralAsStr();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Type::visit(d::IAnalyzer* a) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Type::eval(d::IEvaluator* e) {
  return d::ExprValue();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymTable::SymTable(SymTable* enclosing_scope)
  : SymbolTable(enclosing_scope) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SymTable::SymTable() {
  insert(new BuiltinTypeSymbol("INTEGER"));
  insert(new BuiltinTypeSymbol("REAL"));
  insert(new BuiltinTypeSymbol("STRING"));
  ::printf("Added built-in types.");
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Param::Param(Var* var, Type* type)
  : Ast(), var_node(var), type_node(type) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Param::name() {
  return var_node->name();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Param::visit(d::IAnalyzer* a) {
  auto t= a->lookup(type_node->name().c_str());
  if (!t)
    throw d::SyntaxError("Unknown param type.");
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Param::eval(d::IEvaluator* e) {
  return d::ExprValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VarDecl::VarDecl(Var* var, Type* type)
  : Ast(), var_node(var), type_node(type) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string VarDecl::name() {
  return var_node->name();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void VarDecl::visit(d::IAnalyzer* a) {
  auto type_name = type_node->name();
  auto var_name = var_node->name();
  auto type_symbol = a->lookup(type_name.c_str());
  if (!type_symbol) {
    throw d::SyntaxError("Unknown type.");
  }
  if (a->lookup(var_name.c_str(), false)) {
    throw d::SyntaxError("Duplicate var.");
  } else {
    a->define(new d::VarSymbol(var_name.c_str(), type_symbol));
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue VarDecl::eval(d::IEvaluator* e) {
  return d::ExprValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block::Block(std::vector<Ast*>& decls, Compound* compound)
  : Ast(), compound_statement(compound) {
  declarations.insert(declarations.end(), decls.begin(), decls.end());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Block::name() {
  return "Block";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Block::visit(d::IAnalyzer* a) {
  for (auto& x : declarations) {
    x->visit(a);
  }
  compound_statement->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Block::eval(d::IEvaluator* e) {
  for (auto& x : declarations) {
    x->eval(e);
  }
  return compound_statement->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl::ProcedureDecl(const char* proc_name,
    std::vector<Param*>& pms, Block* block_node)
  : Ast()  {
  _name=proc_name;
  block=block_node;
  params.insert(params.end(), pms.begin(), pms.end());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureDecl::visit(d::IAnalyzer* a) {
  auto ps= new d::FunctionSymbol(_name.c_str());
  a->define(ps);
  a->pushScope();
  for (auto& p : params) {
    auto pt = a->lookup(p->type_node->name().c_str());
    auto pn = p->var_node->name();
    auto v = new d::VarSymbol(pn.c_str(), pt);
    a->define(v);
    ps->params.push_back(v);
  }
  block->visit(a);
  a->popScope();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string ProcedureDecl::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue ProcedureDecl::eval(d::IEvaluator* e) {
  return d::ExprValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall::ProcedureCall(const char* proc_name,
                             std::vector<Ast*>& p, d::IToken* token)
  : Ast(token), _name(proc_name) {
  params.insert(params.end(), p.begin(), p.end());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string ProcedureCall::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void ProcedureCall::visit(d::IAnalyzer* a) {
  for (auto& p : params) {
    p->visit(a);
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue ProcedureCall::eval(d::IEvaluator* e) {
  return d::ExprValue();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program::Program(const char* name, Block* block) : Ast() {
  _name=name;
  this->block=block;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string Program::name() {
  return _name;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void Program::visit(d::IAnalyzer* a) {
  block->visit(a);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue Program::eval(d::IEvaluator* e) {
  return block->eval(e);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Var* variable(SimplePascalParser* ps) {
  auto node = new Var(ps->lex->ctx.cur);
  ps->eat(d::T_IDENT);
  return node;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* factor(SimplePascalParser* ps) {
  auto t= ps->lex->ctx.cur;
  Ast* res=nullptr;
  switch (t->type()) {
    case d::T_PLUS:
      ps->eat(d::T_PLUS);
      res= new UnaryOp(t, factor(ps));
      break;
    case d::T_MINUS:
      ps->eat(d::T_MINUS);
      res = new UnaryOp(t, factor(ps));
      break;
    case d::T_INTEGER:
      ps->eat(d::T_INTEGER);
      res= new Num(t);
      break;
    case d::T_REAL:
      ps->eat(d::T_REAL);
      res= new Num(t);
      break;
    case d::T_STRING:
      ps->eat(d::T_STRING);
      res= new String(t);
      break;
    case d::T_LPAREN:
      ps->eat(d::T_LPAREN);
      res= expr(ps);
      ps->eat(d::T_RPAREN);
      break;
    default:
      res= variable(ps);
      break;
  }
  return res;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* term(SimplePascalParser* ps) {
  static std::set<int> ops {d::T_MULT,d::T_DIV, T_INT_DIV};
  auto res= factor(ps);
  d::IToken* op;
  while (ops.find(ps->lex->ctx.cur->type()) != ops.end()) {
    op= ps->lex->ctx.cur;
    switch (op->type()) {
      case d::T_MULT:
        ps->eat(d::T_MULT);
      break;
      case d::T_DIV:
        ps->eat(d::T_DIV);
      break;
      case T_INT_DIV:
        ps->eat(T_INT_DIV);
      break;
    }
    res = new BinOp(res, op, factor(ps));
  }
  return res;
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* expr(SimplePascalParser* ps) {
  static std::set<int> ops {d::T_PLUS, d::T_MINUS};
  Ast* res= term(ps);
  d::IToken* op;
  while (ops.find(ps->lex->ctx.cur->type()) != ops.end()) {
    op= ps->lex->ctx.cur;
    switch (op->type()) {
      case d::T_PLUS:
        ps->eat(d::T_PLUS);
      break;
      case d::T_MINUS:
        ps->eat(d::T_MINUS);
      break;
    }
    res= new BinOp(res, op, term(ps));
  }

  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Type* type_spec(SimplePascalParser* ps) {
  auto token = ps->lex->ctx.cur;
  switch (token->type()) {
    case T_STR: ps->eat(T_STR); break;
    case T_INT: ps->eat(T_INT); break;
    case T_REAL: ps->eat(T_REAL); break;
    default: throw d::SyntaxError("Unsupported type.");
  }
  return new Type(token);
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<VarDecl*> variable_declaration(SimplePascalParser* ps) {
  std::vector<Var*> vars { new Var(ps->lex->ctx.cur) };
  ps->eat(d::T_IDENT);
  while (ps->lex->ctx.cur->type() == d::T_COMMA) {
    ps->eat(d::T_COMMA);
    vars.push_back(new Var(ps->lex->ctx.cur));
    ps->eat(d::T_IDENT);
  }
  ps->eat(d::T_COLON);

  auto type = type_spec(ps);
  std::vector<VarDecl*> out;
  for (auto &x : vars) {
    out.push_back(new VarDecl(x, type));
  }

  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Assignment* assignment_statement(SimplePascalParser* ps) {
  auto left = variable(ps);
  auto t = ps->lex->ctx.cur;
  ps->eat(T_ASSIGN);
  auto right = expr(ps);
  return new Assignment(left, t, right);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
NoOp* empty(SimplePascalParser* ps) {
  return new NoOp();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureCall* proccall_statement(SimplePascalParser* ps) {
  auto token = ps->lex->ctx.cur;
  std::string proc_name = token->getLiteralAsStr();

  ps->eat(d::T_IDENT);
  ps->eat(d::T_LPAREN);

  std::vector<Ast*> pms;

  if (ps->lex->ctx.cur->type() != d::T_RPAREN) {
    pms.push_back( expr(ps));
  }

  while (ps->lex->ctx.cur->type() == d::T_COMMA) {
    ps->eat(d::T_COMMA);
    pms.push_back(expr(ps));
  }

  ps->eat(d::T_RPAREN);

  return new ProcedureCall(proc_name.c_str(), pms, token);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Ast* statement(SimplePascalParser* ps) {
  Ast* node;
  switch (ps->lex->ctx.cur->type()) {
    case T_BEGIN:
      node = compound_statement(ps);
      break;
    case d::T_IDENT:
      if (d::peek(ps->lex->ctx) == '(') {
        node = proccall_statement(ps);
      } else {
        node = assignment_statement(ps);
      }
      break;
    default:
      node = empty(ps);
      break;
  }
  return node;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Ast*> statement_list(SimplePascalParser* ps) {

  std::vector<Ast*> results {
    statement(ps)
  };

  while (ps->lex->ctx.cur->type() == d::T_SEMI) {
    ps->eat(d::T_SEMI);
    results.push_back(statement(ps));
  }

  if (ps->lex->ctx.cur->type() == d::T_IDENT) {
    ::sprintf(MSGBUF, "Unexpected identifier `%s`", ps->lex->ctx.cur->getLiteralAsStr());
    throw d::SyntaxError(MSGBUF);
  }

  return results;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Compound* compound_statement(SimplePascalParser* ps) {
  ps->eat(T_BEGIN);
  auto nodes = statement_list(ps);
  ps->eat(T_END);
  auto root= new Compound();
  for (auto& node : nodes) {
    root->children.push_back(node);
  }
  return root;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Param*> formal_parameters(SimplePascalParser* ps) {

  std::vector<d::IToken*> param_tokens {ps->lex->ctx.cur};
  std::vector<Param*> pnodes;

  ps->eat(d::T_IDENT);

  while (ps->lex->ctx.cur->type() == d::T_COMMA) {
    ps->eat(d::T_COMMA);
    param_tokens.push_back(ps->lex->ctx.cur);
    ps->eat(d::T_IDENT);
  }

  ps->eat(d::T_COLON);
  auto type_node = type_spec(ps);

  for (auto& t : param_tokens) {
    pnodes.push_back(new Param(new Var(t), type_node));
  }

  return pnodes;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Param*> formal_parameter_list(SimplePascalParser* ps) {
  std::vector<Param*> out;

  if (ps->lex->ctx.cur->type() != d::T_IDENT) {
    return out;
  }

  auto pnodes = formal_parameters(ps);
  while (ps->lex->ctx.cur->type() == d::T_SEMI) {
    ps->eat(d::T_SEMI);
    auto pms = formal_parameters(ps);
    pnodes.insert(pnodes.end(), pms.begin(), pms.end());
  }

  return pnodes;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ProcedureDecl* procedure_declaration(SimplePascalParser* ps) {

  ps->eat(T_PROCEDURE);

  auto proc_name = ps->lex->ctx.cur->getLiteralAsStr();
  std::vector<Param*> params;

  ps->eat(d::T_IDENT);

  if (ps->lex->ctx.cur->type() == d::T_LPAREN) {
    ps->eat(d::T_LPAREN);
    params = formal_parameter_list(ps);
    ps->eat(d::T_RPAREN);
  }

  ps->eat(d::T_SEMI);

  auto block_node = block(ps);
  auto proc_decl = new ProcedureDecl(proc_name, params, block_node);
  ps->eat(d::T_SEMI);

  return proc_decl;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::vector<Ast*> declarations(SimplePascalParser* ps) {
  std::vector<Ast*> ds;

  if (ps->lex->ctx.cur->type() == T_VAR) {
    ps->eat(T_VAR);
    while (ps->lex->ctx.cur->type() == d::T_IDENT) {
      auto vs = variable_declaration(ps);
      ds.insert(ds.end(),vs.begin(),vs.end());
      ps->eat(d::T_SEMI);
    }
  }

  while (ps->lex->ctx.cur->type() == T_PROCEDURE) {
    ds.push_back(procedure_declaration(ps));
  }

  return ds;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Block* block(SimplePascalParser* ps) {
  auto decls = declarations(ps);
  auto cs = compound_statement(ps);
  return new Block(decls, cs);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Program* program(SimplePascalParser* ps) {
  ps->eat(T_PROGRAM);
  auto var_node = variable(ps);
  auto prog_name = var_node->name();
  ps->eat(d::T_SEMI);
  auto block_node = block(ps);
  auto program_node = new Program(prog_name.c_str(), block_node);
  ps->eat(d::T_DOT);
  return program_node;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SimplePascalParser::~SimplePascalParser() {
  delete lex;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SimplePascalParser::SimplePascalParser(const char* src) {
  lex = new Lexer(src);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IAst* SimplePascalParser::parse() {
  return program(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::IToken* SimplePascalParser::eat(int wanted) {
  auto t= lex->ctx.cur;
  if (t->type() != wanted) {
    ::sprintf(MSGBUF, "Expecting token[%d], got[%d] instead.", wanted, t->type());
    throw d::SyntaxError(MSGBUF);
  }
  lex->ctx.cur=lex->getNextToken();
  return t;
}

/* GRAMMER
        program : PROGRAM variable SEMI block DOT

        block : declarations compound_statement

        declarations : (VAR (variable_declaration SEMI)+)? procedure_declaration*

        variable_declaration : ID (COMMA ID)* COLON type_spec

        procedure_declaration :
             PROCEDURE ID (LPAREN formal_parameter_list RPAREN)? SEMI block SEMI

        formal_params_list : formal_parameters
                           | formal_parameters SEMI formal_parameter_list

        formal_parameters : ID (COMMA ID)* COLON type_spec

        type_spec : INTEGER | REAL

        compound_statement : BEGIN statement_list END

        statement_list : statement
                       | statement SEMI statement_list

        statement : compound_statement
                  | proccall_statement
                  | assignment_statement
                  | empty

        proccall_statement : ID LPAREN (expr (COMMA expr)*)? RPAREN

        assignment_statement : variable ASSIGN expr

        empty :

        expr : term ((PLUS | MINUS) term)*

        term : factor ((MUL | INTEGER_DIV | FLOAT_DIV) factor)*

        factor : PLUS factor
               | MINUS factor
               | INTEGER_CONST
               | REAL_CONST
               | LPAREN expr RPAREN
               | variable

        variable: ID
*/
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF
