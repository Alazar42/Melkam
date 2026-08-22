#pragma once

#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include <cmath>
#include <string>

// 2D Camera Node (inspired by Godot Camera2D) supporting viewport transforms and coordinate mapping.
class Camera2D : public Node2D {
public:
  Vector2 offset{0.0f, 0.0f};   // Screen offset (typically screen center {w/2, h/2})
  float zoom = 1.0f;            // Zoom scale factor (1.0 = 100%, 2.0 = 200% zoom-in)

  Camera2D() : Node2D("Camera2D") {}

  Camera2D(const Vector2 &targetPos, const Vector2 &screenOffset, float zoomLevel = 1.0f)
      : Node2D("Camera2D"), offset(screenOffset), zoom(zoomLevel) {
    transform.position = targetPos;
  }

  // Returns rotation of camera.
  float getRotation() const { return transform.rotation; }

  // Converts a screen-space pixel coordinate (e.g. mouse position) to world-space coordinates.
  Vector2 screenToWorld(const Vector2 &screenPos) const {
    Vector2 res = screenPos - offset;
    float rot = transform.rotation;
    if (rot != 0.0f) {
      res = res.rotated(-rot);
    }
    if (zoom != 0.0f) {
      res = res / zoom;
    }
    return res + getGlobalPosition();
  }

  // Converts a world-space coordinate to screen-space pixel coordinates.
  Vector2 worldToScreen(const Vector2 &worldPos) const {
    Vector2 res = worldPos - getGlobalPosition();
    if (zoom != 0.0f) {
      res = res * zoom;
    }
    float rot = transform.rotation;
    if (rot != 0.0f) {
      res = res.rotated(rot);
    }
    return res + offset;
  }
};
