#pragma once

#include "ECS.hpp"
#include "components/Components3D.hpp"
#include "core/Node.hpp"
#include "nodes/3D/Basis.hpp"
#include "nodes/3D/Quaternion.hpp"
#include "nodes/3D/Transform3D.hpp"
#include <cmath>
#include <string>

// 3D Spatial Scene Node (inspired by Godot Node3D / Spatial architecture).
// Connects the high-level scene graph with the cache-friendly EnTT ECS backend.
class Node3D : public Node {
public:
  Transform3D transform;

  Node3D() : Node("Node3D") {
    initECS();
  }

  explicit Node3D(std::string nodeName) : Node(std::move(nodeName)) {
    initECS();
  }

  ~Node3D() override {
    if (m_entity.isValid()) {
      m_entity.destroy();
    }
  }

  Node3D(const Node3D &other)
      : Node(other), transform(other.transform) {
    initECS();
  }

  Node3D &operator=(const Node3D &other) {
    if (this != &other) {
      Node::operator=(other);
      transform = other.transform;
      syncToECS();
    }
    return *this;
  }

  // Position / Translation
  void setPosition(const Vector3 &pos) {
    transform.origin = pos;
    syncToECS();
  }

  const Vector3 &getPosition() const { return transform.origin; }

  // Rotation via Basis
  void setBasis(const Basis &basis) {
    transform.basis = basis;
    syncToECS();
  }

  const Basis &getBasis() const { return transform.basis; }

  // Rotation via Euler angles (in radians, YXZ order)
  void setRotation(const Vector3 &euler) {
    Vector3 currentScale = getScale();
    transform.basis = Basis::fromEuler(euler).scaled(currentScale);
    syncToECS();
  }

  Vector3 getRotation() const {
    return transform.basis.get_quaternion().slerp(Quaternion(), 0.0f).xform(Vector3(1.0f, 1.0f, 1.0f));
  }

  // Rotation via Euler angles (in degrees)
  void setRotationDegrees(const Vector3 &degrees) {
    setRotation(degrees * (3.14159265f / 180.0f));
  }

  // Scale
  void setScale(const Vector3 &scale) {
    transform = Transform3D(transform.basis.orthonormalized().scaled(scale), transform.origin);
    syncToECS();
  }

  Vector3 getScale() const { return transform.basis.get_scale(); }

  // Spatial Transformations
  void translate(const Vector3 &offset) {
    transform = transform.translated(offset);
    syncToECS();
  }

  void translateLocal(const Vector3 &offset) {
    transform = transform.translated_local(offset);
    syncToECS();
  }

  void rotate(const Vector3 &axis, float angleRadians) {
    transform = transform.rotated(axis, angleRadians);
    syncToECS();
  }

  void rotateX(float angleRadians) {
    rotate(Vector3(1.0f, 0.0f, 0.0f), angleRadians);
  }

  void rotateY(float angleRadians) {
    rotate(Vector3(0.0f, 1.0f, 0.0f), angleRadians);
  }

  void rotateZ(float angleRadians) {
    rotate(Vector3(0.0f, 0.0f, 1.0f), angleRadians);
  }

  // Orient towards a target point in world space
  void lookAt(const Vector3 &target, const Vector3 &up = Vector3(0.0f, 1.0f, 0.0f)) {
    transform = transform.looking_at(target, up);
    syncToECS();
  }

  // Returns Global 3D Transform
  Transform3D getGlobalTransform() const {
    if (auto *parent3D = dynamic_cast<Node3D *>(getParent())) {
      return parent3D->getGlobalTransform() * transform;
    }
    return transform;
  }

  Vector3 getGlobalPosition() const {
    return getGlobalTransform().origin;
  }

  void onReady() override {
    syncToECS();
  }

  // Internal EnTT Entity Access
  Entity getEntity() const { return m_entity; }

protected:
  void syncToECS() {
    if (m_entity.isValid()) {
      auto &comp = m_entity.getOrAddComponent<Transform3DComponent>();
      comp.localTransform = transform;
      if (auto *parent3D = dynamic_cast<Node3D *>(getParent())) {
        comp.parent = parent3D->getEntity().getHandle();
      } else {
        comp.parent = entt::null;
      }
      comp.isDirty = true;
    }
  }

  void initECS() {
    m_entity = Entity::create();
    m_entity.addComponent<Transform3DComponent>(transform);
  }

  Entity m_entity;
};
