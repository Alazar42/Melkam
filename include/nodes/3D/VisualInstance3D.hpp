#pragma once

#include "nodes/3D/AABB.hpp"
#include "nodes/3D/Node3D.hpp"
#include <cstdint>

// Base class for all visual 3D entities in Godot 4
class VisualInstance3D : public Node3D {
public:
  uint32_t layers = 1;
  float sortingOffset = 0.0f;
  bool visible = true;

  VisualInstance3D() : Node3D("VisualInstance3D") {}
  explicit VisualInstance3D(std::string name) : Node3D(std::move(name)) {}

  virtual AABB getAABB() const { return AABB(); }

  void setLayerMask(uint32_t mask) { layers = mask; }
  uint32_t getLayerMask() const { return layers; }
  void setLayerMaskValue(int layerNumber, bool value) {
    if (layerNumber < 1 || layerNumber > 32) return;
    uint32_t mask = 1u << (layerNumber - 1);
    if (value) layers |= mask;
    else layers &= ~mask;
  }
  bool getLayerMaskValue(int layerNumber) const {
    if (layerNumber < 1 || layerNumber > 32) return false;
    return (layers & (1u << (layerNumber - 1))) != 0;
  }
};
