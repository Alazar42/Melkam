#pragma once

#include "color/exceptions.hpp"
#include "string/String.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

class Color {
public:
  float r;
  float g;
  float b;
  float a;

  constexpr Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
  constexpr Color(float r, float g, float b, float a = 1.0f)
      : r(r), g(g), b(b), a(a) {}

  Color(const String &code);
  Color(const char *code);

  static Color from_rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
  static Color from_hsv(float h, float s, float v, float a = 1.0f);
  static Color html(const String &rgba);
  static Color hex(uint32_t hex);
  static Color hex64(uint64_t hex);
  static Color from_string(const String &str,
                           const Color &default_color = Color());

  float &operator[](int index);
  float operator[](int index) const;

  float get_h() const;
  float get_s() const;
  float get_v() const;
  float get_luminance() const;

  uint32_t to_rgba32() const;
  uint32_t to_argb32() const;
  uint32_t to_abgr32() const;
  uint64_t to_rgba64() const;
  uint64_t to_argb64() const;
  uint64_t to_abgr64() const;
  String to_html(bool with_alpha = true) const;

  Color inverted() const;
  Color lightened(float amount) const;
  Color darkened(float amount) const;
  Color lerp(const Color &to, float weight) const;
  Color blend(const Color &over) const;
  Color clamp(const Color &min = Color(0.0f, 0.0f, 0.0f, 0.0f),
              const Color &max = Color(1.0f, 1.0f, 1.0f, 1.0f)) const;
  Color linear_to_srgb() const;
  Color srgb_to_linear() const;

  Color operator+(const Color &other) const;
  Color operator-(const Color &other) const;
  Color operator*(const Color &other) const;
  Color operator/(const Color &other) const;
  Color operator*(float scalar) const;
  Color operator/(float scalar) const;

  Color &operator+=(const Color &other);
  Color &operator-=(const Color &other);
  Color &operator*=(const Color &other);
  Color &operator/=(const Color &other);
  Color &operator*=(float scalar);
  Color &operator/=(float scalar);

  Color operator-() const;

  bool operator==(const Color &other) const;
  bool operator!=(const Color &other) const;
  bool operator<(const Color &other) const;
  bool is_equal_approx(const Color &other, float tolerance = 0.00001f) const;

  friend Color operator*(float scalar, const Color &c) { return c * scalar; }

  friend std::ostream &operator<<(std::ostream &os, const Color &color) {
    os << "(" << color.r << ", " << color.g << ", " << color.b << ", "
       << color.a << ")";
    return os;
  }

  static const Color ALICE_BLUE;
  static const Color AQUA;
  static const Color AQUAMARINE;
  static const Color BLACK;
  static const Color BLUE;
  static const Color CHARTREUSE;
  static const Color CLEAR;
  static const Color CORAL;
  static const Color CYAN;
  static const Color DARK_BLUE;
  static const Color DARK_CYAN;
  static const Color DARK_GRAY;
  static const Color DARK_GREEN;
  static const Color DARK_MAGENTA;
  static const Color DARK_RED;
  static const Color GOLD;
  static const Color GRAY;
  static const Color GREEN;
  static const Color LIGHT_BLUE;
  static const Color LIGHT_GRAY;
  static const Color MAGENTA;
  static const Color ORANGE;
  static const Color PINK;
  static const Color PURPLE;
  static const Color RED;
  static const Color TRANSPARENT_COLOR;
  static const Color VIOLET;
  static const Color WHITE;
  static const Color YELLOW;
};

inline const Color Color::ALICE_BLUE{0.941176f, 0.972549f, 1.0f, 1.0f};
inline const Color Color::AQUA{0.0f, 1.0f, 1.0f, 1.0f};
inline const Color Color::AQUAMARINE{0.498039f, 1.0f, 0.831373f, 1.0f};
inline const Color Color::BLACK{0.0f, 0.0f, 0.0f, 1.0f};
inline const Color Color::BLUE{0.0f, 0.0f, 1.0f, 1.0f};
inline const Color Color::CHARTREUSE{0.498039f, 1.0f, 0.0f, 1.0f};
inline const Color Color::CLEAR{0.0f, 0.0f, 0.0f, 0.0f};
inline const Color Color::CORAL{1.0f, 0.498039f, 0.313726f, 1.0f};
inline const Color Color::CYAN{0.0f, 1.0f, 1.0f, 1.0f};
inline const Color Color::DARK_BLUE{0.0f, 0.0f, 0.545098f, 1.0f};
inline const Color Color::DARK_CYAN{0.0f, 0.545098f, 0.545098f, 1.0f};
inline const Color Color::DARK_GRAY{0.662745f, 0.662745f, 0.662745f, 1.0f};
inline const Color Color::DARK_GREEN{0.0f, 0.392157f, 0.0f, 1.0f};
inline const Color Color::DARK_MAGENTA{0.545098f, 0.0f, 0.545098f, 1.0f};
inline const Color Color::DARK_RED{0.545098f, 0.0f, 0.0f, 1.0f};
inline const Color Color::GOLD{1.0f, 0.843137f, 0.0f, 1.0f};
inline const Color Color::GRAY{0.745098f, 0.745098f, 0.745098f, 1.0f};
inline const Color Color::GREEN{0.0f, 1.0f, 0.0f, 1.0f};
inline const Color Color::LIGHT_BLUE{0.678431f, 0.847059f, 0.901961f, 1.0f};
inline const Color Color::LIGHT_GRAY{0.827451f, 0.827451f, 0.827451f, 1.0f};
inline const Color Color::MAGENTA{1.0f, 0.0f, 1.0f, 1.0f};
inline const Color Color::ORANGE{1.0f, 0.647059f, 0.0f, 1.0f};
inline const Color Color::PINK{1.0f, 0.752941f, 0.796078f, 1.0f};
inline const Color Color::PURPLE{0.627451f, 0.12549f, 0.941176f, 1.0f};
inline const Color Color::RED{1.0f, 0.0f, 0.0f, 1.0f};
inline const Color Color::TRANSPARENT_COLOR{0.0f, 0.0f, 0.0f, 0.0f};
inline const Color Color::VIOLET{0.933333f, 0.509804f, 0.933333f, 1.0f};
inline const Color Color::WHITE{1.0f, 1.0f, 1.0f, 1.0f};
inline const Color Color::YELLOW{1.0f, 1.0f, 0.0f, 1.0f};

inline Color::Color(const String &code) { *this = html(code); }

inline Color::Color(const char *code) { *this = html(String(code)); }

inline Color Color::from_rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

inline Color Color::from_hsv(float h, float s, float v, float a) {
  h = std::fmod(h, 1.0f);
  if (h < 0.0f)
    h += 1.0f;
  s = std::clamp(s, 0.0f, 1.0f);
  v = std::clamp(v, 0.0f, 1.0f);

  int i = static_cast<int>(h * 6.0f);
  float f = (h * 6.0f) - i;
  float p = v * (1.0f - s);
  float q = v * (1.0f - s * f);
  float t = v * (1.0f - s * (1.0f - f));

  float cr = 0.0f, cg = 0.0f, cb = 0.0f;
  switch (i % 6) {
  case 0:
    cr = v;
    cg = t;
    cb = p;
    break;
  case 1:
    cr = q;
    cg = v;
    cb = p;
    break;
  case 2:
    cr = p;
    cg = v;
    cb = t;
    break;
  case 3:
    cr = p;
    cg = q;
    cb = v;
    break;
  case 4:
    cr = t;
    cg = p;
    cb = v;
    break;
  case 5:
    cr = v;
    cg = p;
    cb = q;
    break;
  }
  return Color(cr, cg, cb, a);
}

inline Color Color::html(const String &rgba) {
  std::string s = rgba.c_str();
  if (s.empty()) {
    throw InvalidColorFormat();
  }
  if (s[0] == '#') {
    s = s.substr(1);
  }

  for (char c : s) {
    if (!std::isxdigit(static_cast<unsigned char>(c))) {
      throw InvalidColorFormat();
    }
  }

  if (s.length() == 3) {
    int r_val = std::stoi(std::string(2, s[0]), nullptr, 16);
    int g_val = std::stoi(std::string(2, s[1]), nullptr, 16);
    int b_val = std::stoi(std::string(2, s[2]), nullptr, 16);
    return Color(r_val / 255.0f, g_val / 255.0f, b_val / 255.0f, 1.0f);
  } else if (s.length() == 4) {
    int r_val = std::stoi(std::string(2, s[0]), nullptr, 16);
    int g_val = std::stoi(std::string(2, s[1]), nullptr, 16);
    int b_val = std::stoi(std::string(2, s[2]), nullptr, 16);
    int a_val = std::stoi(std::string(2, s[3]), nullptr, 16);
    return Color(r_val / 255.0f, g_val / 255.0f, b_val / 255.0f,
                 a_val / 255.0f);
  } else if (s.length() == 6) {
    int r_val = std::stoi(s.substr(0, 2), nullptr, 16);
    int g_val = std::stoi(s.substr(2, 2), nullptr, 16);
    int b_val = std::stoi(s.substr(4, 2), nullptr, 16);
    return Color(r_val / 255.0f, g_val / 255.0f, b_val / 255.0f, 1.0f);
  } else if (s.length() == 8) {
    int r_val = std::stoi(s.substr(0, 2), nullptr, 16);
    int g_val = std::stoi(s.substr(2, 2), nullptr, 16);
    int b_val = std::stoi(s.substr(4, 2), nullptr, 16);
    int a_val = std::stoi(s.substr(6, 2), nullptr, 16);
    return Color(r_val / 255.0f, g_val / 255.0f, b_val / 255.0f,
                 a_val / 255.0f);
  }

  throw InvalidColorFormat();
}

inline Color Color::hex(uint32_t hex) {
  float r = ((hex >> 24) & 0xFF) / 255.0f;
  float g = ((hex >> 16) & 0xFF) / 255.0f;
  float b = ((hex >> 8) & 0xFF) / 255.0f;
  float a = (hex & 0xFF) / 255.0f;
  return Color(r, g, b, a);
}

inline Color Color::hex64(uint64_t hex) {
  float r = ((hex >> 48) & 0xFFFF) / 65535.0f;
  float g = ((hex >> 32) & 0xFFFF) / 65535.0f;
  float b = ((hex >> 16) & 0xFFFF) / 65535.0f;
  float a = (hex & 0xFFFF) / 65535.0f;
  return Color(r, g, b, a);
}

inline Color Color::from_string(const String &str, const Color &default_color) {
  try {
    return html(str);
  } catch (...) {
    return default_color;
  }
}

inline float &Color::operator[](int index) {
  switch (index) {
  case 0:
    return r;
  case 1:
    return g;
  case 2:
    return b;
  case 3:
    return a;
  default:
    throw ColorIndexOutOfRange();
  }
}

inline float Color::operator[](int index) const {
  switch (index) {
  case 0:
    return r;
  case 1:
    return g;
  case 2:
    return b;
  case 3:
    return a;
  default:
    throw ColorIndexOutOfRange();
  }
}

inline float Color::get_h() const {
  float min_val = std::min({r, g, b});
  float max_val = std::max({r, g, b});
  float delta = max_val - min_val;

  if (delta == 0.0f)
    return 0.0f;

  float h = 0.0f;
  if (r == max_val)
    h = (g - b) / delta;
  else if (g == max_val)
    h = 2.0f + (b - r) / delta;
  else
    h = 4.0f + (r - g) / delta;

  h /= 6.0f;
  if (h < 0.0f)
    h += 1.0f;
  return h;
}

inline float Color::get_s() const {
  float min_val = std::min({r, g, b});
  float max_val = std::max({r, g, b});
  if (max_val == 0.0f)
    return 0.0f;
  return (max_val - min_val) / max_val;
}

inline float Color::get_v() const { return std::max({r, g, b}); }

inline float Color::get_luminance() const {
  return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

inline uint32_t Color::to_rgba32() const {
  uint32_t c =
      static_cast<uint8_t>(std::clamp(r * 255.0f + 0.5f, 0.0f, 255.0f));
  c <<= 8;
  c |= static_cast<uint8_t>(std::clamp(g * 255.0f + 0.5f, 0.0f, 255.0f));
  c <<= 8;
  c |= static_cast<uint8_t>(std::clamp(b * 255.0f + 0.5f, 0.0f, 255.0f));
  c <<= 8;
  c |= static_cast<uint8_t>(std::clamp(a * 255.0f + 0.5f, 0.0f, 255.0f));
  return c;
}

inline uint32_t Color::to_argb32() const {
  uint32_t c =
      static_cast<uint8_t>(std::clamp(a * 255.0f + 0.5f, 0.0f, 255.0f));
  c <<= 8;
  c |= static_cast<uint8_t>(std::clamp(r * 255.0f + 0.5f, 0.0f, 255.0f));
  c <<= 8;
  c |= static_cast<uint8_t>(std::clamp(g * 255.0f + 0.5f, 0.0f, 255.0f));
  c <<= 8;
  c |= static_cast<uint8_t>(std::clamp(b * 255.0f + 0.5f, 0.0f, 255.0f));
  return c;
}

inline uint32_t Color::to_abgr32() const {
  uint32_t c =
      static_cast<uint8_t>(std::clamp(a * 255.0f + 0.5f, 0.0f, 255.0f));
  c <<= 8;
  c |= static_cast<uint8_t>(std::clamp(b * 255.0f + 0.5f, 0.0f, 255.0f));
  c <<= 8;
  c |= static_cast<uint8_t>(std::clamp(g * 255.0f + 0.5f, 0.0f, 255.0f));
  c <<= 8;
  c |= static_cast<uint8_t>(std::clamp(r * 255.0f + 0.5f, 0.0f, 255.0f));
  return c;
}

inline uint64_t Color::to_rgba64() const {
  uint64_t c =
      static_cast<uint16_t>(std::clamp(r * 65535.0f + 0.5f, 0.0f, 65535.0f));
  c <<= 16;
  c |= static_cast<uint16_t>(std::clamp(g * 65535.0f + 0.5f, 0.0f, 65535.0f));
  c <<= 16;
  c |= static_cast<uint16_t>(std::clamp(b * 65535.0f + 0.5f, 0.0f, 65535.0f));
  c <<= 16;
  c |= static_cast<uint16_t>(std::clamp(a * 65535.0f + 0.5f, 0.0f, 65535.0f));
  return c;
}

inline uint64_t Color::to_argb64() const {
  uint64_t c =
      static_cast<uint16_t>(std::clamp(a * 65535.0f + 0.5f, 0.0f, 65535.0f));
  c <<= 16;
  c |= static_cast<uint16_t>(std::clamp(r * 65535.0f + 0.5f, 0.0f, 65535.0f));
  c <<= 16;
  c |= static_cast<uint16_t>(std::clamp(g * 65535.0f + 0.5f, 0.0f, 65535.0f));
  c <<= 16;
  c |= static_cast<uint16_t>(std::clamp(b * 65535.0f + 0.5f, 0.0f, 65535.0f));
  return c;
}

inline uint64_t Color::to_abgr64() const {
  uint64_t c =
      static_cast<uint16_t>(std::clamp(a * 65535.0f + 0.5f, 0.0f, 65535.0f));
  c <<= 16;
  c |= static_cast<uint16_t>(std::clamp(b * 65535.0f + 0.5f, 0.0f, 65535.0f));
  c <<= 16;
  c |= static_cast<uint16_t>(std::clamp(g * 65535.0f + 0.5f, 0.0f, 65535.0f));
  c <<= 16;
  c |= static_cast<uint16_t>(std::clamp(r * 65535.0f + 0.5f, 0.0f, 65535.0f));
  return c;
}

inline String Color::to_html(bool with_alpha) const {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  ss << std::setw(2)
     << static_cast<int>(std::clamp(r * 255.0f + 0.5f, 0.0f, 255.0f));
  ss << std::setw(2)
     << static_cast<int>(std::clamp(g * 255.0f + 0.5f, 0.0f, 255.0f));
  ss << std::setw(2)
     << static_cast<int>(std::clamp(b * 255.0f + 0.5f, 0.0f, 255.0f));
  if (with_alpha) {
    ss << std::setw(2)
       << static_cast<int>(std::clamp(a * 255.0f + 0.5f, 0.0f, 255.0f));
  }
  return String(ss.str().c_str());
}

inline Color Color::inverted() const {
  return Color(1.0f - r, 1.0f - g, 1.0f - b, a);
}

inline Color Color::lightened(float amount) const {
  return Color(r + (1.0f - r) * amount, g + (1.0f - g) * amount,
               b + (1.0f - b) * amount, a);
}

inline Color Color::darkened(float amount) const {
  return Color(r * (1.0f - amount), g * (1.0f - amount), b * (1.0f - amount),
               a);
}

inline Color Color::lerp(const Color &to, float weight) const {
  return Color(r + (to.r - r) * weight, g + (to.g - g) * weight,
               b + (to.b - b) * weight, a + (to.a - a) * weight);
}

inline Color Color::blend(const Color &over) const {
  Color res;
  float sa = 1.0f - over.a;
  res.a = a * sa + over.a;
  if (res.a == 0.0f) {
    return Color(0.0f, 0.0f, 0.0f, 0.0f);
  }
  res.r = (r * a * sa + over.r * over.a) / res.a;
  res.g = (g * a * sa + over.g * over.a) / res.a;
  res.b = (b * a * sa + over.b * over.a) / res.a;
  return res;
}

inline Color Color::clamp(const Color &min, const Color &max) const {
  return Color(std::clamp(r, min.r, max.r), std::clamp(g, min.g, max.g),
               std::clamp(b, min.b, max.b), std::clamp(a, min.a, max.a));
}

inline Color Color::linear_to_srgb() const {
  auto conv = [](float c) {
    return c <= 0.0031308f ? c * 12.92f
                           : 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
  };
  return Color(conv(r), conv(g), conv(b), a);
}

inline Color Color::srgb_to_linear() const {
  auto conv = [](float c) {
    return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
  };
  return Color(conv(r), conv(g), conv(b), a);
}

inline Color Color::operator+(const Color &other) const {
  return Color(r + other.r, g + other.g, b + other.b, a + other.a);
}

inline Color Color::operator-(const Color &other) const {
  return Color(r - other.r, g - other.g, b - other.b, a - other.a);
}

inline Color Color::operator*(const Color &other) const {
  return Color(r * other.r, g * other.g, b * other.b, a * other.a);
}

inline Color Color::operator/(const Color &other) const {
  return Color(r / other.r, g / other.g, b / other.b, a / other.a);
}

inline Color Color::operator*(float scalar) const {
  return Color(r * scalar, g * scalar, b * scalar, a * scalar);
}

inline Color Color::operator/(float scalar) const {
  return Color(r / scalar, g / scalar, b / scalar, a / scalar);
}

inline Color &Color::operator+=(const Color &other) {
  r += other.r;
  g += other.g;
  b += other.b;
  a += other.a;
  return *this;
}

inline Color &Color::operator-=(const Color &other) {
  r -= other.r;
  g -= other.g;
  b -= other.b;
  a -= other.a;
  return *this;
}

inline Color &Color::operator*=(const Color &other) {
  r *= other.r;
  g *= other.g;
  b *= other.b;
  a *= other.a;
  return *this;
}

inline Color &Color::operator/=(const Color &other) {
  r /= other.r;
  g /= other.g;
  b /= other.b;
  a /= other.a;
  return *this;
}

inline Color &Color::operator*=(float scalar) {
  r *= scalar;
  g *= scalar;
  b *= scalar;
  a *= scalar;
  return *this;
}

inline Color &Color::operator/=(float scalar) {
  r /= scalar;
  g /= scalar;
  b /= scalar;
  a /= scalar;
  return *this;
}

inline Color Color::operator-() const { return Color(-r, -g, -b, -a); }

inline bool Color::operator==(const Color &other) const {
  return r == other.r && g == other.g && b == other.b && a == other.a;
}

inline bool Color::operator!=(const Color &other) const {
  return !(*this == other);
}

inline bool Color::operator<(const Color &other) const {
  if (r != other.r)
    return r < other.r;
  if (g != other.g)
    return g < other.g;
  if (b != other.b)
    return b < other.b;
  return a < other.a;
}

inline bool Color::is_equal_approx(const Color &other, float tolerance) const {
  return std::abs(r - other.r) <= tolerance &&
         std::abs(g - other.g) <= tolerance &&
         std::abs(b - other.b) <= tolerance &&
         std::abs(a - other.a) <= tolerance;
}
