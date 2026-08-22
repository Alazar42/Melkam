#pragma once

#include <exception>

class ArrayIndexOutOfRange : public std::exception {
public:
  const char *what() const noexcept override {
    return "Array index out of range.";
  }
};