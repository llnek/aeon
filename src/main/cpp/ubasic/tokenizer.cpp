#define DEBUG 0

#if DEBUG
#define DEBUG_PRINTF(...)  printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

#include "tokenizer.h"
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <string>

static char const *ptr, *nextptr;

#define MAX_NUMLEN 5

struct Token {
  std::string text;
  int value;
};

static int current_token = TOKEN_ERROR;

static Token keywords[] = {
  {"let", TOKEN_LET},
  {"print", TOKEN_PRINT},
  {"if", TOKEN_IF},
  {"then", TOKEN_THEN},
  {"else", TOKEN_ELSE},
  {"for", TOKEN_FOR},
  {"to", TOKEN_TO},
  {"next", TOKEN_NEXT},
  {"goto", TOKEN_GOTO},
  {"gosub", TOKEN_GOSUB},
  {"return", TOKEN_RETURN},
  {"call", TOKEN_CALL},
  {"end", TOKEN_END},
  {"", TOKEN_ERROR}
};

/*---------------------------------------------------------------------------*/
static int singleChar() {
  switch (*ptr) {
    case '\n': return TOKEN_CR;
    case ',': return TOKEN_COMMA;
    case ';': return TOKEN_SEMICOLON;
    case '+': return TOKEN_PLUS;
    case '-': return TOKEN_MINUS;
    case '&': return TOKEN_AND;
    case '|': return TOKEN_OR;
    case '*': return TOKEN_ASTR;
    case '/': return TOKEN_SLASH;
    case '%': return TOKEN_MOD;
    case '(': return TOKEN_LEFTPAREN;
    case ')': return TOKEN_RIGHTPAREN;
    case '<': return TOKEN_LT;
    case '>': return TOKEN_GT;
    case '=': return TOKEN_EQ;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static int get_next_token() {
  TOKEN const *kt;
  int i;

  DEBUG_PRINTF("get_next_token(): '%s'\n", ptr);

  if (*ptr == '\0') { // 0
    return TOKEN_ENDOFINPUT;
  }

  if (::isdigit(*ptr)) {

    for (auto i = 0; i < MAX_NUMLEN; ++i) {
      if (!::isdigit(ptr[i])) {
        if (i > 0) {
          nextptr = ptr + i;
          return TOKEN_NUMBER;
        } else {
          DEBUG_PRINTF("get_next_token: error due to too short number\n");
          return TOKEN_ERROR;
        }
      }
      if (!::isdigit(ptr[i])) {
        DEBUG_PRINTF("get_next_token: error due to malformed number\n");
        return TOKEN_ERROR;
      }
    }
    DEBUG_PRINTF("get_next_token: error due to too long number\n");
    return TOKEN_ERROR;

  } else if (singleChar()) {

    nextptr = ptr + 1;
    return singleChar();

  } else if( *ptr == '"') {

    nextptr = ptr;
    do {
      ++nextptr;
    } while (*nextptr != '"');
    ++nextptr;
    return TOKEN_STRING;

  } else {
    for (kt = keywords; kt->keyword; ++kt) {
      if (::strncmp(ptr, kt->keyword, ::strlen(kt->keyword)) == 0) {
        nextptr = ptr + ::strlen(kt->keyword);
        return kt->token;
      }
    }
  }

  if (*ptr >= 'a' && *ptr <= 'z') {
    nextptr = ptr + 1;
    return TOKEN_VARIABLE;
  }

  return TOKEN_ERROR;
}
/*---------------------------------------------------------------------------*/
void tokenizer_init(const char *program) {
  ptr = program;
  current_token = get_next_token();
}
/*---------------------------------------------------------------------------*/
int tokenizer_token() {
  return current_token;
}
/*---------------------------------------------------------------------------*/
void tokenizer_next() {

  if (tokenizer_finished()) {
    return;
  }

  DEBUG_PRINTF("tokenizer_next: %p\n", nextptr);
  ptr = nextptr;
  while (*ptr == ' ' || *ptr == '\t') {
    ++ptr;
  }
  current_token = get_next_token();
  DEBUG_PRINTF("tokenizer_next: '%s' %d\n", ptr, current_token);
}
/*---------------------------------------------------------------------------*/
int tokenizer_num() {
  return ::atoi(ptr);
}
/*---------------------------------------------------------------------------*/
void tokenizer_string(char *dest, int len) {

  if (tokenizer_token() != TOKEN_STRING) {
    return;
  }

  const char *string_end = ::strchr(ptr + 1, '"');
  int string_len;

  if (string_end == nullptr) {
    return;
  }

  string_len = string_end - ptr - 1;
  if (len < string_len) {
    string_len = len;
  }

  ::memcpy(dest, ptr + 1, string_len);
  dest[string_len] = '\0';
}
/*---------------------------------------------------------------------------*/
void tokenizer_error_print() {
  DEBUG_PRINTF("tokenizer_error_print: '%s'\n", ptr);
}
/*---------------------------------------------------------------------------*/
int tokenizer_finished() {
  return *ptr == '\0' || current_token == TOKEN_ENDOFINPUT;
}
/*---------------------------------------------------------------------------*/
int tokenizer_variable_num() {
  return *ptr - 'a';
}
/*---------------------------------------------------------------------------*/
Token* toke(const char* file, int line, int col, const char* s) {
  return new Token(file, line, col, s);
}
////////////////////////////////////////////////////////////////////////////////
//fn: [lexer] in file: reader.ky, line: 103
//Lexical analyzer
void lexer(const char* source, const char* fname) {
  auto len = ::strlen(source);
  auto comment_QMRK = false;
  auto esc_QMRK = false;
  auto str_QMRK = false;
  auto token = "";
  auto jsEsc = 0;
  auto line = 1;
  char ch = '\0';
  char nx = '\0';
  auto col = 0;
  auto pos = 0;
  auto tree = std::vector();
  auto tcol = col;
  auto tline = line;

  while (pos < len) {
    ch = source[pos];
    ++col;
    ++pos;
    nx = source[pos];

    if (ch == '\n') {
      col = 0;
      ++line;
      if (comment_QMRK) {
        comment_QMRK = false;
      }
    }

    if (comment_QMRK) {
      continue;
    }

    if (esc_QMRK) {
      token.push_back(ch);
      esc_QMRK = false;
      continue;
    }

    if (ch == '\"') {
      if (!str_QMRK) {
        tline = line;
        tcol = col;
        str_QMRK = true;
        token.push_back(ch);
      } else {
        str_QMRK = false;
        token.push_back(ch);
        token = toke(fname,tline, tcol, token, true);
      }
      continue;
    }

    if (str_QMRK) {
      if (ch == '\n') {
        token.push_back('\\');
        token.push_back('n');
      } else {
        if (ch == '\\') {
          esc_QMRK = true;
        }
        token.push_back(ch);
      }
      continue;
    }

    switch (ch) {
      case '[': token=toke(); break;
      case ']': token=toke(); break;
      case '(': token=toke(); break;
      case ')': token=toke(); break;
      case ';': token=toke(); break;
      case ',': token=toke(); break;
      case '~': token=toke(); break;
      default:
        if (isspace(ch)) {
          token = toke((ch == '\n') ? (tline - 1) : tline, tcol, token);
        } else {
          if ( 0 === kirbystdlibref.count(token)) {
            tcol = col;
            tline = line;
          }
          token.push_back(ch);
        }
      break;
    }
  }

  let tmp = (new Map([["source", fname], ["line", tline], ["column", col]]));
  if (esc_QMRK) {
    throwE(tmp, "incomplete escape");
  }
  if (str_QMRK) {
    throwE(tmp, "unterminated string");
  }
  return tree;
}
