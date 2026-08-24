#pragma once

#include "helper/vectors/Vector3.hpp"
#include <algorithm>

// Godot-Standard 3D Axis-Aligned Bounding Box
class AABB {
public:
  Vector3 position{0.0f, 0.0f, 0.0f};
  Vector3 size{0.0f, 0.0f, 0.0f};

  constexpr AABB() = default;
  constexpr AABB(const Vector3 &pos, const Vector3 &sz) : position(pos), size(sz) {}

  Vector3 get_end() const { return position + size; }
  Vector3 get_center() const { return position + size * 0.5f; }

  bool has_point(const Vector3 &point) const {
    if (point.x < position.x || point.y < position.y || point.z < position.z) return false;
    if (point.x > position.x + size.x || point.y > position.y + size.y || point.z > position.z + size.z) return false;
    return true;
  }

  bool intersects(const AABB &other) const {
    if (position.x >= (other.position.x + other.size.x)) return false;
    if ((position.x + size.x) <= other.position.x) return false;
    if (position.y >= (other.position.y + other.size.y)) return false;
    if ((position.y + size.y) <= other.position.y) return false;
    if (position.z >= (other.position.z + other.size.z)) return false;
    if ((position.z + size.z) <= other.position.z) return false;
    return true;
  }

  bool encloses(const AABB &other) const {
    Vector3 src_min = position;
    Vector3 src_max = position + size;
    Vector3 dst_min = other.position;
    Vector3 dst_max = other.position + other.size;

    return dst_min.x >= src_min.x && dst_max.x <= src_max.x &&
           dst_min.y >= src_min.y && dst_max.y <= src_max.y &&
           dst_min.z >= src_min.z && dst_max.z <= src_max.z;
  }

  AABB merge(const AABB &other) const {
    Vector3 min_pos(
        std::min(position.x, other.position.x),
        std::min(position.y, other.position.y),
        std::min(position.z, other.position.z));

    Vector3 max_pos(
        std::max(position.x + size.x, other.position.x + other.size.x),
        std::max(position.y + size.y, other.position.y + other.size.y),
        std::max(position.z + size.z, other.position.z + other.size.z));

    return AABB(min_pos, max_pos - min_pos);
  }

  AABB grow(float by) const {
    return AABB(
        position - Vector3(by, by, by),
        size + Vector3(by * 2.0f, by * 2.0f, by * 2.0f));
  }
};
