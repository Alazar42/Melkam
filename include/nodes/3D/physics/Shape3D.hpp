#pragma once

#include "helper/vectors/Vector3.hpp"
#include "nodes/3D/AABB.hpp"
#include "nodes/3D/Transform3D.hpp"
#include <optional>

struct Ray3D {
  Vector3 origin{0.0f, 0.0f, 0.0f};
  Vector3 direction{0.0f, 0.0f, -1.0f};
  float maxDistance = 1000.0f;
};

struct RayCastHit3D {
  bool hit = false;
  float distance = 0.0f;
  Vector3 point{0.0f, 0.0f, 0.0f};
  Vector3 normal{0.0f, 1.0f, 0.0f};
};

// Base 3D Collision Shape (inspired by Godot Shape3D)
class Shape3D {
public:
  virtual ~Shape3D() = default;
  virtual AABB getAABB() const = 0;
  virtual bool intersectsRay(const Ray3D &ray, const Transform3D &transform, RayCastHit3D &outHit) const = 0;
};
