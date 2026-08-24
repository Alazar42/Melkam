#pragma once

#include "nodes/3D/Node3D.hpp"
#include "nodes/3D/physics/BoxShape3D.hpp"
#include "nodes/3D/physics/CapsuleShape3D.hpp"
#include "nodes/3D/physics/CylinderShape3D.hpp"
#include "nodes/3D/physics/Shape3D.hpp"
#include "nodes/3D/physics/SphereShape3D.hpp"
#include <memory>

// Node that provides 3D collision shape to CollisionObject3D (inspired by Godot CollisionShape3D)
class CollisionShape3D : public Node3D {
public:
  Ref<Shape3D> shape = nullptr;
  bool disabled = false;

  CollisionShape3D() : Node3D("CollisionShape3D") {
    shape = std::make_shared<BoxShape3D>(Vector3(1.0f, 1.0f, 1.0f));
  }

  explicit CollisionShape3D(Ref<Shape3D> s) : Node3D("CollisionShape3D"), shape(std::move(s)) {}

  void onReady() override {
    Node3D::onReady();
    notifyParentRebuild();
  }

  void setShape(const Ref<Shape3D> &s) {
    shape = s;
    notifyParentRebuild();
  }
  Ref<Shape3D> getShape() const { return shape; }

  void setDisabled(bool dis) {
    disabled = dis;
    notifyParentRebuild();
  }
  bool isDisabled() const { return disabled; }

  void notifyParentRebuild();

  AABB getAABB() const {
    if (shape) return shape->getAABB();
    return AABB();
  }
};
