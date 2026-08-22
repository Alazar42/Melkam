#pragma once

#include "helper/vectors/Vector2.hpp"
#include <algorithm>

// 2D Axis-aligned bounding box / rectangle (inspired by Godot Rect2).
struct Rect2 {
  Vector2 position{0.0f, 0.0f};
  Vector2 size{0.0f, 0.0f};

  constexpr Rect2() = default;
  constexpr Rect2(float x, float y, float width, float height)
      : position(x, y), size(width, height) {}
  constexpr Rect2(const Vector2 &pos, const Vector2 &sz)
      : position(pos), size(sz) {}

  // Component accessors
  float getX() const { return position.x; }
  float getY() const { return position.y; }
  float getWidth() const { return size.x; }
  float getHeight() const { return size.y; }

  Vector2 getEnd() const { return position + size; }
  Vector2 getCenter() const { return position + size * 0.5f; }

  // Area
  float getArea() const { return size.x * size.y; }
  bool hasArea() const { return size.x > 0.0f && size.y > 0.0f; }

  // Overlap and point checks
  bool hasPoint(const Vector2 &point) const {
    if (point.x < position.x || point.x > (position.x + size.x)) return false;
    if (point.y < position.y || point.y > (position.y + size.y)) return false;
    return true;
  }

  bool intersects(const Rect2 &other) const {
    if (position.x >= (other.position.x + other.size.x)) return false;
    if ((position.x + size.x) <= other.position.x) return false;
    if (position.y >= (other.position.y + other.size.y)) return false;
    if ((position.y + size.y) <= other.position.y) return false;
    return true;
  }

  Rect2 intersection(const Rect2 &other) const {
    if (!intersects(other)) return Rect2();
    float newX = std::max(position.x, other.position.x);
    float newY = std::max(position.y, other.position.y);
    float newW = std::min(position.x + size.x, other.position.x + other.size.x) - newX;
    float newH = std::min(position.y + size.y, other.position.y + other.size.y) - newY;
    return Rect2(newX, newY, newW, newH);
  }

  Rect2 merge(const Rect2 &other) const {
    float minX = std::min(position.x, other.position.x);
    float minY = std::min(position.y, other.position.y);
    float maxX = std::max(position.x + size.x, other.position.x + other.size.x);
    float maxY = std::max(position.y + size.y, other.position.y + other.size.y);
    return Rect2(minX, minY, maxX - minX, maxY - minY);
  }

  bool operator==(const Rect2 &other) const {
    return position == other.position && size == other.size;
  }

  bool operator!=(const Rect2 &other) const {
    return !(*this == other);
  }
};
