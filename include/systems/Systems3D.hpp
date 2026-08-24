#pragma once

#include "ECS.hpp"
#include "components/Components3D.hpp"
#include "nodes/3D/Frustum.hpp"
#include <algorithm>
#include <array>
#include <cmath>

class Systems3D {
public:
  // Updates global 3D transforms across all spatial entities in the ECS registry
  static void updateTransforms() {
    auto view = Entity::getRegistry().view<Transform3DComponent>();

    auto getGlobal = [&](auto self, entt::entity e) -> Transform3D {
      if (e == entt::null || !Entity::getRegistry().valid(e)) return Transform3D();
      auto *c = Entity::getRegistry().try_get<Transform3DComponent>(e);
      if (!c) return Transform3D();
      if (c->parent != entt::null && Entity::getRegistry().valid(c->parent)) {
        return self(self, c->parent) * c->localTransform;
      }
      return c->localTransform;
    };

    for (auto entity : view) {
      auto &comp = view.get<Transform3DComponent>(entity);
      comp.globalTransform = getGlobal(getGlobal, entity);
    }
  }

  // Updates active Camera3D view, projection, and frustum matrices
  static void updateCameras(float viewportWidth, float viewportHeight) {
    if (viewportWidth <= 0.0f || viewportHeight <= 0.0f) return;
    float aspect = viewportWidth / viewportHeight;

    auto view = Entity::getRegistry().view<Camera3DComponent, Transform3DComponent>();
    for (auto entity : view) {
      auto &cam = view.get<Camera3DComponent>(entity);
      auto &trans = view.get<Transform3DComponent>(entity);

      // 1. View Matrix: Inverse of Camera's Global Transform
      Transform3D inv = trans.globalTransform.affine_inverse();
      cam.viewMatrix = inv.toMatrix4();

      // 2. Projection Matrix (Perspective with Vulkan [0, 1] depth range)
      if (cam.projectionType == Camera3DComponent::ProjectionType::Perspective) {
        float fovRad = cam.fov * 3.14159265f / 180.0f;
        float tanHalfFov = std::tan(fovRad * 0.5f);

        cam.projectionMatrix = {
            1.0f / (aspect * tanHalfFov), 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f / tanHalfFov, 0.0f, 0.0f, // Inverted Y for Vulkan
            0.0f, 0.0f, cam.farPlane / (cam.farPlane - cam.nearPlane), 1.0f,
            0.0f, 0.0f, -(cam.farPlane * cam.nearPlane) / (cam.farPlane - cam.nearPlane), 0.0f
        };
      } else {
        float halfW = cam.orthoSize * aspect * 0.5f;
        float halfH = cam.orthoSize * 0.5f;
        cam.projectionMatrix = {
            1.0f / halfW, 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f / halfH, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f / (cam.farPlane - cam.nearPlane), 0.0f,
            0.0f, 0.0f, -cam.nearPlane / (cam.farPlane - cam.nearPlane), 1.0f
        };
      }

      // 3. Multiply View * Projection Matrix
      cam.viewProjectionMatrix = multiplyMatrices(cam.viewMatrix, cam.projectionMatrix);

      // 4. Extract Frustum
      cam.frustum = Frustum::fromMatrix(cam.viewProjectionMatrix);
    }
  }

private:
  static std::array<float, 16> multiplyMatrices(const std::array<float, 16> &a, const std::array<float, 16> &b) {
    std::array<float, 16> r{};
    for (int row = 0; row < 4; ++row) {
      for (int col = 0; col < 4; ++col) {
        r[col * 4 + row] =
            a[0 * 4 + row] * b[col * 4 + 0] +
            a[1 * 4 + row] * b[col * 4 + 1] +
            a[2 * 4 + row] * b[col * 4 + 2] +
            a[3 * 4 + row] * b[col * 4 + 3];
      }
    }
    return r;
  }
};
