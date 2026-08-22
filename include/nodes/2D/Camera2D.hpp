#pragma once

#include "helper/vectors/Vector2.hpp"
#include <cmath>

// 2D Camera supporting panning, zooming, rotation, and coordinate conversion.
class Camera2D {
public:
  Vector2 position{0.0f, 0.0f}; // Camera center position in world coordinates
  Vector2 offset{0.0f, 0.0f};   // Screen offset (typically screen center {w/2, h/2})
  float zoom = 1.0f;            // Zoom scale factor (1.0 = 100%, 2.0 = 200% zoom-in)
  float rotation = 0.0f;        // Rotation in radians

  Camera2D() = default;

  Camera2D(const Vector2 &target, const Vector2 &screenOffset, float zoomLevel = 1.0f,
           float rotationAngle = 0.0f)
      : position(target), offset(screenOffset), zoom(zoomLevel),
        rotation(rotationAngle) {}

  // Converts a screen-space pixel coordinate (e.g. mouse position) to world-space coordinates.
  Vector2 screenToWorld(const Vector2 &screenPos) const {
    Vector2 res = screenPos - offset;
    if (rotation != 0.0f) {
      res = res.rotated(-rotation);
    }
    if (zoom != 0.0f) {
      res = res / zoom;
    }
    return res + position;
  }

  // Converts a world-space coordinate to screen-space pixel coordinates.
  Vector2 worldToScreen(const Vector2 &worldPos) const {
    Vector2 res = worldPos - position;
    if (zoom != 0.0f) {
      res = res * zoom;
    }
    if (rotation != 0.0f) {
      res = res.rotated(rotation);
    }
    return res + offset;
  }

  // Applies camera transformation to a local vertex for rendering.
  Vector2 transform(const Vector2 &worldPos) const {
    return worldToScreen(worldPos);
  }
};
