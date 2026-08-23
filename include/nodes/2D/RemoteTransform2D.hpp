#pragma once

#include "core/Memory.hpp"
#include "nodes/2D/Node2D.hpp"
#include <string>

// 2D Transform Synchronization Node (inspired by Godot RemoteTransform2D)
class RemoteTransform2D : public Node2D {
public:
  std::weak_ptr<Node2D> remoteNode;
  bool updatePosition = true;
  bool updateRotation = true;
  bool updateScale = true;
  bool useGlobalCoordinates = true;

  RemoteTransform2D() : Node2D("RemoteTransform2D") {}

  explicit RemoteTransform2D(Ref<Node2D> targetNode)
      : Node2D("RemoteTransform2D"), remoteNode(targetNode) {}

  void setRemoteNode(Ref<Node2D> target) {
    remoteNode = target;
  }

  void onProcess(float delta) override {
    (void)delta;
    syncRemoteTransform();
  }

  void syncRemoteTransform() {
    auto target = remoteNode.lock();
    if (!target) return;

    if (useGlobalCoordinates) {
      Transform2D globalTrans = getGlobalTransform();
      if (updatePosition) target->setPosition(globalTrans.position);
      if (updateRotation) target->setRotation(globalTrans.rotation);
      if (updateScale) target->setScale(globalTrans.scale);
    } else {
      if (updatePosition) target->setPosition(getPosition());
      if (updateRotation) target->setRotation(getRotation());
      if (updateScale) target->setScale(getScale());
    }
  }
};
