#pragma once

#include "helper/vectors/Vector3.hpp"
#include <cmath>

// Godot-Standard 3D Plane representation (Ax + By + Cz + D = 0)
class Plane {
public:
  Vector3 normal{0.0f, 1.0f, 0.0f};
  float d = 0.0f;

  constexpr Plane() = default;
  constexpr Plane(float a, float b, float c, float d) : normal(a, b, c), d(d) {}
  constexpr Plane(const Vector3 &n, float d) : normal(n), d(d) {}

  // Constructs plane from a point on the plane and its normal
  Plane(const Vector3 &point, const Vector3 &normal)
      : normal(normal.normalized()), d(normal.dot(point)) {}

  // Constructs plane from 3 non-collinear clockwise points
  Plane(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3) {
    normal = ((p1 - p3).cross(p1 - p2)).normalized();
    d = normal.dot(p1);
  }

  Plane normalized() const {
    float len = normal.length();
    if (len == 0.0f) return Plane();
    return Plane(normal / len, d / len);
  }

  float distance_to(const Vector3 &point) const {
    return normal.dot(point) - d;
  }

  bool has_point(const Vector3 &point, float tolerance = 0.0001f) const {
    return std::abs(distance_to(point)) <= tolerance;
  }

  bool is_point_over(const Vector3 &point) const {
    return normal.dot(point) > d;
  }

  Vector3 project(const Vector3 &point) const {
    return point - normal * distance_to(point);
  }
};
