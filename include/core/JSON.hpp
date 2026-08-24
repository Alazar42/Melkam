#pragma once

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

// Forward declaration
class JSONValue;

using JSONObject = std::map<std::string, JSONValue>;
using JSONArray = std::vector<JSONValue>;

class JSONValue {
public:
  using VariantType = std::variant<std::nullptr_t, bool, double, int64_t, std::string, JSONArray, JSONObject>;

  JSONValue() : m_val(nullptr) {}
  JSONValue(std::nullptr_t) : m_val(nullptr) {}
  JSONValue(bool b) : m_val(b) {}
  JSONValue(double d) : m_val(d) {}
  JSONValue(int i) : m_val(static_cast<int64_t>(i)) {}
  JSONValue(int64_t i) : m_val(i) {}
  JSONValue(const char *s) : m_val(std::string(s)) {}
  JSONValue(std::string s) : m_val(std::move(s)) {}
  JSONValue(JSONArray arr) : m_val(std::move(arr)) {}
  JSONValue(JSONObject obj) : m_val(std::move(obj)) {}

  bool isNull() const { return std::holds_alternative<std::nullptr_t>(m_val); }
  bool isBool() const { return std::holds_alternative<bool>(m_val); }
  bool isNumber() const { return std::holds_alternative<double>(m_val) || std::holds_alternative<int64_t>(m_val); }
  bool isString() const { return std::holds_alternative<std::string>(m_val); }
  bool isArray() const { return std::holds_alternative<JSONArray>(m_val); }
  bool isObject() const { return std::holds_alternative<JSONObject>(m_val); }

  bool asBool(bool def = false) const {
    if (isBool()) return std::get<bool>(m_val);
    return def;
  }

  double asFloat(double def = 0.0) const {
    if (std::holds_alternative<double>(m_val)) return std::get<double>(m_val);
    if (std::holds_alternative<int64_t>(m_val)) return static_cast<double>(std::get<int64_t>(m_val));
    return def;
  }

  int64_t asInt(int64_t def = 0) const {
    if (std::holds_alternative<int64_t>(m_val)) return std::get<int64_t>(m_val);
    if (std::holds_alternative<double>(m_val)) return static_cast<int64_t>(std::get<double>(m_val));
    return def;
  }

  std::string asString(const std::string &def = "") const {
    if (isString()) return std::get<std::string>(m_val);
    return def;
  }

  const JSONArray &asArray() const {
    static JSONArray emptyArr;
    if (isArray()) return std::get<JSONArray>(m_val);
    return emptyArr;
  }

  JSONArray &asArray() {
    if (!isArray()) m_val = JSONArray{};
    return std::get<JSONArray>(m_val);
  }

  const JSONObject &asObject() const {
    static JSONObject emptyObj;
    if (isObject()) return std::get<JSONObject>(m_val);
    return emptyObj;
  }

  JSONObject &asObject() {
    if (!isObject()) m_val = JSONObject{};
    return std::get<JSONObject>(m_val);
  }

  JSONValue &operator[](const std::string &key) {
    return asObject()[key];
  }

  const JSONValue &operator[](const std::string &key) const {
    const auto &obj = asObject();
    auto it = obj.find(key);
    if (it != obj.end()) return it->second;
    static JSONValue nullVal;
    return nullVal;
  }

  JSONValue &operator[](size_t index) {
    auto &arr = asArray();
    if (index >= arr.size()) arr.resize(index + 1);
    return arr[index];
  }

  const JSONValue &operator[](size_t index) const {
    const auto &arr = asArray();
    if (index < arr.size()) return arr[index];
    static JSONValue nullVal;
    return nullVal;
  }

  bool hasKey(const std::string &key) const {
    if (!isObject()) return false;
    return std::get<JSONObject>(m_val).count(key) > 0;
  }

  std::string stringify(int indent = 0, int currentIndent = 0) const {
    std::string indStr(currentIndent, ' ');
    std::string nextIndStr(currentIndent + indent, ' ');

    if (isNull()) return "null";
    if (isBool()) return asBool() ? "true" : "false";
    if (std::holds_alternative<int64_t>(m_val)) return std::to_string(asInt());
    if (std::holds_alternative<double>(m_val)) {
      std::stringstream ss;
      ss << asFloat();
      return ss.str();
    }
    if (isString()) {
      std::stringstream ss;
      ss << std::quoted(asString());
      return ss.str();
    }
    if (isArray()) {
      const auto &arr = asArray();
      if (arr.empty()) return "[]";
      std::stringstream ss;
      ss << "[";
      if (indent > 0) ss << "\n";
      for (size_t i = 0; i < arr.size(); ++i) {
        if (indent > 0) ss << nextIndStr;
        ss << arr[i].stringify(indent, currentIndent + indent);
        if (i + 1 < arr.size()) ss << ",";
        if (indent > 0) ss << "\n";
      }
      if (indent > 0) ss << indStr;
      ss << "]";
      return ss.str();
    }
    if (isObject()) {
      const auto &obj = asObject();
      if (obj.empty()) return "{}";
      std::stringstream ss;
      ss << "{";
      if (indent > 0) ss << "\n";
      size_t i = 0;
      for (const auto &[k, v] : obj) {
        if (indent > 0) ss << nextIndStr;
        ss << std::quoted(k) << ": " << v.stringify(indent, currentIndent + indent);
        if (++i < obj.size()) ss << ",";
        if (indent > 0) ss << "\n";
      }
      if (indent > 0) ss << indStr;
      ss << "}";
      return ss.str();
    }
    return "null";
  }

private:
  VariantType m_val;
};

// Godot-style JSON helper utility
class JSON {
public:
  static std::string stringify(const JSONValue &val, int indent = 0) {
    return val.stringify(indent);
  }

  static JSONValue parse(const std::string &text) {
    size_t pos = 0;
    skipWhitespace(text, pos);
    return parseValue(text, pos);
  }

private:
  static void skipWhitespace(const std::string &s, size_t &pos) {
    while (pos < s.length() && std::isspace(static_cast<unsigned char>(s[pos]))) {
      pos++;
    }
  }

  static JSONValue parseValue(const std::string &s, size_t &pos) {
    skipWhitespace(s, pos);
    if (pos >= s.length()) return nullptr;

    char c = s[pos];
    if (c == 'n') { pos += 4; return nullptr; }
    if (c == 't') { pos += 4; return true; }
    if (c == 'f') { pos += 5; return false; }
    if (c == '"') return parseString(s, pos);
    if (c == '[') return parseArray(s, pos);
    if (c == '{') return parseObject(s, pos);
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber(s, pos);

    return nullptr;
  }

  static std::string parseString(const std::string &s, size_t &pos) {
    pos++; // skip open quote
    std::string res;
    while (pos < s.length()) {
      char c = s[pos++];
      if (c == '"') return res;
      if (c == '\\' && pos < s.length()) {
        char next = s[pos++];
        if (next == 'n') res += '\n';
        else if (next == 't') res += '\t';
        else if (next == 'r') res += '\r';
        else if (next == '"') res += '"';
        else if (next == '\\') res += '\\';
        else res += next;
      } else {
        res += c;
      }
    }
    return res;
  }

  static JSONValue parseNumber(const std::string &s, size_t &pos) {
    size_t start = pos;
    bool isFloat = false;
    if (s[pos] == '-') pos++;
    while (pos < s.length() && (std::isdigit(static_cast<unsigned char>(s[pos])) || s[pos] == '.' || s[pos] == 'e' || s[pos] == 'E' || s[pos] == '+')) {
      if (s[pos] == '.' || s[pos] == 'e' || s[pos] == 'E') isFloat = true;
      pos++;
    }
    std::string numStr = s.substr(start, pos - start);
    try {
      if (isFloat) return std::stod(numStr);
      return static_cast<int64_t>(std::stoll(numStr));
    } catch (...) {
      return 0;
    }
  }

  static JSONArray parseArray(const std::string &s, size_t &pos) {
    pos++; // skip '['
    JSONArray arr;
    skipWhitespace(s, pos);
    if (pos < s.length() && s[pos] == ']') {
      pos++;
      return arr;
    }

    while (pos < s.length()) {
      arr.push_back(parseValue(s, pos));
      skipWhitespace(s, pos);
      if (pos < s.length() && s[pos] == ',') {
        pos++;
      } else if (pos < s.length() && s[pos] == ']') {
        pos++;
        break;
      }
    }
    return arr;
  }

  static JSONObject parseObject(const std::string &s, size_t &pos) {
    pos++; // skip '{'
    JSONObject obj;
    skipWhitespace(s, pos);
    if (pos < s.length() && s[pos] == '}') {
      pos++;
      return obj;
    }

    while (pos < s.length()) {
      skipWhitespace(s, pos);
      if (pos >= s.length() || s[pos] != '"') break;
      std::string key = parseString(s, pos);
      skipWhitespace(s, pos);
      if (pos < s.length() && s[pos] == ':') pos++;
      obj[key] = parseValue(s, pos);

      skipWhitespace(s, pos);
      if (pos < s.length() && s[pos] == ',') {
        pos++;
      } else if (pos < s.length() && s[pos] == '}') {
        pos++;
        break;
      }
    }
    return obj;
  }
};
