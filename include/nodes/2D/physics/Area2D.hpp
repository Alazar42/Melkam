#pragma once

#include "core/Signal.hpp"
#include "nodes/2D/physics/CollisionObject2D.hpp"
#include "nodes/2D/physics/CollisionShape2D.hpp"
#include "physics/2D/PhysicsServer2D.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// 2D Region Detector & Trigger Sensor Node (inspired by Godot's Area2D).
// Detects when physics bodies (CharacterBody2D, RigidBody2D, StaticBody2D)
// or other Area2D nodes enter or exit its collision boundary.
class Area2D : public CollisionObject2D {
public:
  // ===========================================================================
  // Signals (Godot Architecture)
  // ===========================================================================
  Signal<Node2D *> body_entered;
  Signal<Node2D *> body_exited;
  Signal<Area2D *> area_entered;
  Signal<Area2D *> area_exited;

  // Configuration Properties
  bool monitoring = true;  // Whether this area detects other bodies/areas
  bool monitorable = true; // Whether this area can be detected by other areas
  float areaGravity = 0.0f;
  Vector2 gravityDirection{0.0f, 1.0f};
  float linearDamp = 0.1f;
  float angularDamp = 0.1f;

  // Constructors
  Area2D() : CollisionObject2D("Area2D") {}
  explicit Area2D(std::string nodeName)
      : CollisionObject2D(std::move(nodeName)) {}

  // Direct shape convenience constructors
  Area2D(const Vector2 &boxSize,
         const Color &color = Color::from_rgba8(255, 215, 0, 200))
      : CollisionObject2D("Area2D") {
    shapeType = CollisionShapeType::Rectangle;
    shapeSize = boxSize;
    visualColor = color;
    hasVisualMesh = true;
  }

  Area2D(float circleRadius,
         const Color &color = Color::from_rgba8(255, 215, 0, 200))
      : CollisionObject2D("Area2D") {
    shapeType = CollisionShapeType::Circle;
    shapeRadius = circleRadius;
    visualColor = color;
    hasVisualMesh = true;
  }

  // ===========================================================================
  // Query API
  // ===========================================================================

  // Returns true if at least one physics body is currently overlapping this area
  bool hasOverlappingBodies() const {
    return !m_overlappingBodies.empty();
  }

  // Returns true if at least one other area is currently overlapping this area
  bool hasOverlappingAreas() const {
    return !m_overlappingAreas.empty();
  }

  // Returns true if the specified body is currently inside this area
  bool overlapsBody(Node2D *body) const {
    if (!body) return false;
    return std::find(m_overlappingBodies.begin(), m_overlappingBodies.end(),
                     body) != m_overlappingBodies.end();
  }

  // Returns true if the specified area is currently inside this area
  bool overlapsArea(Area2D *area) const {
    if (!area) return false;
    return std::find(m_overlappingAreas.begin(), m_overlappingAreas.end(),
                     area) != m_overlappingAreas.end();
  }

  // Returns the list of physics bodies currently overlapping this area
  const std::vector<Node2D *> &getOverlappingBodies() const {
    return m_overlappingBodies;
  }

  // Returns the list of areas currently overlapping this area
  const std::vector<Area2D *> &getOverlappingAreas() const {
    return m_overlappingAreas;
  }

  // ===========================================================================
  // Signal Dispatching Helpers
  // ===========================================================================

  void notifyBodyEntered(Node2D *body) {
    if (!body) return;
    if (std::find(m_overlappingBodies.begin(), m_overlappingBodies.end(),
                  body) == m_overlappingBodies.end()) {
      m_overlappingBodies.push_back(body);
      body_entered.emit(body);
    }
  }

  void notifyBodyExited(Node2D *body) {
    if (!body) return;
    auto it = std::find(m_overlappingBodies.begin(), m_overlappingBodies.end(),
                        body);
    if (it != m_overlappingBodies.end()) {
      m_overlappingBodies.erase(it);
      body_exited.emit(body);
    }
  }

  void notifyAreaEntered(Area2D *area) {
    if (!area) return;
    if (std::find(m_overlappingAreas.begin(), m_overlappingAreas.end(),
                  area) == m_overlappingAreas.end()) {
      m_overlappingAreas.push_back(area);
      area_entered.emit(area);
    }
  }

  void notifyAreaExited(Area2D *area) {
    if (!area) return;
    auto it = std::find(m_overlappingAreas.begin(), m_overlappingAreas.end(),
                        area);
    if (it != m_overlappingAreas.end()) {
      m_overlappingAreas.erase(it);
      area_exited.emit(area);
    }
  }

  // ===========================================================================
  // Geometric Intersection Math
  // ===========================================================================

  // Checks whether two collision objects overlap based on their shapes & global transforms
  static bool checkOverlap(const CollisionObject2D *a,
                           const CollisionObject2D *b) {
    if (!a || !b) return false;

    Vector2 posA = a->getGlobalPosition();
    Vector2 posB = b->getGlobalPosition();

    // 1. Circle vs Circle
    if (a->shapeType == CollisionShapeType::Circle &&
        b->shapeType == CollisionShapeType::Circle) {
      float radSum = a->shapeRadius + b->shapeRadius;
      return (posA - posB).length_squared() <= (radSum * radSum);
    }

    // 2. Circle vs Box
    if (a->shapeType == CollisionShapeType::Circle &&
        b->shapeType != CollisionShapeType::Circle) {
      Vector2 halfB = b->shapeSize * 0.5f;
      float clampX = std::clamp(posA.x, posB.x - halfB.x, posB.x + halfB.x);
      float clampY = std::clamp(posA.y, posB.y - halfB.y, posB.y + halfB.y);
      Vector2 closest(clampX, clampY);
      return (posA - closest).length_squared() <= (a->shapeRadius * a->shapeRadius);
    }

    // 3. Box vs Circle
    if (a->shapeType != CollisionShapeType::Circle &&
        b->shapeType == CollisionShapeType::Circle) {
      Vector2 halfA = a->shapeSize * 0.5f;
      float clampX = std::clamp(posB.x, posA.x - halfA.x, posA.x + halfA.x);
      float clampY = std::clamp(posB.y, posA.y - halfA.y, posA.y + halfA.y);
      Vector2 closest(clampX, clampY);
      return (posB - closest).length_squared() <= (b->shapeRadius * b->shapeRadius);
    }

    // 4. Box vs Box (AABB)
    Vector2 halfA = a->shapeSize * 0.5f;
    Vector2 halfB = b->shapeSize * 0.5f;
    return std::abs(posA.x - posB.x) <= (halfA.x + halfB.x) &&
           std::abs(posA.y - posB.y) <= (halfA.y + halfB.y);
  }

  // ===========================================================================
  // Physics Lifecycle & Continuous Detection
  // ===========================================================================

  void onPhysicsProcess(float delta) override {
    (void)delta;
    if (!monitoring || !isGlobalVisible()) {
      // Clear overlapping sets when disabled or hidden
      while (!m_overlappingBodies.empty()) {
        notifyBodyExited(m_overlappingBodies.front());
      }
      while (!m_overlappingAreas.empty()) {
        notifyAreaExited(m_overlappingAreas.front());
      }
      return;
    }

    const auto &allObjects = PhysicsServer2D::getRegisteredObjects();
    for (CollisionObject2D *obj : allObjects) {
      if (!obj || obj == this || !obj->isGlobalVisible()) continue;

      // Check collision layer/mask compatibility
      if ((collisionMask & obj->collisionLayer) == 0 &&
          (collisionLayer & obj->collisionMask) == 0) {
        continue;
      }

      bool isOverlapping = checkOverlap(this, obj);

      if (auto *otherArea = dynamic_cast<Area2D *>(obj)) {
        if (isOverlapping && otherArea->monitorable) {
          notifyAreaEntered(otherArea);
        } else {
          notifyAreaExited(otherArea);
        }
      } else {
        if (isOverlapping) {
          notifyBodyEntered(obj);
        } else {
          notifyBodyExited(obj);
        }
      }
    }
  }

  static std::shared_ptr<Area2D> createTrigger(
      const Vector2 &position, const Vector2 &size,
      const Color &color = Color::from_rgba8(255, 215, 0, 200)) {
    auto area = std::make_shared<Area2D>(size, color);
    area->setPosition(position);
    return area;
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

  void registerChildShape(CollisionShape2D *shape) override {
    if (!shape || shape->disabled) return;
    ensureBody();
    if (!isBodyValid()) return;

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.isSensor = true; // Area2D shapes are always non-solid sensors
    shapeDef.enableSensorEvents = true;
    shapeDef.filter.categoryBits = collisionLayer;
    shapeDef.filter.maskBits = collisionMask;
    shapeDef.userData = this;

    shape->createBox2DShape(m_bodyId, shapeDef);

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

  void createDirectShape() override {
    if (!isBodyValid() || shapeType == CollisionShapeType::None) return;

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.isSensor = true;
    shapeDef.enableSensorEvents = true;
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
  }

private:
  std::vector<Node2D *> m_overlappingBodies;
  std::vector<Area2D *> m_overlappingAreas;
};
