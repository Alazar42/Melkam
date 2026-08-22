#pragma once

#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/physics/CollisionShape2D.hpp"
#include "physics/2D/PhysicsServer2D.hpp"
#include "renderers/Renderer2D.hpp"
#include <box2d/box2d.h>
#include <memory>
#include <vector>

enum class CollisionShapeType {
  None,
  Rectangle,
  Circle,
  Capsule
};

// Base class for all 2D Physics and Collision Nodes (inspired by Godot's CollisionObject2D).
class CollisionObject2D : public Node2D {
public:
  uint32_t collisionLayer = 1; // Layer this object exists on
  uint32_t collisionMask = 1;  // Layers this object collides with or detects

  // Shape bounding geometry info
  CollisionShapeType shapeType = CollisionShapeType::None;
  Vector2 shapeSize{50.0f, 50.0f}; // For Box (width, height)
  float shapeRadius = 25.0f;       // For Circle & Capsule
  float shapeHeight = 50.0f;       // For Capsule

  // Visual appearance
  bool hasVisualMesh = false;
  Color visualColor = Color::WHITE;
  bool visualFilled = true;

  virtual Vector2 getHalfExtents() const {
    if (shapeType == CollisionShapeType::Circle) {
      return {shapeRadius, shapeRadius};
    }
    return shapeSize * 0.5f;
  }

  virtual bool isSensorBody() const { return false; }
  virtual bool isDynamicBody() const { return false; }

  Vector2 getGlobalPhysicsPosition() const {
    if (isBodyValid() && isDynamicBody()) {
      b2Vec2 pos = b2Body_GetPosition(m_bodyId);
      return PhysicsServer2D::toPixels(pos);
    }
    return getGlobalPosition();
  }

  CollisionObject2D() : Node2D("CollisionObject2D") {
    PhysicsServer2D::registerObject(this);
  }
  explicit CollisionObject2D(std::string nodeName)
      : Node2D(std::move(nodeName)) {
    PhysicsServer2D::registerObject(this);
  }

  virtual ~CollisionObject2D() {
    destroyBody();
    PhysicsServer2D::unregisterObject(this);
  }

  void onDestroy() override {
    destroyBody();
    PhysicsServer2D::unregisterObject(this);
  }

  // Registers a child CollisionShape2D onto the Box2D physics body
  virtual void registerChildShape(CollisionShape2D *shape) {
    if (!shape || shape->disabled) return;
    ensureBody();
    if (!isBodyValid()) return;

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter.categoryBits = collisionLayer;
    shapeDef.filter.maskBits = collisionMask;
    shapeDef.userData = this;

    b2ShapeId sid = shape->createBox2DShape(m_bodyId, shapeDef);
    applyShapeMaterial(sid);

    // Update body's bounding dimensions for ground checks
    if (shape->shapeType == CollisionShape2DType::Rectangle) {
      shapeType = CollisionShapeType::Rectangle;
      shapeSize = shape->size;
    } else if (shape->shapeType == CollisionShape2DType::Circle) {
      shapeType = CollisionShapeType::Circle;
      shapeRadius = shape->radius;
    } else if (shape->shapeType == CollisionShape2DType::Capsule) {
      shapeType = CollisionShapeType::Capsule;
      shapeRadius = shape->radius;
      shapeHeight = shape->height;
    }
  }

  // One-line setup for rectangular shape and visual mesh
  void setBox(const Vector2 &size, const Color &color = Color::WHITE, bool filled = true) {
    shapeType = CollisionShapeType::Rectangle;
    shapeSize = size;
    visualColor = color;
    visualFilled = filled;
    hasVisualMesh = true;
    rebuildPhysicsBody();
  }

  // One-line setup for circular shape and visual mesh
  void setCircle(float radius, const Color &color = Color::WHITE, bool filled = true) {
    shapeType = CollisionShapeType::Circle;
    shapeRadius = radius;
    visualColor = color;
    visualFilled = filled;
    hasVisualMesh = true;
    rebuildPhysicsBody();
  }

  // One-line setup for capsule shape and visual mesh
  void setCapsule(float radius, float height, const Color &color = Color::WHITE, bool filled = true) {
    shapeType = CollisionShapeType::Capsule;
    shapeRadius = radius;
    shapeHeight = height;
    visualColor = color;
    visualFilled = filled;
    hasVisualMesh = true;
    rebuildPhysicsBody();
  }

  void setBoxShape(const Vector2 &size) {
    shapeType = CollisionShapeType::Rectangle;
    shapeSize = size;
    rebuildPhysicsBody();
  }

  void setCircleShape(float radius) {
    shapeType = CollisionShapeType::Circle;
    shapeRadius = radius;
    rebuildPhysicsBody();
  }

  void setCapsuleShape(float radius, float height) {
    shapeType = CollisionShapeType::Capsule;
    shapeRadius = radius;
    shapeHeight = height;
    rebuildPhysicsBody();
  }

  void setVisualMesh(const Vector2 &size, const Color &color, bool filled = true) {
    shapeSize = size;
    visualColor = color;
    visualFilled = filled;
    hasVisualMesh = true;
  }

  void setVisualCircle(float radius, const Color &color, bool filled = true) {
    shapeRadius = radius;
    visualColor = color;
    visualFilled = filled;
    hasVisualMesh = true;
  }

  void setPosition(const Vector2 &pos) {
    Node2D::setPosition(pos);
    ensureBody();
    syncToPhysics();
  }

  void setRotation(float radians) {
    Node2D::setRotation(radians);
    ensureBody();
    syncToPhysics();
  }

  void syncFromPhysics() {
    if (!isBodyValid()) return;
    b2Vec2 pos = b2Body_GetPosition(m_bodyId);
    b2Rot rot = b2Body_GetRotation(m_bodyId);
    transform.position = PhysicsServer2D::toPixels(pos);
    transform.rotation = b2Rot_GetAngle(rot);
  }

  void syncToPhysics() {
    if (!isBodyValid()) return;
    b2Vec2 pos = PhysicsServer2D::toMeters(transform.position);
    b2Rot rot = b2MakeRot(transform.rotation);
    b2Body_SetTransform(m_bodyId, pos, rot);
  }

  b2BodyId getBodyId() const { return m_bodyId; }
  bool isBodyValid() const { return b2Body_IsValid(m_bodyId); }

  void onDraw() override {
    if (!visible) return;

    Transform2D global = getGlobalTransform();

    // 1. Draw Direct Visual Mesh if configured
    if (hasVisualMesh) {
      switch (shapeType) {
      case CollisionShapeType::Rectangle:
        Renderer2D::drawRect(global.position - shapeSize * 0.5f, shapeSize,
                             visualColor, visualFilled);
        break;
      case CollisionShapeType::Circle:
        Renderer2D::drawCircle(global.position, shapeRadius, visualColor,
                               visualFilled, 32);
        break;
      case CollisionShapeType::Capsule:
        Renderer2D::drawRect(
            global.position - Vector2(shapeRadius, shapeHeight * 0.5f),
            Vector2(shapeRadius * 2.0f, shapeHeight), visualColor, visualFilled);
        break;
      default:
        break;
      }
    }

    // 2. Draw Debug Outline if debug collisions are enabled
    if (PhysicsServer2D::isDebugCollisions()) {
      Color debugColor = Color::from_rgba8(80, 220, 120, 180);
      switch (shapeType) {
      case CollisionShapeType::Rectangle:
        Renderer2D::drawRect(global.position - shapeSize * 0.5f, shapeSize,
                             debugColor, false);
        break;
      case CollisionShapeType::Circle:
        Renderer2D::drawCircle(global.position, shapeRadius, debugColor, false, 32);
        break;
      case CollisionShapeType::Capsule:
        Renderer2D::drawRect(
            global.position - Vector2(shapeRadius, shapeHeight * 0.5f),
            Vector2(shapeRadius * 2.0f, shapeHeight), debugColor, false);
        break;
      default:
        break;
      }
    }
  }

  void ensureBody() {
    if (!isBodyValid()) {
      createBody();
      createDirectShape();
      attachExistingChildShapes();
    }
  }

  void rebuildPhysicsBody() {
    destroyBody();
    ensureBody();
  }

protected:
  virtual void createBody() {}

  virtual void applyShapeMaterial(b2ShapeId sid) {
    (void)sid;
  }

  virtual void createDirectShape() {
    if (!isBodyValid() || shapeType == CollisionShapeType::None) return;

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter.categoryBits = collisionLayer;
    shapeDef.filter.maskBits = collisionMask;
    shapeDef.userData = this;

    switch (shapeType) {
    case CollisionShapeType::Rectangle: {
      b2Polygon box = b2MakeBox(PhysicsServer2D::toMeters(shapeSize.x * 0.5f),
                                PhysicsServer2D::toMeters(shapeSize.y * 0.5f));
      m_shapeId = b2CreatePolygonShape(m_bodyId, &shapeDef, &box);
      break;
    }
    case CollisionShapeType::Circle: {
      b2Circle circle = {{0.0f, 0.0f}, PhysicsServer2D::toMeters(shapeRadius)};
      m_shapeId = b2CreateCircleShape(m_bodyId, &shapeDef, &circle);
      break;
    }
    case CollisionShapeType::Capsule: {
      float halfH = std::max(0.0f, (shapeHeight * 0.5f) - shapeRadius);
      b2Capsule capsule = {
          {0.0f, -PhysicsServer2D::toMeters(halfH)},
          {0.0f, PhysicsServer2D::toMeters(halfH)},
          PhysicsServer2D::toMeters(shapeRadius)};
      m_shapeId = b2CreateCapsuleShape(m_bodyId, &shapeDef, &capsule);
      break;
    }
    default:
      break;
    }

    if (b2Shape_IsValid(m_shapeId)) {
      applyShapeMaterial(m_shapeId);
    }
  }

  void attachExistingChildShapes() {
    for (const auto &child : getChildren()) {
      if (auto colShape = std::dynamic_pointer_cast<CollisionShape2D>(child)) {
        registerChildShape(colShape.get());
      }
    }
  }

  void destroyBody() {
    if (isBodyValid()) {
      b2DestroyBody(m_bodyId);
      m_bodyId = b2_nullBodyId;
      m_shapeId = b2_nullShapeId;
    }
  }

  b2BodyId m_bodyId = b2_nullBodyId;
    b2ShapeId m_shapeId = b2_nullShapeId;
};

// Inline definition of CollisionShape2D::onReady()
inline void CollisionShape2D::onReady() {
  Node *curr = getParent();
  while (curr) {
    if (auto colObj = dynamic_cast<CollisionObject2D *>(curr)) {
      colObj->registerChildShape(this);
      break;
    }
    curr = curr->getParent();
  }
}

// Inline definition of PhysicsServer2D::syncRegisteredObjects()
inline void PhysicsServer2D::syncRegisteredObjects() {
  for (CollisionObject2D *obj : s_registeredObjects) {
    if (obj && obj->isBodyValid() && obj->isDynamicBody()) {
      obj->syncFromPhysics();
    }
  }
}
