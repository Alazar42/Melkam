#pragma once

#include "nodes/3D/Node3D.hpp"

// 3D Spatial Position / Anchor Node (inspired by Godot Marker3D / Position3D)
class Marker3D : public Node3D {
public:
  float gizmoExtents = 1.0f;

  Marker3D() : Node3D("Marker3D") {}
  explicit Marker3D(std::string name) : Node3D(std::move(name)) {}
};
