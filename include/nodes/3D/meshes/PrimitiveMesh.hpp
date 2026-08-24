#pragma once

#include "nodes/3D/meshes/Mesh.hpp"
#include <algorithm>
#include <cmath>
#include <memory>

// Base class for all procedural primitive meshes in Godot 4
class PrimitiveMesh : public Mesh {
public:
  virtual void requestUpdate() = 0;
};

// 1. BoxMesh
class BoxMesh : public PrimitiveMesh {
public:
  Vector3 size{1.0f, 1.0f, 1.0f};

  BoxMesh() { requestUpdate(); }
  explicit BoxMesh(const Vector3 &size) : size(size) { requestUpdate(); }

  void setSize(const Vector3 &s) { size = s; requestUpdate(); }
  Vector3 getSize() const { return size; }

  void requestUpdate() override {
    vertices.clear();
    indices.clear();

    Vector3 h = size * 0.5f;
    aabb = AABB(-h, size);

    // Front (+Z)
    vertices.push_back({Vector3(-h.x, -h.y,  h.z), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 1.0f)});
    vertices.push_back({Vector3( h.x, -h.y,  h.z), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f)});
    vertices.push_back({Vector3( h.x,  h.y,  h.z), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 0.0f)});
    vertices.push_back({Vector3(-h.x,  h.y,  h.z), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 0.0f)});

    // Back (-Z)
    vertices.push_back({Vector3( h.x, -h.y, -h.z), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 1.0f)});
    vertices.push_back({Vector3(-h.x, -h.y, -h.z), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 1.0f)});
    vertices.push_back({Vector3(-h.x,  h.y, -h.z), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 0.0f)});
    vertices.push_back({Vector3( h.x,  h.y, -h.z), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 0.0f)});

    // Top (+Y)
    vertices.push_back({Vector3(-h.x,  h.y,  h.z), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f)});
    vertices.push_back({Vector3( h.x,  h.y,  h.z), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f)});
    vertices.push_back({Vector3( h.x,  h.y, -h.z), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f)});
    vertices.push_back({Vector3(-h.x,  h.y, -h.z), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)});

    // Bottom (-Y)
    vertices.push_back({Vector3(-h.x, -h.y, -h.z), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f)});
    vertices.push_back({Vector3( h.x, -h.y, -h.z), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f)});
    vertices.push_back({Vector3( h.x, -h.y,  h.z), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 0.0f)});
    vertices.push_back({Vector3(-h.x, -h.y,  h.z), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 0.0f)});

    // Right (+X)
    vertices.push_back({Vector3( h.x, -h.y,  h.z), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)});
    vertices.push_back({Vector3( h.x, -h.y, -h.z), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)});
    vertices.push_back({Vector3( h.x,  h.y, -h.z), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)});
    vertices.push_back({Vector3( h.x,  h.y,  h.z), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)});

    // Left (-X)
    vertices.push_back({Vector3(-h.x, -h.y, -h.z), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)});
    vertices.push_back({Vector3(-h.x, -h.y,  h.z), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)});
    vertices.push_back({Vector3(-h.x,  h.y,  h.z), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)});
    vertices.push_back({Vector3(-h.x,  h.y, -h.z), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)});

    for (uint32_t f = 0; f < 6; ++f) {
      uint32_t base = f * 4;
      indices.push_back(base + 0);
      indices.push_back(base + 1);
      indices.push_back(base + 2);
      indices.push_back(base + 0);
      indices.push_back(base + 2);
      indices.push_back(base + 3);
    }
  }

  static Ref<BoxMesh> create(const Vector3 &size = Vector3(1.0f, 1.0f, 1.0f)) {
    return std::make_shared<BoxMesh>(size);
  }
};

// 2. SphereMesh
class SphereMesh : public PrimitiveMesh {
public:
  float radius = 0.5f;
  float height = 1.0f;
  uint32_t radialSegments = 24;
  uint32_t rings = 12;

  SphereMesh() { requestUpdate(); }
  explicit SphereMesh(float r, float h = 1.0f) : radius(r), height(h) { requestUpdate(); }

  void requestUpdate() override {
    vertices.clear();
    indices.clear();

    aabb = AABB(Vector3(-radius, -height * 0.5f, -radius), Vector3(radius * 2.0f, height, radius * 2.0f));

    for (uint32_t r = 0; r <= rings; ++r) {
      float v = static_cast<float>(r) / static_cast<float>(rings);
      float phi = v * 3.1415926535f;

      for (uint32_t s = 0; s <= radialSegments; ++s) {
        float u = static_cast<float>(s) / static_cast<float>(radialSegments);
        float theta = u * 6.283185307f;

        float x = std::sin(phi) * std::cos(theta);
        float y = std::cos(phi);
        float z = std::sin(phi) * std::sin(theta);

        Vector3 norm(x, y, z);
        Vector3 pos(x * radius, y * (height * 0.5f), z * radius);
        vertices.push_back({pos, norm, Vector2(u, v)});
      }
    }

    for (uint32_t r = 0; r < rings; ++r) {
      for (uint32_t s = 0; s < radialSegments; ++s) {
        uint32_t i0 = r * (radialSegments + 1) + s;
        uint32_t i1 = (r + 1) * (radialSegments + 1) + s;
        uint32_t i2 = (r + 1) * (radialSegments + 1) + (s + 1);
        uint32_t i3 = r * (radialSegments + 1) + (s + 1);

        indices.push_back(i0);
        indices.push_back(i3);
        indices.push_back(i2);
        indices.push_back(i0);
        indices.push_back(i2);
        indices.push_back(i1);
      }
    }
  }

  static Ref<SphereMesh> create(float radius = 0.5f) {
    return std::make_shared<SphereMesh>(radius, radius * 2.0f);
  }
};

// 3. PlaneMesh
class PlaneMesh : public PrimitiveMesh {
public:
  Vector2 size{2.0f, 2.0f};

  PlaneMesh() { requestUpdate(); }
  explicit PlaneMesh(const Vector2 &size) : size(size) { requestUpdate(); }

  void requestUpdate() override {
    vertices.clear();
    indices.clear();

    float hw = size.x * 0.5f;
    float hd = size.y * 0.5f;
    aabb = AABB(Vector3(-hw, -0.01f, -hd), Vector3(size.x, 0.02f, size.y));

    vertices.push_back({Vector3(-hw, 0.0f, -hd), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)});
    vertices.push_back({Vector3(-hw, 0.0f,  hd), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f)});
    vertices.push_back({Vector3( hw, 0.0f,  hd), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f)});
    vertices.push_back({Vector3( hw, 0.0f, -hd), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f)});

    indices = {0, 1, 2, 0, 2, 3};
  }

  static Ref<PlaneMesh> create(const Vector2 &size = Vector2(2.0f, 2.0f)) {
    return std::make_shared<PlaneMesh>(size);
  }
};

// 4. CylinderMesh
class CylinderMesh : public PrimitiveMesh {
public:
  float topRadius = 0.5f;
  float bottomRadius = 0.5f;
  float height = 2.0f;
  uint32_t radialSegments = 24;

  CylinderMesh() { requestUpdate(); }
  CylinderMesh(float topR, float botR, float h) : topRadius(topR), bottomRadius(botR), height(h) { requestUpdate(); }

  void requestUpdate() override {
    vertices.clear();
    indices.clear();

    float hh = height * 0.5f;
    float maxR = std::max(topRadius, bottomRadius);
    aabb = AABB(Vector3(-maxR, -hh, -maxR), Vector3(maxR * 2.0f, height, maxR * 2.0f));

    // Top and bottom center vertices
    uint32_t topCenter = static_cast<uint32_t>(vertices.size());
    vertices.push_back({Vector3(0.0f, hh, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.5f, 0.5f)});
    uint32_t botCenter = static_cast<uint32_t>(vertices.size());
    vertices.push_back({Vector3(0.0f, -hh, 0.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.5f, 0.5f)});

    uint32_t ringStart = static_cast<uint32_t>(vertices.size());
    for (uint32_t i = 0; i < radialSegments; ++i) {
      float theta = static_cast<float>(i) * 6.2831853f / static_cast<float>(radialSegments);
      float ct = std::cos(theta), st = std::sin(theta);
      Vector3 norm(ct, 0.0f, st);
      vertices.push_back({Vector3(ct * topRadius, hh, st * topRadius), norm, Vector2(static_cast<float>(i) / radialSegments, 1.0f)});
      vertices.push_back({Vector3(ct * bottomRadius, -hh, st * bottomRadius), norm, Vector2(static_cast<float>(i) / radialSegments, 0.0f)});
    }

    for (uint32_t i = 0; i < radialSegments; ++i) {
      uint32_t next = (i + 1) % radialSegments;
      uint32_t t0 = ringStart + i * 2;
      uint32_t b0 = ringStart + i * 2 + 1;
      uint32_t t1 = ringStart + next * 2;
      uint32_t b1 = ringStart + next * 2 + 1;

      // Top cap triangle
      indices.push_back(topCenter);
      indices.push_back(t0);
      indices.push_back(t1);

      // Bottom cap triangle
      indices.push_back(botCenter);
      indices.push_back(b1);
      indices.push_back(b0);

      // Side quad
      indices.push_back(b0);
      indices.push_back(t0);
      indices.push_back(t1);
      indices.push_back(b0);
      indices.push_back(t1);
      indices.push_back(b1);
    }
  }

  static Ref<CylinderMesh> create(float radius = 0.5f, float height = 2.0f) {
    return std::make_shared<CylinderMesh>(radius, radius, height);
  }
};
