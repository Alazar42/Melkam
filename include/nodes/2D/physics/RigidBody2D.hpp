#pragma once

#include "nodes/2D/physics/CollisionObject2D.hpp"

// Dynamic 2D Physics Body (inspired by Godot's RigidBody2D) with forces, impulses, and realistic gravity.
class RigidBody2D : public CollisionObject2D {
public:
  float mass = 1.0f;
  float friction = 1.0f;     // Godot 4 default friction is 1.0
  float restitution = 0.0f;  // Godot 4 default bounce is 0.0
  float linearDamping = 0.0f;
  float angularDamping = 0.0f;
  float gravityScale = 1.0f;
  bool lockRotation = false;

  bool isDynamicBody() const override { return true; }

  RigidBody2D() : CollisionObject2D("RigidBody2D") {}
  explicit RigidBody2D(std::string nodeName)
      : CollisionObject2D(std::move(nodeName)) {}

  // Direct constructor with box dimensions and visual color
  RigidBody2D(const Vector2 &boxSize,
              const Color &color = Color::from_rgba8(205, 133, 63))
      : CollisionObject2D("RigidBody2D") {
    shapeType = CollisionShapeType::Rectangle;
    shapeSize = boxSize;
    visualColor = color;
    hasVisualMesh = true;
  }

  RigidBody2D(std::string nodeName, const Vector2 &boxSize,
              const Color &color = Color::from_rgba8(205, 133, 63))
      : CollisionObject2D(std::move(nodeName)) {
    shapeType = CollisionShapeType::Rectangle;
    shapeSize = boxSize;
    visualColor = color;
    hasVisualMesh = true;
  }

  // Direct constructor with circle radius and visual color
  RigidBody2D(float circleRadius,
              const Color &color = Color::from_rgba8(205, 133, 63))
      : CollisionObject2D("RigidBody2D") {
    shapeType = CollisionShapeType::Circle;
    shapeRadius = circleRadius;
    visualColor = color;
    hasVisualMesh = true;
  }

  RigidBody2D(std::string nodeName, float circleRadius,
              const Color &color = Color::from_rgba8(205, 133, 63))
      : CollisionObject2D(std::move(nodeName)) {
    shapeType = CollisionShapeType::Circle;
    shapeRadius = circleRadius;
    visualColor = color;
    hasVisualMesh = true;
  }

  void onPhysicsProcess(float delta) override {
    (void)delta;
    syncFromPhysics();
  }

  void applyForce(const Vector2 &force) {
    if (!isBodyValid()) return;
    b2Vec2 b2Force = PhysicsServer2D::toMeters(force);
    b2Body_ApplyForceToCenter(m_bodyId, b2Force, true);
  }

  void applyImpulse(const Vector2 &impulse) {
    if (!isBodyValid()) return;
    b2Vec2 b2Impulse = PhysicsServer2D::toMeters(impulse);
    b2Body_ApplyLinearImpulseToCenter(m_bodyId, b2Impulse, true);
  }

  void setLinearVelocity(const Vector2 &velocity) {
    if (!isBodyValid()) return;
    b2Body_SetLinearVelocity(m_bodyId, PhysicsServer2D::toMeters(velocity));
  }

  Vector2 getLinearVelocity() const {
    if (!isBodyValid()) return {0.0f, 0.0f};
    b2Vec2 vel = b2Body_GetLinearVelocity(m_bodyId);
    return PhysicsServer2D::toPixels(vel);
  }

  static std::shared_ptr<RigidBody2D> createBox(
      const Vector2 &position, const Vector2 &size,
      const Color &color = Color::from_rgba8(205, 133, 63)) {
    auto body = std::make_shared<RigidBody2D>(size, color);
    body->setPosition(position);
    return body;
  }

protected:
  void createBody() override {
    b2WorldId worldId = PhysicsServer2D::getWorldId();
    if (!b2World_IsValid(worldId)) return;

    Transform2D globalTrans = getGlobalTransform();
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = PhysicsServer2D::toMeters(globalTrans.position);
    bodyDef.rotation = b2MakeRot(globalTrans.rotation);

    bodyDef.linearDamping = linearDamping;
    bodyDef.angularDamping = angularDamping;
    bodyDef.gravityScale = gravityScale;
    bodyDef.fixedRotation = lockRotation;
    bodyDef.userData = this;

    m_bodyId = b2CreateBody(worldId, &bodyDef);
  }

  void applyShapeMaterial(b2ShapeId sid) override {
    if (b2Shape_IsValid(sid)) {
      b2Shape_SetFriction(sid, friction);
      b2Shape_SetRestitution(sid, restitution);
    }
  }
};
