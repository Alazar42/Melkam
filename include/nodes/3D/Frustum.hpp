#pragma once

#include "nodes/3D/AABB.hpp"
#include "nodes/3D/Plane.hpp"
#include <array>

// 6-Plane Camera Viewing Frustum for 3D culling
class Frustum {
public:
  enum PlaneSide {
    Near = 0,
    Far = 1,
    Left = 2,
    Right = 3,
    Top = 4,
    Bottom = 5
  };

  Plane planes[6];

  constexpr Frustum() = default;

  // Extracts the 6 frustum planes from a column-major 4x4 View-Projection matrix
  static Frustum fromMatrix(const std::array<float, 16> &m) {
    Frustum f;

    // Left Plane: row4 + row1
    f.planes[Left] = Plane(
        Vector3(m[3] + m[0], m[7] + m[4], m[11] + m[8]),
        -(m[15] + m[12])).normalized();

    // Right Plane: row4 - row1
    f.planes[Right] = Plane(
        Vector3(m[3] - m[0], m[7] - m[4], m[11] - m[8]),
        -(m[15] - m[12])).normalized();

    // Bottom Plane: row4 + row2
    f.planes[Bottom] = Plane(
        Vector3(m[3] + m[1], m[7] + m[5], m[11] + m[9]),
        -(m[15] + m[13])).normalized();

    // Top Plane: row4 - row2
    f.planes[Top] = Plane(
        Vector3(m[3] - m[1], m[7] - m[5], m[11] - m[9]),
        -(m[15] - m[13])).normalized();

    // Near Plane: row4 + row3 (or row3 for Vulkan [0, 1] clip space)
    f.planes[Near] = Plane(
        Vector3(m[3] + m[2], m[7] + m[6], m[11] + m[10]),
        -(m[15] + m[14])).normalized();

    // Far Plane: row4 - row3
    f.planes[Far] = Plane(
        Vector3(m[3] - m[2], m[7] - m[6], m[11] - m[10]),
        -(m[15] - m[14])).normalized();

    return f;
  }

  // Tests if an AABB is partially or fully inside this viewing frustum
  bool intersectsAABB(const AABB &aabb) const {
    Vector3 min = aabb.position;
    Vector3 max = aabb.position + aabb.size;

    for (int i = 0; i < 6; ++i) {
      const Plane &p = planes[i];

      // Find the positive vertex along the normal
      Vector3 p_vertex(
          p.normal.x > 0 ? max.x : min.x,
          p.normal.y > 0 ? max.y : min.y,
          p.normal.z > 0 ? max.z : min.z);

      if (p.distance_to(p_vertex) < 0.0f) {
        return false; // AABB is entirely behind this plane
      }
    }
    return true;
  }
};
