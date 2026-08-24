#pragma once

#include "input.hpp"
#include "nodes/3D/Node3D.hpp"
#include "nodes/3D/physics/PhysicsServer3D.hpp"
#include <algorithm>
#include <cmath>

// 3D Spring Arm / Boom Node (inspired by Godot 4 SpringArm3D)
class SpringArm3D : public Node3D {
public:
  float springLength = 6.0f;
  float margin = 0.25f;
  float minLength = 1.5f;
  float maxLength = 15.0f;
  float currentLength = 6.0f;

  // Orbit rotation (Euler angles in radians)
  float yaw = 0.0f;
  float pitch = -0.35f; // Slight downward look angle by default
  float mouseSensitivity = 0.005f;
  float zoomSensitivity = 0.5f;
  float minPitch = -1.45f; // ~ -83 degrees
  float maxPitch = 0.85f;  // ~ +48 degrees
  bool mouseLookEnabled = true;

  SpringArm3D() : Node3D("SpringArm3D") {
    currentLength = springLength;
    updateRotation();
  }

  void onUnhandledInput(const InputEvent &event) override {
    if (!mouseLookEnabled) return;

    if (event.type == InputEventType::MouseButton) {
      if (event.pressed) {
        if (event.mouseButton == MouseButton::Left || event.mouseButton == MouseButton::Right) {
          m_isDragging = true;
          m_lastMousePos = event.mousePosition;
        }
      } else {
        m_isDragging = false;
      }
    } else if (event.type == InputEventType::MouseMotion) {
      if (m_isDragging) {
        Vector2 delta = event.mouseDelta;
        yaw -= delta.x * mouseSensitivity;
        pitch = std::clamp(pitch - delta.y * mouseSensitivity, minPitch, maxPitch);
        updateRotation();
      }
    } else if (event.type == InputEventType::MouseWheel) {
      springLength = std::clamp(springLength - event.mouseScroll.y * zoomSensitivity, minLength, maxLength);
    }
  }

  void onProcess(float delta) override {
    (void)delta;

    // 1. Raycast against 3D physics environment to prevent clipping into walls/ground
    Vector3 start = getGlobalPosition();
    Vector3 forwardDir = getGlobalTransform().basis.xform(Vector3(0.0f, 0.0f, 1.0f));
    if (forwardDir.length_squared() < 0.001f) forwardDir = Vector3(0.0f, 0.0f, 1.0f);
    else forwardDir = forwardDir.normalized();

    Vector3 end = start + forwardDir * springLength;

    RayCastHit3D hit;
    if ((end - start).length_squared() > 0.01f && PhysicsServer3D::get().raycast(start, end, hit)) {
      currentLength = std::clamp(hit.distance - margin, minLength, springLength);
    } else {
      currentLength = springLength;
    }

    // 2. Position all child nodes (e.g. Camera3D) along the spring boom
    for (const auto &child : getChildren()) {
      if (auto *child3D = dynamic_cast<Node3D *>(child.get())) {
        child3D->setPosition(Vector3(0.0f, 0.0f, currentLength));
      }
    }
  }

  void updateRotation() {
    setRotation(Vector3(pitch, yaw, 0.0f));
  }

  float getHitLength() const { return currentLength; }

private:
  bool m_isDragging = false;
  Vector2 m_lastMousePos;
};
