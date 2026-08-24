#pragma once

#include "components/Components3D.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "helper/vectors/Vector3.hpp"
#include "nodes/3D/AABB.hpp"
#include "nodes/3D/StandardMaterial3D.hpp"
#include <cstdint>
#include <vector>

// Base 3D Mesh Resource (inspired by Godot Mesh)
class Mesh {
public:
  std::vector<Vertex3D> vertices;
  std::vector<uint32_t> indices;
  AABB aabb;
  Ref<StandardMaterial3D> material = nullptr;

  virtual ~Mesh() = default;

  virtual AABB getAABB() const { return aabb; }
  virtual void setMaterial(const Ref<StandardMaterial3D> &mat) { material = mat; }
  virtual Ref<StandardMaterial3D> getMaterial() const { return material; }
};
