#pragma once

#include "input.hpp"
#include "nodes/3D/Camera3D.hpp"
#include <algorithm>
#include <cmath>

enum class Camera3DControlMode {
  Orbit,
  FreeFly
};

// General-purpose Interactive 3D Camera Controller Node (Orbit & Free-fly modes)
class CameraController3D : public Camera3D {
public:
  Camera3DControlMode controlMode = Camera3DControlMode::Orbit;

  // Orbit parameters
  Vector3 targetPosition{0.0f, 0.5f, 0.0f};
  float orbitDistance = 7.0f;
  float minDistance = 1.0f;
  float maxDistance = 50.0f;
  float yaw = 0.0f;
  float pitch = 0.4f;
  float orbitSensitivity = 0.005f;
  float zoomSensitivity = 0.8f;

  // Free-fly parameters
  float moveSpeed = 10.0f;
  float lookSensitivity = 0.003f;

  CameraController3D() : Camera3D("CameraController3D") {
    updateOrbitTransform();
  }

  void onInput(const InputEvent &event) override {
    if (controlMode == Camera3DControlMode::Orbit) {
      if (event.isMouseMotion()) {
        if (Input::isMouseButtonPressed(MouseButton::Left) || Input::isMouseButtonPressed(MouseButton::Right)) {
          yaw -= event.mouseDelta.x * orbitSensitivity;
          pitch -= event.mouseDelta.y * orbitSensitivity;
          pitch = std::clamp(pitch, -1.5f, 1.5f);
          updateOrbitTransform();
        }
      } else if (event.isMouseWheel()) {
        orbitDistance -= event.mouseScroll.y * zoomSensitivity;
        orbitDistance = std::clamp(orbitDistance, minDistance, maxDistance);
        updateOrbitTransform();
      }
    } else if (controlMode == Camera3DControlMode::FreeFly) {
      if (event.isMouseMotion() && Input::isMouseButtonPressed(MouseButton::Right)) {
        yaw -= event.mouseDelta.x * lookSensitivity;
        pitch -= event.mouseDelta.y * lookSensitivity;
        pitch = std::clamp(pitch, -1.5f, 1.5f);
        setRotation(Vector3(pitch, yaw, 0.0f));
      }
    }
  }

  void onProcess(float delta) override {
    Camera3D::onProcess(delta);

    if (controlMode == Camera3DControlMode::FreeFly && Input::isMouseButtonPressed(MouseButton::Right)) {
      Vector3 moveDir{0.0f, 0.0f, 0.0f};
      if (Input::isKeyPressed(Key::W)) moveDir.z -= 1.0f;
      if (Input::isKeyPressed(Key::S)) moveDir.z += 1.0f;
      if (Input::isKeyPressed(Key::A)) moveDir.x -= 1.0f;
      if (Input::isKeyPressed(Key::D)) moveDir.x += 1.0f;
      if (Input::isKeyPressed(Key::Space)) moveDir.y += 1.0f;
      if (Input::isKeyPressed(Key::LShift)) moveDir.y -= 1.0f;

      if (moveDir.length_squared() > 0.001f) {
        moveDir = moveDir.normalized();
        translateLocal(moveDir * moveSpeed * delta);
      }
    }
  }

  void updateOrbitTransform() {
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);

    Vector3 offset(sy * cp * orbitDistance, sp * orbitDistance, cy * cp * orbitDistance);
    setPosition(targetPosition + offset);
    lookAt(targetPosition, Vector3(0.0f, 1.0f, 0.0f));
  }
};
