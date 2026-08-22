#pragma once

#include <exception>

class VectorIndexOutOfRange : public std::exception {
public:
  const char *what() const noexcept override {
    return "Vector index out of range.";
  }
};

class Vector2IndexOutOfRange : public VectorIndexOutOfRange {
public:
  const char *what() const noexcept override {
    return "Vector2 index out of range (must be 0 or 1).";
  }
};

class Vector3IndexOutOfRange : public VectorIndexOutOfRange {
public:
  const char *what() const noexcept override {
    return "Vector3 index out of range (must be 0, 1, or 2).";
  }
};

class DivisionByZero : public std::exception {
public:
  const char *what() const noexcept override {
    return "Division by zero.";
  }
};
