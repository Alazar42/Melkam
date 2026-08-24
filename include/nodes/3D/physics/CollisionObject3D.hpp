#pragma once

#include "nodes/3D/Node3D.hpp"
#include "nodes/3D/physics/CollisionShape3D.hpp"
#include <cstdint>
#include <vector>

// Base class for 3D physics bodies and areas (inspired by Godot CollisionObject3D)
class CollisionObject3D : public Node3D {
public:
  uint32_t collisionLayer = 1;
  uint32_t collisionMask = 1;
  bool inputRayPickable = true;

  CollisionObject3D() : Node3D("CollisionObject3D") {}
  explicit CollisionObject3D(std::string name) : Node3D(std::move(name)) {}

  void setCollisionLayer(uint32_t layer) { collisionLayer = layer; }
  uint32_t getCollisionLayer() const { return collisionLayer; }

  void setCollisionMask(uint32_t mask) { collisionMask = mask; }
  uint32_t getCollisionMask() const { return collisionMask; }

  void setCollisionLayerValue(int layerNumber, bool value) {
    if (layerNumber < 1 || layerNumber > 32) return;
    uint32_t mask = 1u << (layerNumber - 1);
    if (value) collisionLayer |= mask;
    else collisionLayer &= ~mask;
  }

  bool getCollisionLayerValue(int layerNumber) const {
    if (layerNumber < 1 || layerNumber > 32) return false;
    return (collisionLayer & (1u << (layerNumber - 1))) != 0;
  }

  void setCollisionMaskValue(int layerNumber, bool value) {
    if (layerNumber < 1 || layerNumber > 32) return;
    uint32_t mask = 1u << (layerNumber - 1);
    if (value) collisionMask |= mask;
    else collisionMask &= ~mask;
  }

  bool getCollisionMaskValue(int layerNumber) const {
    if (layerNumber < 1 || layerNumber > 32) return false;
    return (collisionMask & (1u << (layerNumber - 1))) != 0;
  }

  // Returns all child CollisionShape3D shapes attached to this body
  std::vector<Ref<Shape3D>> getShapes() const {
    std::vector<Ref<Shape3D>> shapes;
    for (const auto &child : getChildren()) {
      if (auto *cs = dynamic_cast<CollisionShape3D *>(child.get())) {
        if (!cs->disabled && cs->shape) {
          shapes.push_back(cs->shape);
        }
      }
    }
    return shapes;
  }

  virtual void rebuildBulletBody() {}
};

inline void CollisionShape3D::notifyParentRebuild() {
  if (auto *obj = dynamic_cast<CollisionObject3D *>(getParent())) {
    obj->rebuildBulletBody();
  }
}
