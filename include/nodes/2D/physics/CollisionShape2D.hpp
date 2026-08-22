#pragma once

#include "nodes/2D/Node2D.hpp"
#include "physics/2D/PhysicsServer2D.hpp"
#include "renderers/Renderer2D.hpp"
#include <box2d/box2d.h>
#include <algorithm>
#include <memory>

class CollisionObject2D;

enum class CollisionShape2DType {
  Rectangle,
  Circle,
  Capsule
};

// Collision shape node (inspired by Godot's CollisionShape2D) defining collision bounds on a physics body.
class CollisionShape2D : public Node2D {
public:
  CollisionShape2DType shapeType = CollisionShape2DType::Rectangle;
  Vector2 size{50.0f, 50.0f}; // For Rectangle (width, height)
  float radius = 25.0f;       // For Circle & Capsule radius
  float height = 50.0f;       // For Capsule height
  bool disabled = false;

  CollisionShape2D() : Node2D("CollisionShape2D") {}

  // Direct rectangle shape constructor
  explicit CollisionShape2D(const Vector2 &rectSize)
      : Node2D("CollisionShape2D"), shapeType(CollisionShape2DType::Rectangle),
        size(rectSize) {}

  // Direct circle shape constructor
  explicit CollisionShape2D(float circleRadius)
      : Node2D("CollisionShape2D"), shapeType(CollisionShape2DType::Circle),
        radius(circleRadius) {}

  // Direct capsule shape constructor
  CollisionShape2D(float capRadius, float capHeight)
      : Node2D("CollisionShape2D"), shapeType(CollisionShape2DType::Capsule),
        radius(capRadius), height(capHeight) {}

  // Factory helpers
  static std::shared_ptr<CollisionShape2D> createRectangle(const Vector2 &size) {
    return std::make_shared<CollisionShape2D>(size);
  }

  static std::shared_ptr<CollisionShape2D> createCircle(float radius) {
    return std::make_shared<CollisionShape2D>(radius);
  }

  static std::shared_ptr<CollisionShape2D> createCapsule(float radius, float height) {
    return std::make_shared<CollisionShape2D>(radius, height);
  }

  // Automatically registers this shape onto parent CollisionObject2D when entering the scene tree
  void onReady() override;

  // Creates the physical Box2D shape attached to the parent b2BodyId
  b2ShapeId createBox2DShape(b2BodyId bodyId, const b2ShapeDef &shapeDef) {
    if (!b2Body_IsValid(bodyId)) return b2_nullShapeId;

    switch (shapeType) {
    case CollisionShape2DType::Rectangle: {
      b2Polygon box = b2MakeBox(PhysicsServer2D::toMeters(size.x * 0.5f),
                                PhysicsServer2D::toMeters(size.y * 0.5f));
      m_shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
      break;
    }

    case CollisionShape2DType::Circle: {
      b2Circle circle = {{0.0f, 0.0f}, PhysicsServer2D::toMeters(radius)};
      m_shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
      break;
    }

    case CollisionShape2DType::Capsule: {
      float halfH = std::max(0.0f, (height * 0.5f) - radius);
      b2Capsule capsule = {
          {0.0f, -PhysicsServer2D::toMeters(halfH)},
          {0.0f, PhysicsServer2D::toMeters(halfH)},
          PhysicsServer2D::toMeters(radius)};
      m_shapeId = b2CreateCapsuleShape(bodyId, &shapeDef, &capsule);
      break;
    }
    }
    return m_shapeId;
  }

  // Renders debug wireframe when debug collisions are enabled
  void onDraw() override {
    if (!PhysicsServer2D::isDebugCollisions() || disabled) return;

    Transform2D global = getGlobalTransform();
    Color debugColor = disabled ? Color::from_rgba8(120, 120, 120, 120)
                                : Color::from_rgba8(80, 220, 120, 150);

    switch (shapeType) {
    case CollisionShape2DType::Rectangle:
      Renderer2D::drawRect(global.position - size * 0.5f, size, debugColor, false);
      break;

    case CollisionShape2DType::Circle:
      Renderer2D::drawCircle(global.position, radius, debugColor, false, 32);
      break;

    case CollisionShape2DType::Capsule:
      Renderer2D::drawRect(global.position - Vector2(radius, height * 0.5f),
                           Vector2(radius * 2.0f, height), debugColor, false);
      break;
    }
  }

  b2ShapeId getShapeId() const { return m_shapeId; }

private:
  b2ShapeId m_shapeId = b2_nullShapeId;
};
