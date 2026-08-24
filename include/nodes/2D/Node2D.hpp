#pragma once

#include "core/Node.hpp"
#include "nodes/2D/Transform2D.hpp"
#include <cmath>
#include <string>

// 2D Spatial Scene Node (inspired by Godot Node2D) supporting 2D transforms,
// canvas rendering, and fixed-timestep render transform interpolation.
class Node2D : public Node {
public:
  Transform2D transform;

  inline static bool s_inRenderPass = false;

  Node2D() : Node("Node2D") {
    resetPhysicsInterpolation();
  }
  explicit Node2D(std::string nodeName) : Node(std::move(nodeName)) {
    resetPhysicsInterpolation();
  }

  Node2D(const Node2D &other)
      : Node(other),
        transform(other.transform),
        m_prevTransform(other.m_prevTransform),
        m_renderTransform(other.m_renderTransform),
        m_physicsInterpolation(other.m_physicsInterpolation) {}

  Node2D &operator=(const Node2D &other) {
    if (this != &other) {
      Node::operator=(other);
      transform = other.transform;
      m_prevTransform = other.m_prevTransform;
      m_renderTransform = other.m_renderTransform;
      m_physicsInterpolation = other.m_physicsInterpolation;
    }
    return *this;
  }

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

  // Enables or disables render transform interpolation for this node
  void setPhysicsInterpolation(bool enabled) { m_physicsInterpolation = enabled; }
  bool isPhysicsInterpolationEnabled() const { return m_physicsInterpolation; }

  // Resets the interpolation history (useful when teleporting or spawning to prevent visual stretching)
  void resetPhysicsInterpolation() {
    m_prevTransform = transform;
    m_renderTransform = transform;
  }

  // Teleports the node to a new location without rendering an interpolation streak
  void teleport(const Vector2 &pos) {
    transform.position = pos;
    resetPhysicsInterpolation();
  }

  // Saves transform state before fixed physics step
  void savePhysicsTransformState() override {
    m_prevTransform = transform;
    Node::savePhysicsTransformState();
  }

  // Computes the blended render transform between the last two physics states
  void interpolatePhysicsTransforms(float alpha) override {
    if (m_physicsInterpolation) {
      m_renderTransform.position = m_prevTransform.position.lerp(transform.position, alpha);
      m_renderTransform.scale = m_prevTransform.scale.lerp(transform.scale, alpha);

      // Smooth shortest-arc angular interpolation
      float diff = std::fmod(transform.rotation - m_prevTransform.rotation, 6.28318530718f);
      float distance = std::fmod(2.0f * diff, 6.28318530718f) - diff;
      m_renderTransform.rotation = m_prevTransform.rotation + distance * alpha;
    } else {
      m_renderTransform = transform;
    }
    Node::interpolatePhysicsTransforms(alpha);
  }

  // Returns the local render transform (interpolated during render passes, authoritative otherwise)
  const Transform2D &getRenderTransform() const {
    return (s_inRenderPass && m_physicsInterpolation) ? m_renderTransform : transform;
  }

  // Computes the node's global transform in world space compounding parent transforms.
  // Seamlessly uses interpolated transforms during rendering to guarantee smooth 144Hz+ rendering.
  Transform2D getGlobalTransform() const {
    const Transform2D &local = (s_inRenderPass && m_physicsInterpolation) ? m_renderTransform : transform;
    const Node *parent = getParent();
    if (parent) {
      const auto *parent2D = dynamic_cast<const Node2D *>(parent);
      if (parent2D) {
        Transform2D parentGlobal = parent2D->getGlobalTransform();
        return local.getGlobal(&parentGlobal);
      }
    }
    return local;
  }

  // Returns the global position in world space.
  Vector2 getGlobalPosition() const {
    return getGlobalTransform().position;
  }

private:
  Transform2D m_prevTransform;
  Transform2D m_renderTransform;
  bool m_physicsInterpolation = true;
};
