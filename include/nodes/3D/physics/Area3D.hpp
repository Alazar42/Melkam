#pragma once

#include "core/Signal.hpp"
#include "nodes/3D/physics/CollisionObject3D.hpp"
#include <vector>

// 3D Area / Trigger Volume Node (inspired by Godot Area3D)
class Area3D : public CollisionObject3D {
public:
  Signal<Node3D *> bodyEntered;
  Signal<Node3D *> bodyExited;
  Signal<Area3D *> areaEntered;
  Signal<Area3D *> areaExited;

  bool monitoring = true;
  bool monitorable = true;

  Area3D() : CollisionObject3D("Area3D") {}

  void setMonitoring(bool enable) { monitoring = enable; }
  bool isMonitoring() const { return monitoring; }

  void setMonitorable(bool enable) { monitorable = enable; }
  bool isMonitorable() const { return monitorable; }

  std::vector<Node3D *> getOverlappingBodies() const {
    return m_overlappingBodies;
  }

private:
  std::vector<Node3D *> m_overlappingBodies;
};
