#pragma once

#include "core/Signal.hpp"
#include "nodes/3D/physics/BoxShape3D.hpp"
#include "nodes/3D/physics/CollisionObject3D.hpp"
#include "nodes/3D/physics/SphereShape3D.hpp"
#include <algorithm>
#include <unordered_set>
#include <vector>

// 3D Trigger & Influence Area in Godot 4 (Area3D)
class Area3D : public CollisionObject3D {
public:
  bool monitoring = true;
  bool monitorable = true;
  float gravity = 9.8f;
  Vector3 gravityDirection{0.0f, -1.0f, 0.0f};

  // Godot 4 Standard Area3D Signals
  Signal<Node3D *> body_entered;
  Signal<Node3D *> body_exited;
  Signal<Area3D *> area_entered;
  Signal<Area3D *> area_exited;

  Area3D() : CollisionObject3D("Area3D") {}
  explicit Area3D(std::string name) : CollisionObject3D(std::move(name)) {}

  ~Area3D() override {
    m_overlappingBodies.clear();
  }

  // Returns list of currently overlapping bodies
  const std::unordered_set<Node3D *> &getOverlappingBodies() const {
    return m_overlappingBodies;
  }

  bool hasOverlappingBody(Node3D *body) const {
    return m_overlappingBodies.find(body) != m_overlappingBodies.end();
  }

  // Tests if an external node (e.g. CharacterBody3D) overlaps this Area's collision shape / bounds
  bool overlapsNode(Node3D *other) const {
    if (!other || !monitoring) return false;

    Vector3 myPos = getGlobalPosition();
    Vector3 otherPos = other->getGlobalPosition();

    // Default trigger radius / box check based on child collision shapes
    auto shapes = getShapes();
    float extents = 1.5f;
    if (!shapes.empty() && shapes[0]) {
      if (auto *box = dynamic_cast<BoxShape3D *>(shapes[0].get())) {
        Vector3 sz = box->size * 0.5f;
        Vector3 diff = otherPos - myPos;
        return (std::abs(diff.x) <= sz.x + 0.4f && std::abs(diff.y) <= sz.y + 1.2f && std::abs(diff.z) <= sz.z + 0.4f);
      } else if (auto *sphere = dynamic_cast<SphereShape3D *>(shapes[0].get())) {
        float r = sphere->radius + 0.4f;
        Vector3 diff = otherPos - myPos;
        return (diff.length_squared() <= r * r);
      }
    }

    Vector3 diff = otherPos - myPos;
    return (std::abs(diff.x) <= extents && std::abs(diff.y) <= extents && std::abs(diff.z) <= extents);
  }

  // Called by physics server or scene update to synchronize overlap signals
  void updateOverlaps(const std::vector<Node3D *> &candidates) {
    if (!monitoring) return;

    std::unordered_set<Node3D *> currentOverlaps;

    for (Node3D *cand : candidates) {
      if (!cand || cand == this) continue;
      if (overlapsNode(cand)) {
        currentOverlaps.insert(cand);
        if (m_overlappingBodies.find(cand) == m_overlappingBodies.end()) {
          m_overlappingBodies.insert(cand);
          body_entered.emit(cand);
        }
      }
    }

    // Check for bodies that exited
    for (auto it = m_overlappingBodies.begin(); it != m_overlappingBodies.end();) {
      if (currentOverlaps.find(*it) == currentOverlaps.end()) {
        Node3D *exitedBody = *it;
        it = m_overlappingBodies.erase(it);
        body_exited.emit(exitedBody);
      } else {
        ++it;
      }
    }
  }

private:
  std::unordered_set<Node3D *> m_overlappingBodies;
};
