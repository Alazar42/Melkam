#pragma once

#include "core/Node.hpp"
#include "nodes/2D/Transform2D.hpp"
#include <string>

// 2D Spatial Scene Node (inspired by Godot Node2D) supporting 2D transforms and canvas rendering.
class Node2D : public Node {
public:
  Transform2D transform;

  Node2D() : Node("Node2D") {}
  explicit Node2D(std::string nodeName) : Node(std::move(nodeName)) {}

  // Sets local position.
  void setPosition(const Vector2 &pos) { transform.position = pos; }

  // Returns local position.
  const Vector2 &getPosition() const { return transform.position; }

  // Sets rotation in radians.
  void setRotation(float radians) { transform.rotation = radians; }

  // Returns rotation in radians.
  float getRotation() const { return transform.rotation; }

  // Sets rotation in degrees.
  void setRotationDegrees(float degrees) { transform.setRotationDegrees(degrees); }

  // Returns rotation in degrees.
  float getRotationDegrees() const { return transform.getRotationDegrees(); }

  // Sets scale.
  void setScale(const Vector2 &scale) { transform.scale = scale; }

  // Returns scale.
  const Vector2 &getScale() const { return transform.scale; }

  // Translates position by an offset.
  void translate(const Vector2 &offset) { transform.translate(offset); }

  // Rotates by an angle in radians.
  void rotate(float angleRadians) { transform.rotate(angleRadians); }

  // Computes the node's global transform in world space compounding parent transforms.
  Transform2D getGlobalTransform() const {
    const Node *parent = getParent();
    if (parent) {
      const auto *parent2D = dynamic_cast<const Node2D *>(parent);
      if (parent2D) {
        Transform2D parentGlobal = parent2D->getGlobalTransform();
        return transform.getGlobal(&parentGlobal);
      }
    }
    return transform;
  }

  // Returns the global position in world space.
  Vector2 getGlobalPosition() const {
    return getGlobalTransform().position;
  }
};
