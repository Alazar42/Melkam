#pragma once

#include "components/Components3D.hpp"
#include "nodes/3D/Node3D.hpp"
#include <cmath>
#include <vector>

// 3D Mesh Geometry Generator & Procedural Primitives
class Mesh3D {
public:
  std::vector<Vertex3D> vertices;
  std::vector<uint32_t> indices;
  AABB aabb;

  // Generates a 3D Box / Cube Mesh with correct vertex normals and UVs
  static Mesh3D createBox(const Vector3 &size = Vector3(1.0f, 1.0f, 1.0f)) {
    Mesh3D mesh;
    Vector3 h = size * 0.5f;

    mesh.aabb = AABB(-h, size);

    // Front Face (+Z)
    mesh.vertices.push_back({Vector3(-h.x, -h.y,  h.z), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 1.0f)});
    mesh.vertices.push_back({Vector3( h.x, -h.y,  h.z), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f)});
    mesh.vertices.push_back({Vector3( h.x,  h.y,  h.z), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 0.0f)});
    mesh.vertices.push_back({Vector3(-h.x,  h.y,  h.z), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 0.0f)});

    // Back Face (-Z)
    mesh.vertices.push_back({Vector3( h.x, -h.y, -h.z), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 1.0f)});
    mesh.vertices.push_back({Vector3(-h.x, -h.y, -h.z), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 1.0f)});
    mesh.vertices.push_back({Vector3(-h.x,  h.y, -h.z), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 0.0f)});
    mesh.vertices.push_back({Vector3( h.x,  h.y, -h.z), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 0.0f)});

    // Top Face (+Y)
    mesh.vertices.push_back({Vector3(-h.x,  h.y,  h.z), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f)});
    mesh.vertices.push_back({Vector3( h.x,  h.y,  h.z), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f)});
    mesh.vertices.push_back({Vector3( h.x,  h.y, -h.z), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f)});
    mesh.vertices.push_back({Vector3(-h.x,  h.y, -h.z), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)});

    // Bottom Face (-Y)
    mesh.vertices.push_back({Vector3(-h.x, -h.y, -h.z), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f)});
    mesh.vertices.push_back({Vector3( h.x, -h.y, -h.z), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f)});
    mesh.vertices.push_back({Vector3( h.x, -h.y,  h.z), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 0.0f)});
    mesh.vertices.push_back({Vector3(-h.x, -h.y,  h.z), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 0.0f)});

    // Right Face (+X)
    mesh.vertices.push_back({Vector3( h.x, -h.y,  h.z), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)});
    mesh.vertices.push_back({Vector3( h.x, -h.y, -h.z), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)});
    mesh.vertices.push_back({Vector3( h.x,  h.y, -h.z), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)});
    mesh.vertices.push_back({Vector3( h.x,  h.y,  h.z), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)});

    // Left Face (-X)
    mesh.vertices.push_back({Vector3(-h.x, -h.y, -h.z), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)});
    mesh.vertices.push_back({Vector3(-h.x, -h.y,  h.z), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)});
    mesh.vertices.push_back({Vector3(-h.x,  h.y,  h.z), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)});
    mesh.vertices.push_back({Vector3(-h.x,  h.y, -h.z), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)});

    for (uint32_t f = 0; f < 6; ++f) {
      uint32_t base = f * 4;
      mesh.indices.push_back(base + 0);
      mesh.indices.push_back(base + 1);
      mesh.indices.push_back(base + 2);
      mesh.indices.push_back(base + 0);
      mesh.indices.push_back(base + 2);
      mesh.indices.push_back(base + 3);
    }

    return mesh;
  }

  // Generates a 3D UV Sphere Mesh
  static Mesh3D createSphere(float radius = 0.5f, uint32_t rings = 16, uint32_t segments = 32) {
    Mesh3D mesh;
    mesh.aabb = AABB(Vector3(-radius, -radius, -radius), Vector3(radius * 2.0f, radius * 2.0f, radius * 2.0f));

    for (uint32_t r = 0; r <= rings; ++r) {
      float v = static_cast<float>(r) / static_cast<float>(rings);
      float phi = v * 3.14159265f;

      for (uint32_t s = 0; s <= segments; ++s) {
        float u = static_cast<float>(s) / static_cast<float>(segments);
        float theta = u * 3.14159265f * 2.0f;

        float x = std::cos(theta) * std::sin(phi);
        float y = std::cos(phi);
        float z = std::sin(theta) * std::sin(phi);

        Vector3 normal(x, y, z);
        Vector3 pos = normal * radius;

        mesh.vertices.push_back({pos, normal, Vector2(u, v)});
      }
    }

    for (uint32_t r = 0; r < rings; ++r) {
      for (uint32_t s = 0; s < segments; ++s) {
        uint32_t i0 = r * (segments + 1) + s;
        uint32_t i1 = (r + 1) * (segments + 1) + s;
        uint32_t i2 = (r + 1) * (segments + 1) + (s + 1);
        uint32_t i3 = r * (segments + 1) + (s + 1);

        mesh.indices.push_back(i0);
        mesh.indices.push_back(i3);
        mesh.indices.push_back(i2);
        mesh.indices.push_back(i0);
        mesh.indices.push_back(i2);
        mesh.indices.push_back(i1);
      }
    }

    return mesh;
  }

  // Generates a 3D Plane Mesh (horizontal grid, double-sided)
  // Godot 4 standard PlaneMesh (size in XZ plane, normal pointing +Y)
  static Mesh3D createPlane(const Vector2 &size = Vector2(8.0f, 8.0f)) {
    Mesh3D mesh;
    float hw = size.x * 0.5f;
    float hd = size.y * 0.5f;
    mesh.aabb = AABB(Vector3(-hw, -0.01f, -hd), Vector3(size.x, 0.02f, size.y));

    mesh.vertices.push_back({Vector3(-hw, 0.0f, -hd), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)}); // 0 (TL)
    mesh.vertices.push_back({Vector3(-hw, 0.0f,  hd), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f)}); // 1 (BL)
    mesh.vertices.push_back({Vector3( hw, 0.0f,  hd), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f)}); // 2 (BR)
    mesh.vertices.push_back({Vector3( hw, 0.0f, -hd), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f)}); // 3 (TR)

    // CCW Top Face
    mesh.indices = {0, 1, 2, 0, 2, 3};
    return mesh;
  }

  // Godot 4 standard CylinderMesh
  static Mesh3D createCylinder(float topRadius = 1.0f, float bottomRadius = 1.0f, float height = 2.0f, uint32_t radialSegments = 16) {
    Mesh3D mesh;
    float hh = height * 0.5f;
    float maxR = std::max(topRadius, bottomRadius);
    mesh.aabb = AABB(Vector3(-maxR, -hh, -maxR), Vector3(maxR * 2.0f, height, maxR * 2.0f));

    // Top and bottom rings
    for (uint32_t i = 0; i < radialSegments; ++i) {
      float theta = static_cast<float>(i) * 6.2831853f / static_cast<float>(radialSegments);
      float nextTheta = static_cast<float>((i + 1) % radialSegments) * 6.2831853f / static_cast<float>(radialSegments);

      float ct = std::cos(theta), st = std::sin(theta);
      float cnt = std::cos(nextTheta), snt = std::sin(nextTheta);

      // Side Quad
      Vector3 t0(ct * topRadius, hh, st * topRadius);
      Vector3 b0(ct * bottomRadius, -hh, st * bottomRadius);
      Vector3 t1(cnt * topRadius, hh, snt * topRadius);
      Vector3 b1(cnt * bottomRadius, -hh, snt * bottomRadius);

      Vector3 n0 = Vector3(ct, 0.0f, st).normalized();
      Vector3 n1 = Vector3(cnt, 0.0f, snt).normalized();

      uint32_t base = static_cast<uint32_t>(mesh.vertices.size());
      mesh.vertices.push_back({b0, n0, Vector2(0.0f, 0.0f)});
      mesh.vertices.push_back({t0, n0, Vector2(0.0f, 1.0f)});
      mesh.vertices.push_back({t1, n1, Vector2(1.0f, 1.0f)});
      mesh.vertices.push_back({b1, n1, Vector2(1.0f, 0.0f)});

      mesh.indices.push_back(base + 0);
      mesh.indices.push_back(base + 1);
      mesh.indices.push_back(base + 2);
      mesh.indices.push_back(base + 0);
      mesh.indices.push_back(base + 2);
      mesh.indices.push_back(base + 3);
    }

    return mesh;
  }
};

// 3D Mesh Instance Node (inspired by Godot MeshInstance3D)
class MeshInstance3D : public Node3D {
public:
  Mesh3D mesh;
  Color albedoColor = Color::WHITE;
  float roughness = 0.5f;
  float metallic = 0.0f;
  bool castShadow = true;
  bool cullBackfaces = true;
  Ref<StandardMaterial3D> material = nullptr;

  MeshInstance3D() : Node3D("MeshInstance3D") {
    setMesh(Mesh3D::createBox());
  }

  explicit MeshInstance3D(const Mesh3D &m) : Node3D("MeshInstance3D") {
    setMesh(m);
  }

  void setMesh(const Mesh3D &m) {
    mesh = m;
    if (m_entity.isValid()) {
      auto &comp = m_entity.getOrAddComponent<Mesh3DComponent>();
      comp.vertices = mesh.vertices;
      comp.indices = mesh.indices;
      comp.aabb = mesh.aabb;
      comp.albedoColor = albedoColor;
      comp.roughness = roughness;
      comp.metallic = metallic;
      comp.cullBackfaces = cullBackfaces;
      comp.material = material;
    }
  }

  void setColor(const Color &col) {
    albedoColor = col;
    if (m_entity.isValid() && m_entity.hasComponent<Mesh3DComponent>()) {
      m_entity.getComponent<Mesh3DComponent>().albedoColor = col;
    }
  }

  void setMaterial(const Ref<StandardMaterial3D> &mat) {
    material = mat;
    if (material) {
      albedoColor = material->albedoColor;
      roughness = material->roughness;
      metallic = material->metallic;
      cullBackfaces = (material->cullMode != CullMode3D::Disabled);
    }
    if (m_entity.isValid() && m_entity.hasComponent<Mesh3DComponent>()) {
      auto &comp = m_entity.getComponent<Mesh3DComponent>();
      comp.material = material;
      comp.albedoColor = albedoColor;
      comp.roughness = roughness;
      comp.metallic = metallic;
      comp.cullBackfaces = cullBackfaces;
    }
  }

  Ref<StandardMaterial3D> getMaterial() const { return material; }

  void setRoughness(float r) {
    roughness = r;
    if (m_entity.isValid() && m_entity.hasComponent<Mesh3DComponent>()) {
      m_entity.getComponent<Mesh3DComponent>().roughness = r;
    }
  }

  void setMetallic(float m) {
    metallic = m;
    if (m_entity.isValid() && m_entity.hasComponent<Mesh3DComponent>()) {
      m_entity.getComponent<Mesh3DComponent>().metallic = m;
    }
  }

  void setCullBackfaces(bool cull) {
    cullBackfaces = cull;
    if (m_entity.isValid() && m_entity.hasComponent<Mesh3DComponent>()) {
      m_entity.getComponent<Mesh3DComponent>().cullBackfaces = cull;
    }
  }

  AABB getAABB() const { return mesh.aabb; }
};
