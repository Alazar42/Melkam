#pragma once

#include "nodes/2D/physics/CollisionObject2D.hpp"

// Immovable solid physics body (inspired by Godot's StaticBody2D) for walls, floors, and platforms.
class StaticBody2D : public CollisionObject2D {
public:
  float friction = 0.5f;
  float restitution = 0.0f; // Bounciness

  StaticBody2D() : CollisionObject2D("StaticBody2D") {}
  explicit StaticBody2D(std::string nodeName)
      : CollisionObject2D(std::move(nodeName)) {}

  // Direct constructor with box dimensions and visual color
  StaticBody2D(const Vector2 &boxSize,
               const Color &color = Color::from_rgba8(70, 70, 85))
      : CollisionObject2D("StaticBody2D") {
    shapeType = CollisionShapeType::Rectangle;
    shapeSize = boxSize;
    visualColor = color;
    hasVisualMesh = true;
  }

  StaticBody2D(std::string nodeName, const Vector2 &boxSize,
               const Color &color = Color::from_rgba8(70, 70, 85))
      : CollisionObject2D(std::move(nodeName)) {
    shapeType = CollisionShapeType::Rectangle;
    shapeSize = boxSize;
    visualColor = color;
    hasVisualMesh = true;
  }

  // Direct constructor with circle radius and visual color
  StaticBody2D(float circleRadius,
               const Color &color = Color::from_rgba8(70, 70, 85))
      : CollisionObject2D("StaticBody2D") {
    shapeType = CollisionShapeType::Circle;
    shapeRadius = circleRadius;
    visualColor = color;
    hasVisualMesh = true;
  }

  StaticBody2D(std::string nodeName, float circleRadius,
               const Color &color = Color::from_rgba8(70, 70, 85))
      : CollisionObject2D(std::move(nodeName)) {
    shapeType = CollisionShapeType::Circle;
    shapeRadius = circleRadius;
    visualColor = color;
    hasVisualMesh = true;
  }

  static std::shared_ptr<StaticBody2D> createPlatform(
      const Vector2 &position, const Vector2 &size,
      const Color &color = Color::from_rgba8(70, 70, 85)) {
    auto body = std::make_shared<StaticBody2D>(size, color);
    body->setPosition(position);
    return body;
  }

protected:
  void createBody() override {
    b2WorldId worldId = PhysicsServer2D::getWorldId();
    if (!b2World_IsValid(worldId)) return;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = PhysicsServer2D::toMeters(transform.position);
    bodyDef.rotation = b2MakeRot(transform.rotation);
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
