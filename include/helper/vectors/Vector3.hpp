#pragma once

#include "vectors/exceptions.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

class Vector3 {
public:
  float x;
  float y;
  float z;

  constexpr Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
  constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
  constexpr explicit Vector3(float scalar) : x(scalar), y(scalar), z(scalar) {}

  float &operator[](int index);
  float operator[](int index) const;

  float length() const;
  float length_squared() const;
  bool is_normalized() const;
  bool is_zero_approx(float tolerance = 0.00001f) const;
  bool is_equal_approx(const Vector3 &other,
                       float tolerance = 0.00001f) const;
  bool is_finite() const;

  int min_axis_index() const;
  int max_axis_index() const;

  float distance_to(const Vector3 &to) const;
  float distance_squared_to(const Vector3 &to) const;
  Vector3 direction_to(const Vector3 &to) const;
  float angle_to(const Vector3 &to) const;

  float dot(const Vector3 &with) const;
  Vector3 cross(const Vector3 &with) const;
  Vector3 normalized() const;
  Vector3 inverse() const;
  Vector3 reflect(const Vector3 &n) const;
  Vector3 bounce(const Vector3 &n) const;
  Vector3 project(const Vector3 &b) const;
  Vector3 slide(const Vector3 &n) const;

  Vector3 lerp(const Vector3 &to, float weight) const;
  Vector3 slerp(const Vector3 &to, float weight) const;
  Vector3 move_toward(const Vector3 &to, float delta) const;
  Vector3 clamp(const Vector3 &min, const Vector3 &max) const;
  Vector3 limit_length(float max_length = 1.0f) const;

  Vector3 min(const Vector3 &other) const;
  Vector3 max(const Vector3 &other) const;
  Vector3 abs() const;
  Vector3 floor() const;
  Vector3 ceil() const;
  Vector3 round() const;
  Vector3 sign() const;
  Vector3 snapped(const Vector3 &step) const;
  Vector3 posmod(float mod) const;
  Vector3 posmodv(const Vector3 &modv) const;
  Vector3 rotated(const Vector3 &axis, float angle) const;

  Vector3 operator+(const Vector3 &other) const;
  Vector3 operator-(const Vector3 &other) const;
  Vector3 operator*(const Vector3 &other) const;
  Vector3 operator/(const Vector3 &other) const;
  Vector3 operator*(float scalar) const;
  Vector3 operator/(float scalar) const;

  Vector3 &operator+=(const Vector3 &other);
  Vector3 &operator-=(const Vector3 &other);
  Vector3 &operator*=(const Vector3 &other);
  Vector3 &operator/=(const Vector3 &other);
  Vector3 &operator*=(float scalar);
  Vector3 &operator/=(float scalar);

  Vector3 operator-() const;

  bool operator==(const Vector3 &other) const;
  bool operator!=(const Vector3 &other) const;
  bool operator<(const Vector3 &other) const;

  friend Vector3 operator*(float scalar, const Vector3 &v) {
    return v * scalar;
  }

  friend std::ostream &operator<<(std::ostream &os, const Vector3 &v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
  }

  static const Vector3 ZERO;
  static const Vector3 ONE;
  static const Vector3 INF_VEC;
  static const Vector3 LEFT;
  static const Vector3 RIGHT;
  static const Vector3 UP;
  static const Vector3 DOWN;
  static const Vector3 FORWARD;
  static const Vector3 BACK;
};

inline const Vector3 Vector3::ZERO{0.0f, 0.0f, 0.0f};
inline const Vector3 Vector3::ONE{1.0f, 1.0f, 1.0f};
inline const Vector3 Vector3::INF_VEC{
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity()};
inline const Vector3 Vector3::LEFT{-1.0f, 0.0f, 0.0f};
inline const Vector3 Vector3::RIGHT{1.0f, 0.0f, 0.0f};
inline const Vector3 Vector3::UP{0.0f, 1.0f, 0.0f};
inline const Vector3 Vector3::DOWN{0.0f, -1.0f, 0.0f};
inline const Vector3 Vector3::FORWARD{0.0f, 0.0f, -1.0f};
inline const Vector3 Vector3::BACK{0.0f, 0.0f, 1.0f};

inline float &Vector3::operator[](int index) {
  switch (index) {
  case 0:
    return x;
  case 1:
    return y;
  case 2:
    return z;
  default:
    throw Vector3IndexOutOfRange();
  }
}

inline float Vector3::operator[](int index) const {
  switch (index) {
  case 0:
    return x;
  case 1:
    return y;
  case 2:
    return z;
  default:
    throw Vector3IndexOutOfRange();
  }
}

inline float Vector3::length() const {
  return std::sqrt(x * x + y * y + z * z);
}

inline float Vector3::length_squared() const {
  return x * x + y * y + z * z;
}

inline bool Vector3::is_normalized() const {
  return std::abs(length_squared() - 1.0f) <= 0.0001f;
}

inline bool Vector3::is_zero_approx(float tolerance) const {
  return std::abs(x) <= tolerance && std::abs(y) <= tolerance &&
         std::abs(z) <= tolerance;
}

inline bool Vector3::is_equal_approx(const Vector3 &other,
                                     float tolerance) const {
  return std::abs(x - other.x) <= tolerance &&
         std::abs(y - other.y) <= tolerance &&
         std::abs(z - other.z) <= tolerance;
}

inline bool Vector3::is_finite() const {
  return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
}

inline int Vector3::min_axis_index() const {
  if (x <= y && x <= z)
    return 0;
  if (y <= z)
    return 1;
  return 2;
}

inline int Vector3::max_axis_index() const {
  if (x >= y && x >= z)
    return 0;
  if (y >= z)
    return 1;
  return 2;
}

inline float Vector3::distance_to(const Vector3 &to) const {
  return (to - *this).length();
}

inline float Vector3::distance_squared_to(const Vector3 &to) const {
  return (to - *this).length_squared();
}

inline Vector3 Vector3::direction_to(const Vector3 &to) const {
  return (to - *this).normalized();
}

inline float Vector3::angle_to(const Vector3 &to) const {
  return std::atan2(cross(to).length(), dot(to));
}

inline float Vector3::dot(const Vector3 &with) const {
  return x * with.x + y * with.y + z * with.z;
}

inline Vector3 Vector3::cross(const Vector3 &with) const {
  return Vector3(y * with.z - z * with.y,
                 z * with.x - x * with.z,
                 x * with.y - y * with.x);
}

inline Vector3 Vector3::normalized() const {
  float len = length();
  if (len == 0.0f) {
    return Vector3(0.0f, 0.0f, 0.0f);
  }
  return Vector3(x / len, y / len, z / len);
}

inline Vector3 Vector3::inverse() const {
  return Vector3(1.0f / x, 1.0f / y, 1.0f / z);
}

inline Vector3 Vector3::reflect(const Vector3 &n) const {
  return 2.0f * n * dot(n) - *this;
}

inline Vector3 Vector3::bounce(const Vector3 &n) const {
  return -reflect(n);
}

inline Vector3 Vector3::project(const Vector3 &b) const {
  float len_sq = b.length_squared();
  if (len_sq == 0.0f) {
    return Vector3(0.0f, 0.0f, 0.0f);
  }
  return b * (dot(b) / len_sq);
}

inline Vector3 Vector3::slide(const Vector3 &n) const {
  return *this - n * dot(n);
}

inline Vector3 Vector3::lerp(const Vector3 &to, float weight) const {
  return Vector3(x + (to.x - x) * weight,
                 y + (to.y - y) * weight,
                 z + (to.z - z) * weight);
}

inline Vector3 Vector3::slerp(const Vector3 &to, float weight) const {
  float start_len_sq = length_squared();
  float end_len_sq = to.length_squared();
  if (start_len_sq == 0.0f || end_len_sq == 0.0f) {
    return lerp(to, weight);
  }
  float start_len = std::sqrt(start_len_sq);
  float end_len = std::sqrt(end_len_sq);
  float len = start_len + (end_len - start_len) * weight;

  Vector3 v0 = *this / start_len;
  Vector3 v1 = to / end_len;

  float cos_omega = v0.dot(v1);
  if (cos_omega > 0.9999f) {
    return lerp(to, weight);
  }
  if (cos_omega < -0.9999f) {
    // Vectors are opposite, pick an orthogonal axis
    Vector3 axis = v0.cross(Vector3(1.0f, 0.0f, 0.0f));
    if (axis.length_squared() < 0.001f) {
      axis = v0.cross(Vector3(0.0f, 1.0f, 0.0f));
    }
    constexpr float PI_CONST = 3.14159265358979323846f;
    return v0.rotated(axis.normalized(), PI_CONST * weight) * len;
  }

  float omega = std::acos(std::clamp(cos_omega, -1.0f, 1.0f));
  float sin_omega = std::sin(omega);
  float scale0 = std::sin((1.0f - weight) * omega) / sin_omega;
  float scale1 = std::sin(weight * omega) / sin_omega;

  return (v0 * scale0 + v1 * scale1) * len;
}

inline Vector3 Vector3::move_toward(const Vector3 &to, float delta) const {
  Vector3 vd = to - *this;
  float len = vd.length();
  if (len <= delta || len < 0.00001f) {
    return to;
  }
  return *this + (vd / len) * delta;
}

inline Vector3 Vector3::clamp(const Vector3 &min, const Vector3 &max) const {
  return Vector3(std::clamp(x, min.x, max.x),
                 std::clamp(y, min.y, max.y),
                 std::clamp(z, min.z, max.z));
}

inline Vector3 Vector3::limit_length(float max_length) const {
  float l = length();
  if (l > 0.0f && max_length < l) {
    return (*this / l) * max_length;
  }
  return *this;
}

inline Vector3 Vector3::min(const Vector3 &other) const {
  return Vector3(std::min(x, other.x),
                 std::min(y, other.y),
                 std::min(z, other.z));
}

inline Vector3 Vector3::max(const Vector3 &other) const {
  return Vector3(std::max(x, other.x),
                 std::max(y, other.y),
                 std::max(z, other.z));
}

inline Vector3 Vector3::abs() const {
  return Vector3(std::abs(x), std::abs(y), std::abs(z));
}

inline Vector3 Vector3::floor() const {
  return Vector3(std::floor(x), std::floor(y), std::floor(z));
}

inline Vector3 Vector3::ceil() const {
  return Vector3(std::ceil(x), std::ceil(y), std::ceil(z));
}

inline Vector3 Vector3::round() const {
  return Vector3(std::round(x), std::round(y), std::round(z));
}

inline Vector3 Vector3::sign() const {
  return Vector3(x > 0.0f ? 1.0f : (x < 0.0f ? -1.0f : 0.0f),
                 y > 0.0f ? 1.0f : (y < 0.0f ? -1.0f : 0.0f),
                 z > 0.0f ? 1.0f : (z < 0.0f ? -1.0f : 0.0f));
}

inline Vector3 Vector3::snapped(const Vector3 &step) const {
  return Vector3(
      step.x != 0.0f ? std::floor(x / step.x + 0.5f) * step.x : x,
      step.y != 0.0f ? std::floor(y / step.y + 0.5f) * step.y : y,
      step.z != 0.0f ? std::floor(z / step.z + 0.5f) * step.z : z);
}

inline Vector3 Vector3::posmod(float mod) const {
  return Vector3(std::fmod(std::fmod(x, mod) + mod, mod),
                 std::fmod(std::fmod(y, mod) + mod, mod),
                 std::fmod(std::fmod(z, mod) + mod, mod));
}

inline Vector3 Vector3::posmodv(const Vector3 &modv) const {
  return Vector3(
      modv.x != 0.0f ? std::fmod(std::fmod(x, modv.x) + modv.x, modv.x) : x,
      modv.y != 0.0f ? std::fmod(std::fmod(y, modv.y) + modv.y, modv.y) : y,
      modv.z != 0.0f ? std::fmod(std::fmod(z, modv.z) + modv.z, modv.z) : z);
}

inline Vector3 Vector3::rotated(const Vector3 &axis, float angle) const {
  Vector3 k = axis.normalized();
  float cos_theta = std::cos(angle);
  float sin_theta = std::sin(angle);
  return (*this) * cos_theta + k.cross(*this) * sin_theta +
         k * (k.dot(*this)) * (1.0f - cos_theta);
}

inline Vector3 Vector3::operator+(const Vector3 &other) const {
  return Vector3(x + other.x, y + other.y, z + other.z);
}

inline Vector3 Vector3::operator-(const Vector3 &other) const {
  return Vector3(x - other.x, y - other.y, z - other.z);
}

inline Vector3 Vector3::operator*(const Vector3 &other) const {
  return Vector3(x * other.x, y * other.y, z * other.z);
}

inline Vector3 Vector3::operator/(const Vector3 &other) const {
  return Vector3(x / other.x, y / other.y, z / other.z);
}

inline Vector3 Vector3::operator*(float scalar) const {
  return Vector3(x * scalar, y * scalar, z * scalar);
}

inline Vector3 Vector3::operator/(float scalar) const {
  return Vector3(x / scalar, y / scalar, z / scalar);
}

inline Vector3 &Vector3::operator+=(const Vector3 &other) {
  x += other.x;
  y += other.y;
  z += other.z;
  return *this;
}

inline Vector3 &Vector3::operator-=(const Vector3 &other) {
  x -= other.x;
  y -= other.y;
  z -= other.z;
  return *this;
}

inline Vector3 &Vector3::operator*=(const Vector3 &other) {
  x *= other.x;
  y *= other.y;
  z *= other.z;
  return *this;
}

inline Vector3 &Vector3::operator/=(const Vector3 &other) {
  x /= other.x;
  y /= other.y;
  z /= other.z;
  return *this;
}

inline Vector3 &Vector3::operator*=(float scalar) {
  x *= scalar;
  y *= scalar;
  z *= scalar;
  return *this;
}

inline Vector3 &Vector3::operator/=(float scalar) {
  x /= scalar;
  y /= scalar;
  z /= scalar;
  return *this;
}

inline Vector3 Vector3::operator-() const {
  return Vector3(-x, -y, -z);
}

inline bool Vector3::operator==(const Vector3 &other) const {
  return x == other.x && y == other.y && z == other.z;
}

inline bool Vector3::operator!=(const Vector3 &other) const {
  return !(*this == other);
}

inline bool Vector3::operator<(const Vector3 &other) const {
  if (x != other.x)
    return x < other.x;
  if (y != other.y)
    return y < other.y;
  return z < other.z;
}
