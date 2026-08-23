#pragma once

#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include "window.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

// 2D Camera Node (inspired by Godot Camera2D) supporting viewport transforms, coordinate mapping, and active camera tracking.
class Camera2D : public Node2D {
public:
  enum class AnchorMode {
    FixedTopLeft = 0,
    DragCenter = 1
  };

  AnchorMode anchorMode = AnchorMode::DragCenter; // Godot default: centers viewport on camera/player
  Vector2 offset{0.0f, 0.0f};                    // Additional screen offset
  float zoom = 1.0f;                             // Zoom scale factor (1.0 = 100%, 2.0 = 200% zoom-in)
  bool enabled = true;                           // Whether camera is active when current

  // Viewport / World limits
  float limitLeft = -10000000.0f;
  float limitTop = -10000000.0f;
  float limitRight = 10000000.0f;
  float limitBottom = 10000000.0f;

  Camera2D() : Node2D("Camera2D") {}

  explicit Camera2D(std::string name) : Node2D(std::move(name)) {}

  Camera2D(const Vector2 &targetPos, const Vector2 &screenOffset, float zoomLevel = 1.0f)
      : Node2D("Camera2D"), offset(screenOffset), zoom(zoomLevel) {
    transform.position = targetPos;
  }

  ~Camera2D() override {
    if (s_currentCamera == this) {
      s_currentCamera = nullptr;
    }
  }

  void onDestroy() override {
    if (s_currentCamera == this) {
      s_currentCamera = nullptr;
    }
  }

  static void clearCurrentCamera() {
    s_currentCamera = nullptr;
  }


  // Makes this camera the active/current camera for 2D scene rendering.
  void makeCurrent() {
    s_currentCamera = this;
    enabled = true;
  }

  // Godot-style snake_case alias
  void make_current() { makeCurrent(); }

  // Returns true if this camera is currently the active camera.
  bool isCurrent() const {
    return s_currentCamera == this && enabled;
  }

  // Godot-style snake_case alias
  bool is_current() const { return isCurrent(); }

  // Clears this camera from being current.
  void clearCurrent() {
    if (s_currentCamera == this) {
      s_currentCamera = nullptr;
    }
  }

  // Godot-style snake_case alias
  void clear_current() { clearCurrent(); }

  // Sets whether the camera is enabled.
  void setEnabled(bool isEnabled) { enabled = isEnabled; }
  bool isEnabled() const { return enabled; }

  // Sets camera zoom level (e.g. 1.0 = normal, 2.0 = 2x zoom).
  void setZoom(float zoomLevel) { zoom = zoomLevel; }
  float getZoom() const { return zoom; }

  // Sets screen offset.
  void setOffset(const Vector2 &off) { offset = off; }
  const Vector2 &getOffset() const { return offset; }

  // Sets anchor mode (FixedTopLeft or DragCenter).
  void setAnchorMode(AnchorMode mode) { anchorMode = mode; }
  AnchorMode getAnchorMode() const { return anchorMode; }

  // Sets viewport limits.
  void setLimit(float left, float top, float right, float bottom) {
    limitLeft = left;
    limitTop = top;
    limitRight = right;
    limitBottom = bottom;
  }

  // Returns rotation of camera.
  float getRotation() const { return transform.rotation; }

  // Returns the effective screen-space center offset based on anchor mode.
  Vector2 getEffectiveOffset() const {
    if (anchorMode == AnchorMode::DragCenter) {
      return Window::getViewportCenter() + offset;
    }
    return offset;
  }

  // Returns the clamped camera center position respecting viewport limits.
  Vector2 getClampedPosition() const {
    Vector2 pos = getGlobalPosition();
    pos.x = std::clamp(pos.x, limitLeft, limitRight);
    pos.y = std::clamp(pos.y, limitTop, limitBottom);
    return pos;
  }

  // Converts a screen-space pixel coordinate (e.g. mouse position) to world-space coordinates.
  Vector2 screenToWorld(const Vector2 &screenPos) const {
    Vector2 effOffset = getEffectiveOffset();
    Vector2 res = screenPos - effOffset;
    float rot = transform.rotation;
    if (rot != 0.0f) {
      res = res.rotated(-rot);
    }
    if (zoom != 0.0f) {
      res = res / zoom;
    }
    return res + getClampedPosition();
  }

  // Converts a world-space coordinate to screen-space pixel coordinates.
  Vector2 worldToScreen(const Vector2 &worldPos) const {
    Vector2 effOffset = getEffectiveOffset();
    Vector2 res = worldPos - getClampedPosition();
    if (zoom != 0.0f) {
      res = res * zoom;
    }
    float rot = transform.rotation;
    if (rot != 0.0f) {
      res = res.rotated(rot);
    }
    return res + effOffset;
  }

  // Returns pointer to active current camera or nullptr if none.
  static Camera2D *getCurrent() {
    return (s_currentCamera && s_currentCamera->enabled) ? s_currentCamera : nullptr;
  }

  // Godot-style snake_case alias
  static Camera2D *get_current() { return getCurrent(); }

  // Explicitly sets or unsets the active current camera.
  static void setCurrent(Camera2D *camera) {
    s_currentCamera = camera;
  }

private:
  inline static Camera2D *s_currentCamera = nullptr;
};
