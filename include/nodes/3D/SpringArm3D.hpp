#pragma once

#include "nodes/3D/Node3D.hpp"
#include <algorithm>

// 3D Spring Arm / Boom Node (inspired by Godot SpringArm3D for 3rd person follow cameras)
class SpringArm3D : public Node3D {
public:
  float springLength = 5.0f;
  float margin = 0.2f;
  float currentLength = 5.0f;

  SpringArm3D() : Node3D("SpringArm3D") {}

  void onProcess(float delta) override {
    (void)delta;
    currentLength = springLength;

    // Propagate position to all children (e.g. Camera3D attached at the end of the spring arm)
    for (const auto &child : getChildren()) {
      if (auto *child3D = dynamic_cast<Node3D *>(child.get())) {
        child3D->setPosition(Vector3(0.0f, 0.0f, currentLength));
      }
    }
  }

  float getHitLength() const { return currentLength; }
};
