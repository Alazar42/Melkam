#pragma once

#include <exception>

class ColorIndexOutOfRange : public std::exception {
public:
  const char *what() const noexcept override {
    return "Color index out of range (must be 0-3).";
  }
};

class InvalidColorFormat : public std::exception {
public:
  const char *what() const noexcept override {
    return "Invalid color format.";
  }
};
