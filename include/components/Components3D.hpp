#pragma once

#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "helper/vectors/Vector3.hpp"
#include "nodes/3D/AABB.hpp"
#include "nodes/3D/Frustum.hpp"
#include "nodes/3D/Transform3D.hpp"
#include <array>
#include <entt/entt.hpp>
#include <string>
#include <vector>

// 3D Vertex Definition (Position, Normal, UV Coordinates, Vertex Color)
struct Vertex3D {
  Vector3 position{0.0f, 0.0f, 0.0f};
  Vector3 normal{0.0f, 1.0f, 0.0f};
  Vector2 uv{0.0f, 0.0f};
  Color color{1.0f, 1.0f, 1.0f, 1.0f};

  constexpr Vertex3D() = default;
  constexpr Vertex3D(const Vector3 &pos, const Vector3 &norm, const Vector2 &uv, const Color &col = Color::WHITE)
      : position(pos), normal(norm), uv(uv), color(col) {}
};

// 3D Spatial Transform Component
struct Transform3DComponent {
  Transform3D localTransform;
  Transform3D globalTransform;
  bool isDirty = true;
  entt::entity parent = entt::null;

  Transform3DComponent() = default;
  explicit Transform3DComponent(const Transform3D &transform)
      : localTransform(transform), globalTransform(transform) {}
};

// 3D Mesh Geometry Component
struct Mesh3DComponent {
  std::vector<Vertex3D> vertices;
  std::vector<uint32_t> indices;
  AABB aabb;
  Color albedoColor = Color::WHITE;
  float roughness = 0.5f;
  float metallic = 0.0f;
  bool castShadow = true;
  bool visible = true;
  bool cullBackfaces = true; // By default cull back-faces for solid 3D geometry

  Mesh3DComponent() = default;
  Mesh3DComponent(std::vector<Vertex3D> verts, std::vector<uint32_t> inds, const AABB &box)
      : vertices(std::move(verts)), indices(std::move(inds)), aabb(box) {}
};

// 3D Camera Component
struct Camera3DComponent {
  enum class ProjectionType {
    Perspective,
    Orthographic
  };

  ProjectionType projectionType = ProjectionType::Perspective;
  float fov = 75.0f; // in degrees
  float nearPlane = 0.05f;
  float farPlane = 4000.0f;
  float orthoSize = 10.0f;
  bool isCurrent = false;

  std::array<float, 16> viewMatrix{};
  std::array<float, 16> projectionMatrix{};
  std::array<float, 16> viewProjectionMatrix{};
  Frustum frustum;

  Camera3DComponent() = default;
};

// 3D Directional Light Component
struct DirectionalLight3DComponent {
  Color color = Color::WHITE;
  float energy = 1.0f;
  Vector3 direction{-0.5f, -1.0f, -0.5f};
  bool castShadows = true;

  DirectionalLight3DComponent() = default;
};

// 3D Point Light Component
struct PointLight3DComponent {
  Color color = Color::WHITE;
  float energy = 1.0f;
  float range = 10.0f;
  float attenuation = 1.0f;

  PointLight3DComponent() = default;
};
