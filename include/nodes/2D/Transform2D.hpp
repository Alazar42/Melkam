#pragma once

#include "helper/vectors/Vector2.hpp"
#include <cmath>
#include <vector>

// 2D Spatial Transform supporting position, rotation, scale, and parent-child hierarchy.
class Transform2D {
public:
  Vector2 position{0.0f, 0.0f};
  float rotation = 0.0f; // in radians
  Vector2 scale{1.0f, 1.0f};
  int zIndex = 0;

  Transform2D() = default;
  Transform2D(const Vector2 &pos, float rot = 0.0f, const Vector2 &s = {1.0f, 1.0f})
      : position(pos), rotation(rot), scale(s) {}

  // Sets rotation in degrees (converts to radians internally).
  void setRotationDegrees(float degrees) {
    rotation = degrees * (3.14159265358979323846f / 180.0f);
  }

  // Returns rotation in degrees.
  float getRotationDegrees() const {
    return rotation * (180.0f / 3.14159265358979323846f);
  }

  // Translates position by an offset.
  void translate(const Vector2 &offset) {
    position += offset;
  }

  // Rotates by an angle in radians.
  void rotate(float angleRadians) {
    rotation += angleRadians;
  }

  // Transforms a local-space point to world/global space relative to this transform.
  Vector2 transformPoint(const Vector2 &localPoint) const {
    Vector2 scaled = Vector2(localPoint.x * scale.x, localPoint.y * scale.y);
    Vector2 rotated = (rotation != 0.0f) ? scaled.rotated(rotation) : scaled;
    return rotated + position;
  }

  // Computes the combined global transform when parented.
  Transform2D getGlobal(const Transform2D *parent = nullptr) const {
    if (!parent) return *this;

    Transform2D global;
    global.scale = Vector2(scale.x * parent->scale.x, scale.y * parent->scale.y);
    global.rotation = rotation + parent->rotation;
    global.position = parent->transformPoint(position);
    global.zIndex = zIndex + parent->zIndex;
    return global;
  }
};
