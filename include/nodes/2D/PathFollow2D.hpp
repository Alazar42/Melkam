#pragma once

#include "core/Memory.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/Path2D.hpp"
#include <cmath>

// Automatic Spline-Following 2D Spatial Node (inspired by Godot PathFollow2D)
class PathFollow2D : public Node2D {
public:
  float progress = 0.0f;       // Distance along curve in pixels
  float speed = 120.0f;        // Automatic movement speed along curve (pixels/second)
  float hOffset = 0.0f;        // Horizontal offset perpendicular to curve
  float vOffset = 0.0f;        // Vertical offset along curve
  bool rotates = true;         // Auto-align node rotation with curve tangent
  bool loop = true;            // Loop back to beginning when reaching end (one-way)
  bool pingPong = false;       // Ping-pong back and forth between endpoints

  PathFollow2D() : Node2D("PathFollow2D") {}

  explicit PathFollow2D(float moveSpeed, bool isPingPong = false)
      : Node2D("PathFollow2D"), speed(moveSpeed), pingPong(isPingPong) {}

  void setProgressRatio(float ratio) {
    auto *parentPath = dynamic_cast<Path2D *>(getParent());
    if (parentPath && parentPath->curve) {
      progress = ratio * parentPath->curve->getBakedLength();
      updateTransformFromCurve();
    }
  }

  float getProgressRatio() const {
    const auto *parentPath = dynamic_cast<const Path2D *>(getParent());
    if (parentPath && parentPath->curve) {
      float len = parentPath->curve->getBakedLength();
      return (len > 0.0001f) ? (progress / len) : 0.0f;
    }
    return 0.0f;
  }

  void onPhysicsProcess(float delta) override {
    auto *parentPath = dynamic_cast<Path2D *>(getParent());
    if (speed != 0.0f && parentPath && parentPath->curve) {
      float totalLen = parentPath->curve->getBakedLength();
      progress += speed * delta;

      if (pingPong && totalLen > 0.0f) {
        if (progress >= totalLen) {
          progress = totalLen;
          speed = -std::abs(speed);
        } else if (progress <= 0.0f) {
          progress = 0.0f;
          speed = std::abs(speed);
        }
      }
    }
    updateTransformFromCurve();
  }

  void updateTransformFromCurve() {
    auto *parentPath = dynamic_cast<Path2D *>(getParent());
    if (!parentPath || !parentPath->curve || parentPath->curve->getPointCount() < 2) return;

    float rotRad = 0.0f;
    bool shouldLoop = loop && !pingPong;
    Vector2 basePos = parentPath->curve->sampleBakedWithRotation(progress + vOffset, rotRad, shouldLoop);

    if (hOffset != 0.0f) {
      Vector2 normal = Vector2(-std::sin(rotRad), std::cos(rotRad));
      basePos += normal * hOffset;
    }

    setPosition(basePos);
    if (rotates) {
      setRotation(rotRad);
    }
  }
};
