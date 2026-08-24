#pragma once

#include "nodes/3D/Node3D.hpp"
#include "nodes/3D/physics/Shape3D.hpp"

// 3D RayCast Node (inspired by Godot RayCast3D)
class RayCast3D : public Node3D {
public:
  Vector3 targetPosition{0.0f, -1.0f, 0.0f};
  bool enabled = true;

  RayCast3D() : Node3D("RayCast3D") {}

  bool isColliding() const { return m_lastHit.hit; }
  Vector3 getCollisionPoint() const { return m_lastHit.point; }
  Vector3 getCollisionNormal() const { return m_lastHit.normal; }
  float getCollisionDistance() const { return m_lastHit.distance; }

  // Casts ray against a specific collision shape with transform
  bool castAgainst(const Shape3D &shape, const Transform3D &shapeTransform) {
    if (!enabled) return false;

    Transform3D global = getGlobalTransform();
    Ray3D ray;
    ray.origin = global.origin;
    Vector3 globalTarget = global.xform(targetPosition);
    Vector3 diff = globalTarget - ray.origin;
    ray.maxDistance = diff.length();
    ray.direction = (ray.maxDistance > 0.0001f) ? (diff / ray.maxDistance) : Vector3(0.0f, -1.0f, 0.0f);

    m_lastHit = {};
    return shape.intersectsRay(ray, shapeTransform, m_lastHit);
  }

private:
  RayCastHit3D m_lastHit{};
};
