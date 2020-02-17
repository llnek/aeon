#pragma once
#include <string>
#include <map>
#include <stdexcept>

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
struct ErrorCode : public std::runtime_error {
  ErrorCode(const std::string& s) : runtime_error(s) {}
};
struct SyntaxError : public ErrorCode {
  SyntaxError(const std::string& s) : ErrorCode(s) {}
};
struct IdNotFound : public ErrorCode {
  IdNotFound(const std::string& s) : ErrorCode(s) {}
};
struct DuplicateId : public ErrorCode {
  DuplicateId(const std::string& s) : ErrorCode(s) {}
};


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF


