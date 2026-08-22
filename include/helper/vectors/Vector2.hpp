#pragma once

#include "vectors/exceptions.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

class Vector2 {
public:
  float x;
  float y;

  constexpr Vector2() : x(0.0f), y(0.0f) {}
  constexpr Vector2(float x, float y) : x(x), y(y) {}
  constexpr explicit Vector2(float scalar) : x(scalar), y(scalar) {}

  float &operator[](int index);
  float operator[](int index) const;

  float length() const;
  float length_squared() const;
  bool is_normalized() const;
  bool is_zero_approx(float tolerance = 0.00001f) const;
  bool is_equal_approx(const Vector2 &other,
                       float tolerance = 0.00001f) const;
  bool is_finite() const;

  float angle() const;
  float angle_to(const Vector2 &to) const;
  float angle_to_point(const Vector2 &to) const;
  float distance_to(const Vector2 &to) const;
  float distance_squared_to(const Vector2 &to) const;
  Vector2 direction_to(const Vector2 &to) const;

  float dot(const Vector2 &with) const;
  float cross(const Vector2 &with) const;
  Vector2 normalized() const;
  Vector2 rotated(float angle) const;
  Vector2 orthogonal() const;
  Vector2 reflect(const Vector2 &n) const;
  Vector2 bounce(const Vector2 &n) const;
  Vector2 project(const Vector2 &b) const;
  Vector2 slide(const Vector2 &n) const;

  Vector2 lerp(const Vector2 &to, float weight) const;
  Vector2 slerp(const Vector2 &to, float weight) const;
  Vector2 move_toward(const Vector2 &to, float delta) const;
  Vector2 clamp(const Vector2 &min, const Vector2 &max) const;
  Vector2 limit_length(float max_length = 1.0f) const;

  Vector2 min(const Vector2 &other) const;
  Vector2 max(const Vector2 &other) const;
  Vector2 abs() const;
  Vector2 floor() const;
  Vector2 ceil() const;
  Vector2 round() const;
  Vector2 sign() const;
  Vector2 snapped(const Vector2 &step) const;
  Vector2 posmod(float mod) const;
  Vector2 posmodv(const Vector2 &modv) const;
  float aspect() const;

  Vector2 operator+(const Vector2 &other) const;
  Vector2 operator-(const Vector2 &other) const;
  Vector2 operator*(const Vector2 &other) const;
  Vector2 operator/(const Vector2 &other) const;
  Vector2 operator*(float scalar) const;
  Vector2 operator/(float scalar) const;

  Vector2 &operator+=(const Vector2 &other);
  Vector2 &operator-=(const Vector2 &other);
  Vector2 &operator*=(const Vector2 &other);
  Vector2 &operator/=(const Vector2 &other);
  Vector2 &operator*=(float scalar);
  Vector2 &operator/=(float scalar);

  Vector2 operator-() const;

  bool operator==(const Vector2 &other) const;
  bool operator!=(const Vector2 &other) const;
  bool operator<(const Vector2 &other) const;

  friend Vector2 operator*(float scalar, const Vector2 &v) {
    return v * scalar;
  }

  friend std::ostream &operator<<(std::ostream &os, const Vector2 &v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
  }

  static const Vector2 ZERO;
  static const Vector2 ONE;
  static const Vector2 INF_VEC;
  static const Vector2 LEFT;
  static const Vector2 RIGHT;
  static const Vector2 UP;
  static const Vector2 DOWN;
};

inline const Vector2 Vector2::ZERO{0.0f, 0.0f};
inline const Vector2 Vector2::ONE{1.0f, 1.0f};
inline const Vector2 Vector2::INF_VEC{
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity()};
inline const Vector2 Vector2::LEFT{-1.0f, 0.0f};
inline const Vector2 Vector2::RIGHT{1.0f, 0.0f};
inline const Vector2 Vector2::UP{0.0f, -1.0f};
inline const Vector2 Vector2::DOWN{0.0f, 1.0f};

inline float &Vector2::operator[](int index) {
  switch (index) {
  case 0:
    return x;
  case 1:
    return y;
  default:
    throw Vector2IndexOutOfRange();
  }
}

inline float Vector2::operator[](int index) const {
  switch (index) {
  case 0:
    return x;
  case 1:
    return y;
  default:
    throw Vector2IndexOutOfRange();
  }
}

inline float Vector2::length() const {
  return std::sqrt(x * x + y * y);
}

inline float Vector2::length_squared() const {
  return x * x + y * y;
}

inline bool Vector2::is_normalized() const {
  return std::abs(length_squared() - 1.0f) <= 0.0001f;
}

inline bool Vector2::is_zero_approx(float tolerance) const {
  return std::abs(x) <= tolerance && std::abs(y) <= tolerance;
}

inline bool Vector2::is_equal_approx(const Vector2 &other,
                                     float tolerance) const {
  return std::abs(x - other.x) <= tolerance &&
         std::abs(y - other.y) <= tolerance;
}

inline bool Vector2::is_finite() const {
  return std::isfinite(x) && std::isfinite(y);
}

inline float Vector2::angle() const {
  return std::atan2(y, x);
}

inline float Vector2::angle_to(const Vector2 &to) const {
  return std::atan2(cross(to), dot(to));
}

inline float Vector2::angle_to_point(const Vector2 &to) const {
  return (to - *this).angle();
}

inline float Vector2::distance_to(const Vector2 &to) const {
  return (to - *this).length();
}

inline float Vector2::distance_squared_to(const Vector2 &to) const {
  return (to - *this).length_squared();
}

inline Vector2 Vector2::direction_to(const Vector2 &to) const {
  return (to - *this).normalized();
}

inline float Vector2::dot(const Vector2 &with) const {
  return x * with.x + y * with.y;
}

inline float Vector2::cross(const Vector2 &with) const {
  return x * with.y - y * with.x;
}

inline Vector2 Vector2::normalized() const {
  float len = length();
  if (len == 0.0f) {
    return Vector2(0.0f, 0.0f);
  }
  return Vector2(x / len, y / len);
}

inline Vector2 Vector2::rotated(float angle) const {
  float sine = std::sin(angle);
  float cosine = std::cos(angle);
  return Vector2(x * cosine - y * sine, x * sine + y * cosine);
}

inline Vector2 Vector2::orthogonal() const {
  return Vector2(-y, x);
}

inline Vector2 Vector2::reflect(const Vector2 &n) const {
  return 2.0f * n * dot(n) - *this;
}

inline Vector2 Vector2::bounce(const Vector2 &n) const {
  return -reflect(n);
}

inline Vector2 Vector2::project(const Vector2 &b) const {
  float len_sq = b.length_squared();
  if (len_sq == 0.0f) {
    return Vector2(0.0f, 0.0f);
  }
  return b * (dot(b) / len_sq);
}

inline Vector2 Vector2::slide(const Vector2 &n) const {
  return *this - n * dot(n);
}

inline Vector2 Vector2::lerp(const Vector2 &to, float weight) const {
  return Vector2(x + (to.x - x) * weight, y + (to.y - y) * weight);
}

inline Vector2 Vector2::slerp(const Vector2 &to, float weight) const {
  float start_len_sq = length_squared();
  float end_len_sq = to.length_squared();
  if (start_len_sq == 0.0f || end_len_sq == 0.0f) {
    return lerp(to, weight);
  }
  float start_len = std::sqrt(start_len_sq);
  float end_len = std::sqrt(end_len_sq);
  float len = start_len + (end_len - start_len) * weight;
  float ang = angle_to(to);
  return rotated(ang * weight).normalized() * len;
}

inline Vector2 Vector2::move_toward(const Vector2 &to, float delta) const {
  Vector2 vd = to - *this;
  float len = vd.length();
  if (len <= delta || len < 0.00001f) {
    return to;
  }
  return *this + (vd / len) * delta;
}

inline Vector2 Vector2::clamp(const Vector2 &min, const Vector2 &max) const {
  return Vector2(std::clamp(x, min.x, max.x), std::clamp(y, min.y, max.y));
}

inline Vector2 Vector2::limit_length(float max_length) const {
  float l = length();
  if (l > 0.0f && max_length < l) {
    return (*this / l) * max_length;
  }
  return *this;
}

inline Vector2 Vector2::min(const Vector2 &other) const {
  return Vector2(std::min(x, other.x), std::min(y, other.y));
}

inline Vector2 Vector2::max(const Vector2 &other) const {
  return Vector2(std::max(x, other.x), std::max(y, other.y));
}

inline Vector2 Vector2::abs() const {
  return Vector2(std::abs(x), std::abs(y));
}

inline Vector2 Vector2::floor() const {
  return Vector2(std::floor(x), std::floor(y));
}

inline Vector2 Vector2::ceil() const {
  return Vector2(std::ceil(x), std::ceil(y));
}

inline Vector2 Vector2::round() const {
  return Vector2(std::round(x), std::round(y));
}

inline Vector2 Vector2::sign() const {
  return Vector2(x > 0.0f ? 1.0f : (x < 0.0f ? -1.0f : 0.0f),
                 y > 0.0f ? 1.0f : (y < 0.0f ? -1.0f : 0.0f));
}

inline Vector2 Vector2::snapped(const Vector2 &step) const {
  return Vector2(
      step.x != 0.0f ? std::floor(x / step.x + 0.5f) * step.x : x,
      step.y != 0.0f ? std::floor(y / step.y + 0.5f) * step.y : y);
}

inline Vector2 Vector2::posmod(float mod) const {
  return Vector2(std::fmod(std::fmod(x, mod) + mod, mod),
                 std::fmod(std::fmod(y, mod) + mod, mod));
}

inline Vector2 Vector2::posmodv(const Vector2 &modv) const {
  return Vector2(
      modv.x != 0.0f ? std::fmod(std::fmod(x, modv.x) + modv.x, modv.x) : x,
      modv.y != 0.0f ? std::fmod(std::fmod(y, modv.y) + modv.y, modv.y) : y);
}

inline float Vector2::aspect() const {
  return y != 0.0f ? x / y : 0.0f;
}

inline Vector2 Vector2::operator+(const Vector2 &other) const {
  return Vector2(x + other.x, y + other.y);
}

inline Vector2 Vector2::operator-(const Vector2 &other) const {
  return Vector2(x - other.x, y - other.y);
}

inline Vector2 Vector2::operator*(const Vector2 &other) const {
  return Vector2(x * other.x, y * other.y);
}

inline Vector2 Vector2::operator/(const Vector2 &other) const {
  return Vector2(x / other.x, y / other.y);
}

inline Vector2 Vector2::operator*(float scalar) const {
  return Vector2(x * scalar, y * scalar);
}

inline Vector2 Vector2::operator/(float scalar) const {
  return Vector2(x / scalar, y / scalar);
}

inline Vector2 &Vector2::operator+=(const Vector2 &other) {
  x += other.x;
  y += other.y;
  return *this;
}

inline Vector2 &Vector2::operator-=(const Vector2 &other) {
  x -= other.x;
  y -= other.y;
  return *this;
}

inline Vector2 &Vector2::operator*=(const Vector2 &other) {
  x *= other.x;
  y *= other.y;
  return *this;
}

inline Vector2 &Vector2::operator/=(const Vector2 &other) {
  x /= other.x;
  y /= other.y;
  return *this;
}

inline Vector2 &Vector2::operator*=(float scalar) {
  x *= scalar;
  y *= scalar;
  return *this;
}

inline Vector2 &Vector2::operator/=(float scalar) {
  x /= scalar;
  y /= scalar;
  return *this;
}

inline Vector2 Vector2::operator-() const {
  return Vector2(-x, -y);
}

inline bool Vector2::operator==(const Vector2 &other) const {
  return x == other.x && y == other.y;
}

inline bool Vector2::operator!=(const Vector2 &other) const {
  return !(*this == other);
}

inline bool Vector2::operator<(const Vector2 &other) const {
  if (x != other.x)
    return x < other.x;
  return y < other.y;
}
