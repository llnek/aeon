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

#include "lexer.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::tiny14e {
namespace a = czlab::aeon;
namespace d = czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<int, stdstr> TOKENS {
  {T_PROCEDURE, "PROCEDURE"},
  {T_PROGRAM, "PROGRAM"},
  {T_WRITELN, "WRITELN"},
  {T_WRITE, "WRITE"},
  {T_READLN, "READLN"},
  {T_READ, "READ"},
  {T_WHILE, "WHILE"},
  {T_ENDWHILE, "ENDWHILE"},
  {T_REPEAT, "REPEAT"},
  {T_UNTIL, "UNTIL"},
  {T_FOR, "FOR"},
  {T_ENDFOR, "ENDFOR"},
  {T_IF, "IF"},
  {T_ELSE,"ELSE"},
  {T_ENDIF, "ENDIF"},
  {T_VAR, "VAR"},
  {T_REAL, "REAL"},
  {T_INT, "INTEGER"},
  {T_STR, "STRING"},
  {T_INT_DIV, "DIV"},
  {T_BEGIN, "BEGIN"},
  {T_END, "END"}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::map<stdstr,int> KEYWORDS {
  {map__val(TOKENS,T_BEGIN), T_BEGIN},
  {map__val(TOKENS,T_END), T_END},
  {map__val(TOKENS,T_PROGRAM), T_PROGRAM},
  {map__val(TOKENS,T_PROCEDURE), T_PROCEDURE},
  {map__val(TOKENS,T_WRITELN),T_WRITELN},
  {map__val(TOKENS,T_WRITE),T_WRITE},
  {map__val(TOKENS,T_READLN),T_READLN},
  {map__val(TOKENS,T_READ),T_READ},
  {map__val(TOKENS,T_WHILE), T_WHILE},
  {map__val(TOKENS,T_ENDWHILE), T_ENDWHILE},
  {map__val(TOKENS,T_REPEAT), T_REPEAT},
  {map__val(TOKENS,T_UNTIL), T_UNTIL},
  {map__val(TOKENS,T_FOR), T_FOR},
  {map__val(TOKENS,T_ENDFOR), T_ENDFOR},
  {map__val(TOKENS,T_IF),T_IF},
  {map__val(TOKENS,T_ELSE),T_ELSE},
  {map__val(TOKENS,T_ENDIF),T_ENDIF},
  {map__val(TOKENS,T_VAR), T_VAR},
  {map__val(TOKENS,T_STR), T_STR},
  {map__val(TOKENS,T_INT), T_INT},
  {map__val(TOKENS,T_REAL), T_REAL},
  {map__val(TOKENS,T_INT_DIV), T_INT_DIV}
};

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::typeToString(int type) {
  if (auto x= TOKENS.find(type); x != TOKENS.end()) {
    return map__val(TOKENS,type);
  } else  {
    return stdstr("token-type=") + std::to_string(type);
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const char ch, d::SrcInfo info) : d::AbstractToken(type) {
  _impl.text= "";
  _impl.text += ch;
  _impl.line=info.first;
  _impl.col=info.second;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Token::Token(int type, const std::string& s, d::SrcInfo info) : d::AbstractToken(type) {
  _impl.text= s;
  _impl.line=info.first;
  _impl.col=info.second;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
double Token::getLiteralAsReal() const {
  if (type() != d::T_REAL) {
    RAISE(d::SemanticError,
          "Expecting float near %d(%d).\n", _impl.line, _impl.col);
  }
  return _impl.value.num.getFloat();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong Token::getLiteralAsInt() const {
  if (type() != d::T_INTEGER) {
    RAISE(d::SemanticError,
          "Expecting int near %d(%d).\n", _impl.line, _impl.col);
  }
  return _impl.value.num.getInt();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::getLiteralAsStr() const {
  if (type() == d::T_IDENT ||
      type() == d::T_STRING) {
    return _impl.value.cs.get()->get();
  }

  if (! s__contains(TOKENS,type())) {
    RAISE(d::SemanticError,
          "Expecting identifier near %d(%d).\n", _impl.line, _impl.col);
  }

  return TOKENS.at(type());
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const char c, d::SrcInfo info) {
  return d::DslToken(new Token(type, c, info));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& s, d::SrcInfo info) {
  return d::DslToken(new Token(type, s, info));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type, const stdstr& x,
    d::SrcInfo info, const stdstr& s) {
  auto t= new Token(type, x, info);
  auto len= s.length();
  t->impl().value.cs = std::make_shared<a::CString>(len);
  t->impl().value.cs.get()->copy(s.c_str());
  return d::DslToken(t);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type,
    const stdstr& s, d::SrcInfo info, llong n) {
  auto t= new Token(type, s, info);
  t->impl().value.num.setInt(n);
  return d::DslToken(t);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken token(int type,
    const stdstr& s, d::SrcInfo info, double d) {
  auto t= new Token(type,s, info);
  t->impl().value.num.setFloat(d);
  return d::DslToken(t);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr Token::toString() const {
  char buf[1024];
  ::sprintf(buf,
            "Token#{type = %d, text = %s}", type(), _impl.text.c_str());
  return stdstr(buf);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Lexer::Lexer(const char* src) {
  _ctx.len= ::strlen(src);
  _ctx.eof=false;
  _ctx.src=src;
  _ctx.line=0;
  _ctx.col=0;
  _ctx.pos=0;
  _ctx.cur= getNextToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool Lexer::isKeyword(const stdstr& k) const {
  return KEYWORDS.find(k) != KEYWORDS.end();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::skipComment() {
  while (!_ctx.eof) {
    d::advance(_ctx);
    if (d::peek(_ctx) == '}') {
      d::advance(_ctx);
      break;
    }
  }
  return d::DslToken();
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::number() {
  auto m= _ctx.mark();
  auto s = d::numeric(_ctx).c_str();
  return ::strchr(s, '.')
    ? token(d::T_REAL, s, m, ::atof(s))
    : token(d::T_INTEGER, s, m, (llong) ::atol(s));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::string() {
  auto m = _ctx.mark();
  auto s = d::str(_ctx);
  return token(d::T_STRING, s, m, s);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool lexer_id(Tchar ch, bool first) {
  if (first) {
    return (ch=='_' || ::isalpha(ch));
  } else {
    return (ch=='_' || ::isalpha(ch) || isdigit(ch));
  }
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::id() {
  auto s= d::identifier(_ctx, &lexer_id);
  auto S= a::to_upper(s);
  auto m= _ctx.mark();
  return !isKeyword(S)
    ? token(d::T_IDENT, s, m, s)
    : d::DslToken(new Token(KEYWORDS.at(S), S, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslToken Lexer::getNextToken() {
  Tchar ch;
  while (!_ctx.eof) {
    ch= d::peek(_ctx);
    if (::isspace(ch)) {
      d::skipWhitespace(_ctx);
    }
    else
    if (::isdigit(ch)) {
      return number();
    }
    else
    if (ch == '"') {
      return string();
    }
    else
    if (ch == '*') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_MULT, ch, m);
    }
    else
    if (ch == '/') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_DIV, ch, m);
    }
    else
    if (ch == '+') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_PLUS, ch, m);
    }
    else
    if (ch == '-') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_MINUS, ch, m);
    }
    else
    if (ch == '(') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_LPAREN, ch, m);
    }
    else
    if (ch == ')') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_RPAREN, ch, m);
    }
    else
    if (ch == '_' || ::isalpha(ch)) {
      return id();
    }
    else
    if (ch== ':' && '=' == d::peekNext(_ctx)) {
      auto m= _ctx.mark();
      d::advance(_ctx);
      d::advance(_ctx);
      return token(T_ASSIGN, ":=", m);
    }
    else
    if (ch== '=' && '=' == d::peekNext(_ctx)) {
      auto m= _ctx.mark();
      d::advance(_ctx);
      d::advance(_ctx);
      return token(T_EQUALS, "==", m);
    }
    else
    if (ch == '{') {
      d::advance(_ctx);
      skipComment();
    }
    else
    if (ch == '<') {
      auto m= _ctx.mark();
      if (d::peekNext(_ctx) == '=') {
        d::advance(_ctx);
        d::advance(_ctx);
        return token(T_LTEQ, "<=", m);
      }
      else if (d::peekNext(_ctx) == '>') {
        d::advance(_ctx);
        d::advance(_ctx);
        return token(T_NOTEQ, "<>", m);
      }
      else {
        d::advance(_ctx);
        return token(d::T_LT, ch, m);
      }
    }
    else
    if (ch == '>') {
      auto m= _ctx.mark();
      if (d::peekNext(_ctx) == '=') {
        d::advance(_ctx);
        d::advance(_ctx);
        return token(T_GTEQ, ">=", m);
      } else {
        d::advance(_ctx);
        return token(d::T_GT, ch, m);
      }
    }
    else
    if (ch == '|') {
      if (d::peekNext(_ctx) == '|') {
        auto m= _ctx.mark();
        d::advance(_ctx);
        d::advance(_ctx);
        return token(T_OR, "||", m);
      }
    }
    else
    if (ch == '&') {
      if (d::peekNext(_ctx) == '&') {
        auto m= _ctx.mark();
        d::advance(_ctx);
        d::advance(_ctx);
        return token(T_AND, "&&", m);
      }
    }
    else
    if (ch == ';') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_SEMI, ch, m);
    }
    else
    if (ch == '!') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(T_NOT, ch, m);
    }
    else
    if (ch == '~') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(T_XOR, ch, m);
    }
    else
    if (ch == ':') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_COLON, ch, m);
    }
    else
    if (ch == ',') {
      auto m= _ctx.mark();
      d::advance(_ctx);
      return token(d::T_COMMA, ch, m);
    }
    else
    if (ch == '.') {
      auto m=_ctx.mark();
      d::advance(_ctx);
      return token(d::T_DOT, ch, m);
    }
    else {
      RAISE(d::SyntaxError,
            "Unexpected char %c near line %d(%d).\n", ch, _ctx.line, _ctx.col);
    }
  }

  return token(d::T_EOF, "<EOF>", _ctx.mark());
}








//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

