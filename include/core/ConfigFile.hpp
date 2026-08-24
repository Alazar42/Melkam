#pragma once

#include "core/Memory.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

// Godot-style ConfigFile and Game State Serialization (.cfg / .ini / state persistence).
class ConfigFile : public RefCounted {
public:
  using Value = std::variant<std::string, int64_t, double, bool, Vector2, Color>;

  ConfigFile() = default;

  // Sets a value in the given section and key
  void set_value(const std::string &section, const std::string &key, const Value &value) {
    m_data[section][key] = value;
  }

  void set_value(const std::string &section, const std::string &key, const char *value) {
    m_data[section][key] = std::string(value);
  }

  void setValue(const std::string &section, const std::string &key, const Value &value) {
    set_value(section, key, value);
  }

  void setValue(const std::string &section, const std::string &key, const char *value) {
    set_value(section, key, value);
  }


  // Gets a value with fallback default
  Value get_value(const std::string &section, const std::string &key, const Value &defaultVal = std::string("")) const {
    auto sIt = m_data.find(section);
    if (sIt != m_data.end()) {
      auto kIt = sIt->second.find(key);
      if (kIt != sIt->second.end()) {
        return kIt->second;
      }
    }
    return defaultVal;
  }

  Value getValue(const std::string &section, const std::string &key, const Value &defaultVal = std::string("")) const {
    return get_value(section, key, defaultVal);
  }

  // Type-specific helper getters
  std::string getString(const std::string &section, const std::string &key, const std::string &defaultVal = "") const {
    Value v = get_value(section, key, Value(defaultVal));
    if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
    return defaultVal;
  }


  int64_t getInt(const std::string &section, const std::string &key, int64_t defaultVal = 0) const {
    Value v = get_value(section, key, defaultVal);
    if (std::holds_alternative<int64_t>(v)) return std::get<int64_t>(v);
    if (std::holds_alternative<double>(v)) return static_cast<int64_t>(std::get<double>(v));
    return defaultVal;
  }

  double getFloat(const std::string &section, const std::string &key, double defaultVal = 0.0) const {
    Value v = get_value(section, key, defaultVal);
    if (std::holds_alternative<double>(v)) return std::get<double>(v);
    if (std::holds_alternative<int64_t>(v)) return static_cast<double>(std::get<int64_t>(v));
    return defaultVal;
  }

  bool getBool(const std::string &section, const std::string &key, bool defaultVal = false) const {
    Value v = get_value(section, key, defaultVal);
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v);
    return defaultVal;
  }

  Vector2 getVector2(const std::string &section, const std::string &key, const Vector2 &defaultVal = Vector2(0.0f, 0.0f)) const {
    Value v = get_value(section, key, defaultVal);
    if (std::holds_alternative<Vector2>(v)) return std::get<Vector2>(v);
    return defaultVal;
  }

  Color getColor(const std::string &section, const std::string &key, const Color &defaultVal = Color::WHITE) const {
    Value v = get_value(section, key, defaultVal);
    if (std::holds_alternative<Color>(v)) return std::get<Color>(v);
    return defaultVal;
  }

  bool has_section(const std::string &section) const {
    return m_data.find(section) != m_data.end();
  }

  bool has_section_key(const std::string &section, const std::string &key) const {
    auto sIt = m_data.find(section);
    if (sIt == m_data.end()) return false;
    return sIt->second.find(key) != sIt->second.end();
  }

  std::vector<std::string> get_sections() const {
    std::vector<std::string> res;
    for (const auto &[sec, _] : m_data) {
      res.push_back(sec);
    }
    return res;
  }

  std::vector<std::string> get_section_keys(const std::string &section) const {
    std::vector<std::string> res;
    auto sIt = m_data.find(section);
    if (sIt != m_data.end()) {
      for (const auto &[k, _] : sIt->second) {
        res.push_back(k);
      }
    }
    return res;
  }

  void erase_section(const std::string &section) {
    m_data.erase(section);
  }

  void erase_section_key(const std::string &section, const std::string &key) {
    auto sIt = m_data.find(section);
    if (sIt != m_data.end()) {
      sIt->second.erase(key);
    }
  }

  void clear() {
    m_data.clear();
  }

  // Encodes current data to Godot-compatible text format
  std::string encode_to_text() const {
    std::stringstream ss;
    for (const auto &[sec, keys] : m_data) {
      if (!sec.empty()) {
        ss << "[" << sec << "]\n\n";
      }
      for (const auto &[k, val] : keys) {
        ss << k << "=" << serializeValue(val) << "\n";
      }
      ss << "\n";
    }
    return ss.str();
  }

  // Parses Godot-compatible text
  bool parse(const std::string &text) {
    std::stringstream ss(text);
    std::string line;
    std::string currentSection = "";

    while (std::getline(ss, line)) {
      line = trim(line);
      if (line.empty() || line[0] == ';' || line[0] == '#') continue;

      if (line.front() == '[' && line.back() == ']') {
        currentSection = line.substr(1, line.length() - 2);
        continue;
      }

      size_t eqPos = line.find('=');
      if (eqPos != std::string::npos) {
        std::string key = trim(line.substr(0, eqPos));
        std::string valStr = trim(line.substr(eqPos + 1));
        m_data[currentSection][key] = deserializeValue(valStr);
      }
    }
    return true;
  }

  // Saves to file
  bool save(const std::string &path) const {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << encode_to_text();
    return true;
  }

  // Loads from file
  bool load(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    std::stringstream ss;
    ss << file.rdbuf();
    return parse(ss.str());
  }

private:
  static std::string trim(const std::string &s) {
    auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) { return std::isspace(c); });
    auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) { return std::isspace(c); }).base();
    return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
  }

  static std::string serializeValue(const Value &val) {
    return std::visit([](const auto &v) -> std::string {
      using T = std::decay_t<decltype(v)>;
      if constexpr (std::is_same_v<T, std::string>) {
        return "\"" + v + "\"";
      } else if constexpr (std::is_same_v<T, int64_t>) {
        return std::to_string(v);
      } else if constexpr (std::is_same_v<T, double>) {
        return std::to_string(v);
      } else if constexpr (std::is_same_v<T, bool>) {
        return v ? "true" : "false";
      } else if constexpr (std::is_same_v<T, Vector2>) {
        return "Vector2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
      } else if constexpr (std::is_same_v<T, Color>) {
        return "Color(" + std::to_string(v.r) + ", " + std::to_string(v.g) + ", " +
               std::to_string(v.b) + ", " + std::to_string(v.a) + ")";
      }
      return "";
    }, val);
  }

  static Value deserializeValue(const std::string &str) {
    if (str.empty()) return std::string("");

    if (str.front() == '"' && str.back() == '"' && str.length() >= 2) {
      return str.substr(1, str.length() - 2);
    }
    if (str == "true") return true;
    if (str == "false") return false;

    if (str.rfind("Vector2(", 0) == 0 && str.back() == ')') {
      std::string inner = str.substr(8, str.length() - 9);
      size_t comma = inner.find(',');
      if (comma != std::string::npos) {
        float x = std::stof(trim(inner.substr(0, comma)));
        float y = std::stof(trim(inner.substr(comma + 1)));
        return Vector2(x, y);
      }
    }

    if (str.rfind("Color(", 0) == 0 && str.back() == ')') {
      std::string inner = str.substr(6, str.length() - 7);
      std::stringstream ss(inner);
      std::string part;
      std::vector<float> parts;
      while (std::getline(ss, part, ',')) {
        parts.push_back(std::stof(trim(part)));
      }
      if (parts.size() >= 3) {
        float a = parts.size() >= 4 ? parts[3] : 1.0f;
        return Color(parts[0], parts[1], parts[2], a);
      }
    }

    if (str.find('.') != std::string::npos) {
      try {
        return std::stod(str);
      } catch (...) {}
    } else {
      try {
        return static_cast<int64_t>(std::stoll(str));
      } catch (...) {}
    }

    return str;
  }

  std::map<std::string, std::map<std::string, Value>> m_data;
};
