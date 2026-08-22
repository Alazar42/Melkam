#pragma once

#include "array/Array.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

class String {
public:
  String();
  ~String();

  String(const char *cstr);
  String(const String &other);
  String(String &&other) noexcept;
  String(std::string s);
  String(const char c);
  String(int n);
  String(long long n);
  String(long double n);

  String &operator=(const String &other);
  String &operator=(String &&other) noexcept;

  size_t length() const;
  bool is_empty() const;
  bool contains(const String &what) const;
  bool begins_with(const String &prefix) const;
  bool ends_with(const String &suffix) const;
  char32_t unicode_at(int index) const;

  int find(const String &what, int from = 0) const;
  int rfind(const String &what, int from = -1) const;
  String substr(int from, int len = -1) const;

  String strip_edges() const;
  String replace(const String &what, const String &forwhat) const;
  String to_lower() const;
  String to_upper() const;
  String format(const Array<String> &values) const;

  Array<String> split(const String &delimiter) const;
  String join(const Array<String> &parts) const;

  int64_t to_int() const;
  double to_float() const;
  static String chr(char32_t code);
  Array<uint8_t> to_utf8_buffer() const;

  const char *c_str() const { return str.c_str(); }

  auto begin() noexcept { return str.begin(); }
  auto end() noexcept { return str.end(); }

  auto begin() const noexcept { return str.begin(); }
  auto end() const noexcept { return str.end(); }

  auto cbegin() const noexcept { return str.cbegin(); }
  auto cend() const noexcept { return str.cend(); }

  char &operator[](size_t index) { return str[index]; }
  const char &operator[](size_t index) const { return str[index]; }

  bool operator==(const String &other) const { return str == other.str; }
  bool operator!=(const String &other) const { return str != other.str; }
  bool operator<(const String &other) const { return str < other.str; }

  friend std::ostream &operator<<(std::ostream &os, const String &s) {
    os << s.str;
    return os;
  }

private:
  std::string str;
};

inline String::String() : str("") {}

inline String::~String() {}

inline String::String(const char *cstr) : str(cstr ? cstr : "") {}

inline String::String(const String &other) : str(other.str) {}

inline String::String(String &&other) noexcept : str(std::move(other.str)) {}

inline String::String(std::string s) : str(std::move(s)) {}

inline String::String(const char c) : str(1, c) {}

inline String::String(int n) : str(std::to_string(n)) {}

inline String::String(long long n) : str(std::to_string(n)) {}

inline String::String(long double n) : str(std::to_string(n)) {}

inline String &String::operator=(const String &other) {
  if (this != &other) {
    str = other.str;
  }
  return *this;
}

inline String &String::operator=(String &&other) noexcept {
  if (this != &other) {
    str = std::move(other.str);
  }
  return *this;
}

inline size_t String::length() const { return str.length(); }

inline bool String::is_empty() const { return str.empty(); }

inline bool String::contains(const String &what) const {
  return str.find(what.str) != std::string::npos;
}

inline bool String::begins_with(const String &prefix) const {
  if (prefix.str.length() > str.length()) {
    return false;
  }
  return str.compare(0, prefix.str.length(), prefix.str) == 0;
}

inline bool String::ends_with(const String &suffix) const {
  if (suffix.str.length() > str.length()) {
    return false;
  }
  return str.compare(str.length() - suffix.str.length(), suffix.str.length(),
                     suffix.str) == 0;
}

inline char32_t String::unicode_at(int index) const {
  if (index < 0) {
    return 0;
  }
  int cur_idx = 0;
  size_t i = 0;
  while (i < str.length()) {
    unsigned char c = static_cast<unsigned char>(str[i]);
    char32_t cp = 0;
    size_t bytes = 0;
    if (c < 0x80) {
      cp = c;
      bytes = 1;
    } else if ((c & 0xE0) == 0xC0) {
      cp = c & 0x1F;
      bytes = 2;
    } else if ((c & 0xF0) == 0xE0) {
      cp = c & 0x0F;
      bytes = 3;
    } else if ((c & 0xF8) == 0xF0) {
      cp = c & 0x07;
      bytes = 4;
    } else {
      cp = c;
      bytes = 1;
    }
    if (i + bytes > str.length()) {
      break;
    }
    for (size_t j = 1; j < bytes; ++j) {
      cp = (cp << 6) | (static_cast<unsigned char>(str[i + j]) & 0x3F);
    }
    if (cur_idx == index) {
      return cp;
    }
    i += bytes;
    cur_idx++;
  }
  return 0;
}

inline int String::find(const String &what, int from) const {
  if (from < 0) {
    from = 0;
  }
  if (static_cast<size_t>(from) > str.length()) {
    return -1;
  }
  size_t pos = str.find(what.str, static_cast<size_t>(from));
  return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
}

inline int String::rfind(const String &what, int from) const {
  size_t pos;
  if (from < 0 || static_cast<size_t>(from) >= str.length()) {
    pos = str.rfind(what.str);
  } else {
    pos = str.rfind(what.str, static_cast<size_t>(from));
  }
  return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
}

inline String String::substr(int from, int len) const {
  if (from < 0) {
    from = std::max(0, static_cast<int>(str.length()) + from);
  }
  if (static_cast<size_t>(from) >= str.length()) {
    return String("");
  }
  if (len < 0) {
    return String(str.substr(static_cast<size_t>(from)));
  }
  return String(
      str.substr(static_cast<size_t>(from), static_cast<size_t>(len)));
}

inline String String::strip_edges() const {
  size_t start = str.find_first_not_of(" \t\n\r\v\f");
  if (start == std::string::npos) {
    return String("");
  }
  size_t end = str.find_last_not_of(" \t\n\r\v\f");
  return String(str.substr(start, end - start + 1));
}

inline String String::replace(const String &what, const String &forwhat) const {
  if (what.str.empty()) {
    return *this;
  }
  std::string result = str;
  size_t pos = 0;
  while ((pos = result.find(what.str, pos)) != std::string::npos) {
    result.replace(pos, what.str.length(), forwhat.str);
    pos += forwhat.str.length();
  }
  return String(std::move(result));
}

inline String String::to_lower() const {
  std::string result = str;
  for (char &c : result) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return String(std::move(result));
}

inline String String::to_upper() const {
  std::string result = str;
  for (char &c : result) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return String(std::move(result));
}

inline String String::format(const Array<String> &values) const {
  String result = *this;
  for (size_t i = 0; i < values.size(); ++i) {
    std::string placeholder = "{" + std::to_string(i) + "}";
    result = result.replace(String(std::move(placeholder)),
                            values.get(static_cast<int>(i)));
  }
  return result;
}

inline Array<String> String::split(const String &delimiter) const {
  Array<String> result;
  if (delimiter.str.empty()) {
    for (char c : str) {
      result.append(String(c));
    }
    return result;
  }
  size_t start = 0;
  size_t end = str.find(delimiter.str);
  while (end != std::string::npos) {
    result.append(String(str.substr(start, end - start)));
    start = end + delimiter.str.length();
    end = str.find(delimiter.str, start);
  }
  result.append(String(str.substr(start)));
  return result;
}

inline String String::join(const Array<String> &parts) const {
  std::string result;
  for (size_t i = 0; i < parts.size(); ++i) {
    result += parts.get(static_cast<int>(i)).str;
    if (i + 1 != parts.size()) {
      result += str;
    }
  }
  return String(std::move(result));
}

inline int64_t String::to_int() const {
  try {
    return std::stoll(str);
  } catch (...) {
    return 0;
  }
}

inline double String::to_float() const {
  try {
    return std::stod(str);
  } catch (...) {
    return 0.0;
  }
}

inline String String::chr(char32_t code) {
  std::string s;
  if (code <= 0x7F) {
    s.push_back(static_cast<char>(code));
  } else if (code <= 0x7FF) {
    s.push_back(static_cast<char>(0xC0 | ((code >> 6) & 0x1F)));
    s.push_back(static_cast<char>(0x80 | (code & 0x3F)));
  } else if (code <= 0xFFFF) {
    s.push_back(static_cast<char>(0xE0 | ((code >> 12) & 0x0F)));
    s.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
    s.push_back(static_cast<char>(0x80 | (code & 0x3F)));
  } else if (code <= 0x10FFFF) {
    s.push_back(static_cast<char>(0xF0 | ((code >> 18) & 0x07)));
    s.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
    s.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
    s.push_back(static_cast<char>(0x80 | (code & 0x3F)));
  }
  return String(std::move(s));
}

inline Array<uint8_t> String::to_utf8_buffer() const {
  Array<uint8_t> buffer;
  for (unsigned char c : str) {
    buffer.append(static_cast<uint8_t>(c));
  }
  return buffer;
}