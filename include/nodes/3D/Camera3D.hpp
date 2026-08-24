#pragma once

#include "nodes/3D/Node3D.hpp"

// 3D Camera Node (inspired by Godot Camera3D)
class Camera3D : public Node3D {
public:
  inline static Camera3D *s_currentCamera = nullptr;

  float fov = 75.0f; // in degrees
  float nearPlane = 0.05f;
  float farPlane = 4000.0f;
  bool isOrthographic = false;
  float orthoSize = 10.0f;

  Camera3D() : Node3D("Camera3D") {
    initCameraECS();
  }

  explicit Camera3D(std::string nodeName) : Node3D(std::move(nodeName)) {
    initCameraECS();
  }

  ~Camera3D() override {
    if (s_currentCamera == this) {
      s_currentCamera = nullptr;
    }
  }

  void makeCurrent() {
    s_currentCamera = this;
    if (m_entity.isValid()) {
      auto &comp = m_entity.getOrAddComponent<Camera3DComponent>();
      comp.isCurrent = true;
    }
  }

  static Camera3D *getCurrent() { return s_currentCamera; }
  static void clearCurrentCamera() { s_currentCamera = nullptr; }

  bool isCurrent() const { return s_currentCamera == this; }

  void onReady() override {
    if (!s_currentCamera) {
      makeCurrent();
    }
  }

  void onProcess(float delta) override {
    (void)delta;
    if (m_entity.isValid()) {
      auto &comp = m_entity.getOrAddComponent<Camera3DComponent>();
      comp.fov = fov;
      comp.nearPlane = nearPlane;
      comp.farPlane = farPlane;
      comp.orthoSize = orthoSize;
      comp.projectionType = isOrthographic ? Camera3DComponent::ProjectionType::Orthographic
                                           : Camera3DComponent::ProjectionType::Perspective;
    }
  }

  // Returns viewing Frustum for culling
  Frustum getFrustum() const {
    if (m_entity.isValid() && m_entity.hasComponent<Camera3DComponent>()) {
      return m_entity.getComponent<Camera3DComponent>().frustum;
    }
    return Frustum();
  }

private:
  void initCameraECS() {
    auto &comp = m_entity.getOrAddComponent<Camera3DComponent>();
    comp.fov = fov;
    comp.nearPlane = nearPlane;
    comp.farPlane = farPlane;
  }
};
