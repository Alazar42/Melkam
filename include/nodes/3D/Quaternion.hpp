#pragma once

#include "helper/vectors/Vector3.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

// Godot-Standard Quaternion for 3D rotations without gimbal lock
class Quaternion {
public:
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float w = 1.0f;

  constexpr Quaternion() = default;
  constexpr Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

  // Constructs Quaternion from axis and angle (in radians)
  Quaternion(const Vector3 &axis, float angle) {
    float len = axis.length();
    if (len == 0.0f) {
      x = 0.0f;
      y = 0.0f;
      z = 0.0f;
      w = 1.0f;
    } else {
      float sin_half = std::sin(angle * 0.5f) / len;
      x = axis.x * sin_half;
      y = axis.y * sin_half;
      z = axis.z * sin_half;
      w = std::cos(angle * 0.5f);
    }
  }

  // Constructs Quaternion from Euler angles (in radians, YXZ Godot order)
  static Quaternion fromEuler(const Vector3 &euler) {
    float half_x = euler.x * 0.5f;
    float half_y = euler.y * 0.5f;
    float half_z = euler.z * 0.5f;

    float sin_x = std::sin(half_x), cos_x = std::cos(half_x);
    float sin_y = std::sin(half_y), cos_y = std::cos(half_y);
    float sin_z = std::sin(half_z), cos_z = std::cos(half_z);

    return Quaternion(
        sin_x * cos_y * cos_z - cos_x * sin_y * sin_z,
        cos_x * sin_y * cos_z + sin_x * cos_y * sin_z,
        cos_x * cos_y * sin_z - sin_x * sin_y * cos_z,
        cos_x * cos_y * cos_z + sin_x * sin_y * sin_z);
  }

  float length_squared() const { return x * x + y * y + z * z + w * w; }
  float length() const { return std::sqrt(length_squared()); }

  Quaternion normalized() const {
    float len = length();
    if (len == 0.0f) return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    float inv = 1.0f / len;
    return Quaternion(x * inv, y * inv, z * inv, w * inv);
  }

  bool is_normalized(float tolerance = 0.0001f) const {
    return std::abs(length_squared() - 1.0f) < tolerance;
  }

  Quaternion inverse() const {
    float len_sq = length_squared();
    if (len_sq == 0.0f) return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    float inv = 1.0f / len_sq;
    return Quaternion(-x * inv, -y * inv, -z * inv, w * inv);
  }

  Quaternion conjugate() const {
    return Quaternion(-x, -y, -z, w);
  }

  float dot(const Quaternion &other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
  }

  // Spherical Linear Interpolation (SLERP)
  Quaternion slerp(const Quaternion &to, float weight) const {
    float cos_half_theta = dot(to);
    Quaternion target = to;

    if (cos_half_theta < 0.0f) {
      target = Quaternion(-to.x, -to.y, -to.z, -to.w);
      cos_half_theta = -cos_half_theta;
    }

    if (std::abs(cos_half_theta) >= 1.0f) {
      return *this;
    }

    float half_theta = std::acos(cos_half_theta);
    float sin_half_theta = std::sqrt(1.0f - cos_half_theta * cos_half_theta);

    if (std::abs(sin_half_theta) < 0.001f) {
      return Quaternion(
          x * (1.0f - weight) + target.x * weight,
          y * (1.0f - weight) + target.y * weight,
          z * (1.0f - weight) + target.z * weight,
          w * (1.0f - weight) + target.w * weight).normalized();
    }

    float ratio_a = std::sin((1.0f - weight) * half_theta) / sin_half_theta;
    float ratio_b = std::sin(weight * half_theta) / sin_half_theta;

    return Quaternion(
        x * ratio_a + target.x * ratio_b,
        y * ratio_a + target.y * ratio_b,
        z * ratio_a + target.z * ratio_b,
        w * ratio_a + target.w * ratio_b);
  }

  // Multiplies two Quaternions
  Quaternion operator*(const Quaternion &rhs) const {
    return Quaternion(
        w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
        w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
        w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x,
        w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
  }

  // Transforms a Vector3 by this rotation
  Vector3 xform(const Vector3 &v) const {
    Vector3 u(x, y, z);
    float s = w;
    return u * 2.0f * u.dot(v) +
           v * (s * s - u.dot(u)) +
           u.cross(v) * 2.0f * s;
  }

  Vector3 operator*(const Vector3 &v) const { return xform(v); }
};
