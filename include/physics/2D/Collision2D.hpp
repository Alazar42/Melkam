#pragma once

#include "helper/vectors/Vector2.hpp"
#include <cstdint>

class Node2D;

// Raycast hit result structure
struct RaycastHit2D {
  bool hasHit = false;
  Vector2 point{0.0f, 0.0f};
  Vector2 normal{0.0f, 0.0f};
  float fraction = 0.0f;
  float distance = 0.0f;
  Node2D *collider = nullptr;
};

// Detailed collision manifold and contact information
struct CollisionInfo2D {
  bool hasCollision = false;
  Vector2 normal{0.0f, 0.0f};
  Vector2 contactPoint{0.0f, 0.0f};
  float depth = 0.0f;
  Node2D *collider = nullptr;
  Vector2 remainder{0.0f, 0.0f};
};

// 2D Axis-Aligned Bounding Box (AABB)
struct AABB2D {
  Vector2 position{0.0f, 0.0f}; // Top-left position
  Vector2 size{0.0f, 0.0f};

  AABB2D() = default;
  AABB2D(const Vector2 &pos, const Vector2 &sz) : position(pos), size(sz) {}

  Vector2 getCenter() const { return position + size * 0.5f; }
  Vector2 getMin() const { return position; }
  Vector2 getMax() const { return position + size; }

  bool contains(const Vector2 &point) const {
    return point.x >= position.x && point.x <= position.x + size.x &&
           point.y >= position.y && point.y <= position.y + size.y;
  }

  bool intersects(const AABB2D &other) const {
    return position.x < other.position.x + other.size.x &&
           position.x + size.x > other.position.x &&
           position.y < other.position.y + other.size.y &&
           position.y + size.y > other.position.y;
  }
};
