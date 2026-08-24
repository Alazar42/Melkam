#pragma once

#include "nodes/3D/physics/Shape3D.hpp"
#include <algorithm>
#include <cmath>

// 3D Box Collision Shape (inspired by Godot BoxShape3D)
class BoxShape3D : public Shape3D {
public:
  Vector3 size{1.0f, 1.0f, 1.0f};

  BoxShape3D() = default;
  explicit BoxShape3D(const Vector3 &size) : size(size) {}

  AABB getAABB() const override {
    Vector3 h = size * 0.5f;
    return AABB(-h, size);
  }

  bool intersectsRay(const Ray3D &ray, const Transform3D &transform, RayCastHit3D &outHit) const override {
    Transform3D inv = transform.affine_inverse();
    Vector3 localOrigin = inv.xform(ray.origin);
    Vector3 localDir = inv.basis.xform(ray.direction).normalized();

    Vector3 h = size * 0.5f;
    Vector3 min = -h;
    Vector3 max = h;

    float tmin = 0.0f;
    float tmax = ray.maxDistance;
    Vector3 hitNorm(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < 3; ++i) {
      float o = (i == 0 ? localOrigin.x : (i == 1 ? localOrigin.y : localOrigin.z));
      float d = (i == 0 ? localDir.x : (i == 1 ? localDir.y : localDir.z));
      float minVal = (i == 0 ? min.x : (i == 1 ? min.y : min.z));
      float maxVal = (i == 0 ? max.x : (i == 1 ? max.y : max.z));

      if (std::abs(d) < 0.00001f) {
        if (o < minVal || o > maxVal) return false;
      } else {
        float invD = 1.0f / d;
        float t1 = (minVal - o) * invD;
        float t2 = (maxVal - o) * invD;
        Vector3 n1(i == 0 ? -1.0f : 0.0f, i == 1 ? -1.0f : 0.0f, i == 2 ? -1.0f : 0.0f);
        Vector3 n2(i == 0 ? 1.0f : 0.0f, i == 1 ? 1.0f : 0.0f, i == 2 ? 1.0f : 0.0f);

        if (t1 > t2) {
          std::swap(t1, t2);
          std::swap(n1, n2);
        }

        if (t1 > tmin) {
          tmin = t1;
          hitNorm = n1;
        }
        tmax = std::min(tmax, t2);
        if (tmin > tmax) return false;
      }
    }

    if (tmin >= 0.0f && tmin <= ray.maxDistance) {
      outHit.hit = true;
      outHit.distance = tmin;
      outHit.point = ray.origin + ray.direction * tmin;
      outHit.normal = transform.basis.xform(hitNorm).normalized();
      return true;
    }

    return false;
  }
};
