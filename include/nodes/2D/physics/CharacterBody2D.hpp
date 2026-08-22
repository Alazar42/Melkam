#pragma once

#include "nodes/2D/physics/CollisionObject2D.hpp"
#include "time.hpp"
#include <cmath>

enum class MotionMode {
  Grounded, // Platformer / side-scroller with floor detection
  Floating  // Top-down / omnidirectional movement
};

// Character Controller Physics Body (inspired by Godot's CharacterBody2D) with moveAndSlide.
class CharacterBody2D : public CollisionObject2D {
public:
  Vector2 velocity{0.0f, 0.0f};
  Vector2 upDirection{0.0f, -1.0f}; // Points upwards for platformers
  MotionMode motionMode = MotionMode::Grounded;
  float floorMaxAngle = 0.785398f;  // Max slope angle (45 degrees in radians)
  int maxSlides = 4;

  CharacterBody2D() : CollisionObject2D("CharacterBody2D") {}
  explicit CharacterBody2D(std::string nodeName)
      : CollisionObject2D(std::move(nodeName)) {}

  // Direct constructor with box dimensions and visual color
  CharacterBody2D(const Vector2 &boxSize, const Color &color = Color::GOLD)
      : CollisionObject2D("CharacterBody2D") {
    shapeType = CollisionShapeType::Rectangle;
    shapeSize = boxSize;
    visualColor = color;
    hasVisualMesh = true;
  }

  bool isOnFloor() const { return m_isOnFloor; }
  bool isOnWall() const { return m_isOnWall; }
  bool isOnCeiling() const { return m_isOnCeiling; }
  const Vector2 &getFloorNormal() const { return m_floorNormal; }
  const Vector2 &getWallNormal() const { return m_wallNormal; }

  // Moves the character body based on current velocity, sliding along surfaces and updating floor/wall states
  void moveAndSlide() {
    float dt = Time::getDeltaTime();
    if (dt <= 0.0f) return;

    ensureBody();

    m_isOnFloor = false;
    m_isOnWall = false;
    m_isOnCeiling = false;
    m_floorNormal = {0.0f, -1.0f};
    m_wallNormal = {0.0f, 0.0f};

    if (isBodyValid()) {
      // 1. Set linear velocity on the Box2D dynamic body
      b2Vec2 b2Vel = PhysicsServer2D::toMeters(velocity);
      b2Body_SetLinearVelocity(m_bodyId, b2Vel);

      // 2. Read physical position solved by Box2D
      b2Vec2 pos = b2Body_GetPosition(m_bodyId);
      transform.position = PhysicsServer2D::toPixels(pos);

      // 3. Ground detection raycast (cast across 5 foot points with 10px depth)
      if (motionMode == MotionMode::Grounded) {
        float halfH = (shapeType == CollisionShapeType::Circle) ? shapeRadius : (shapeSize.y * 0.5f);
        float halfW = (shapeType == CollisionShapeType::Circle) ? shapeRadius : (shapeSize.x * 0.5f);

        b2QueryFilter filter = b2DefaultQueryFilter();
        filter.maskBits = collisionMask;
        b2Vec2 translation = {0.0f, PhysicsServer2D::toMeters(10.0f)};

        float footOffsets[5] = {0.0f, -halfW * 0.45f, halfW * 0.45f, -halfW * 0.85f, halfW * 0.85f};
        for (float xOff : footOffsets) {
          b2Vec2 origin = {pos.x + PhysicsServer2D::toMeters(xOff),
                           pos.y + PhysicsServer2D::toMeters(halfH - 1.0f)};
          b2RayResult rayResult =
              b2World_CastRayClosest(PhysicsServer2D::getWorldId(), origin, translation, filter);
          if (rayResult.hit && b2Shape_IsValid(rayResult.shapeId)) {
            // Ignore sensor shapes (Area2D, triggers, coins) for ground detection
            if (b2Shape_IsSensor(rayResult.shapeId)) {
              continue;
            }
            void *hitUserData = b2Shape_GetUserData(rayResult.shapeId);
            if (hitUserData != this) {
              m_isOnFloor = true;
              m_floorNormal = Vector2(rayResult.normal.x, rayResult.normal.y);
              break;
            }
          }
        }
      }
    } else {
      transform.position += velocity * dt;
    }
  }

  void setOnFloor(bool onFloor, const Vector2 &normal = {0.0f, -1.0f}) {
    m_isOnFloor = onFloor;
    if (onFloor) m_floorNormal = normal;
  }

  void setOnWall(bool onWall, const Vector2 &normal = {1.0f, 0.0f}) {
    m_isOnWall = onWall;
    if (onWall) m_wallNormal = normal;
  }

  void setOnCeiling(bool onCeiling) {
    m_isOnCeiling = onCeiling;
  }

protected:
  void createBody() override {
    b2WorldId worldId = PhysicsServer2D::getWorldId();
    if (!b2World_IsValid(worldId)) return;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody; // Dynamic body prevents passing through solid walls/floors
    bodyDef.gravityScale = 0.0f;   // Gravity is handled via CharacterBody2D script
    bodyDef.fixedRotation = true;  // Character bodies do not rotate
    bodyDef.position = PhysicsServer2D::toMeters(transform.position);
    bodyDef.rotation = b2MakeRot(transform.rotation);
    bodyDef.userData = this;

    m_bodyId = b2CreateBody(worldId, &bodyDef);
  }

  void applyShapeMaterial(b2ShapeId sid) override {
    if (b2Shape_IsValid(sid)) {
      b2Shape_SetFriction(sid, 0.0f); // Smooth sliding along walls and floors
      b2Shape_SetRestitution(sid, 0.0f);
    }
  }

private:
  bool m_isOnFloor = false;
  bool m_isOnWall = false;
  bool m_isOnCeiling = false;
  Vector2 m_floorNormal{0.0f, -1.0f};
  Vector2 m_wallNormal{0.0f, 0.0f};
};
