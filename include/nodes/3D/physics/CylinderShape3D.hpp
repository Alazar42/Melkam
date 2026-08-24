#pragma once

#include "nodes/3D/physics/Shape3D.hpp"
#include <algorithm>
#include <cmath>

// 3D Cylinder Collision Shape (inspired by Godot CylinderShape3D)
class CylinderShape3D : public Shape3D {
public:
  float radius = 0.5f;
  float height = 2.0f;

  CylinderShape3D() = default;
  CylinderShape3D(float radius, float height) : radius(radius), height(height) {}

  AABB getAABB() const override {
    float hh = height * 0.5f;
    return AABB(Vector3(-radius, -hh, -radius), Vector3(radius * 2.0f, height, radius * 2.0f));
  }

  bool intersectsRay(const Ray3D &ray, const Transform3D &transform, RayCastHit3D &outHit) const override {
    Vector3 center = transform.origin;
    Vector3 oc = ray.origin - center;
    float b = oc.dot(ray.direction);
    float c = oc.dot(oc) - radius * radius;

    if (c > 0.0f && b > 0.0f) return false;
    float discriminant = b * b - c;
    if (discriminant < 0.0f) return false;

    float t = -b - std::sqrt(discriminant);
    if (t < 0.0f) t = -b + std::sqrt(discriminant);

    if (t >= 0.0f && t <= ray.maxDistance) {
      outHit.hit = true;
      outHit.distance = t;
      outHit.point = ray.origin + ray.direction * t;
      outHit.normal = Vector3(outHit.point.x - center.x, 0.0f, outHit.point.z - center.z).normalized();
      return true;
    }
    return false;
  }
};
