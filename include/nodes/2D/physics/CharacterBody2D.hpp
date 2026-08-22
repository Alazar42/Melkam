#pragma once

#include "nodes/2D/physics/CollisionObject2D.hpp"
#include "nodes/2D/physics/KinematicCollision2D.hpp"
#include "nodes/2D/physics/RigidBody2D.hpp"
#include "physics/2D/PhysicsServer2D.hpp"
#include "time.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

enum class MotionMode {
  Grounded, // Platformer / side-scroller with floor detection
  Floating  // Top-down / omnidirectional movement
};

// 2D Kinematic Character Controller (inspired by Godot's CharacterBody2D).
// Features robust swept-shape collision detection, sliding along surfaces,
// floor/wall/ceiling state tracking, and dynamic body pushing.
class CharacterBody2D : public CollisionObject2D {
public:
  Vector2 velocity{0.0f, 0.0f};
  Vector2 upDirection{0.0f, -1.0f}; // Default upwards vector for 2D platformers
  MotionMode motionMode = MotionMode::Grounded;
  float floorMaxAngle = 0.785398f;  // Max slope angle (45 degrees in radians)
  bool floorStopOnSlope = true;     // Prevents sliding on slopes when idle
  float floorSnapLength = 8.0f;     // Downward ground probing snap distance
  int maxSlides = 4;                // Maximum slide iterations per moveAndSlide call
  float safeMargin = 0.08f;         // Micro-margin to avoid shape penetration

  CharacterBody2D() : CollisionObject2D("CharacterBody2D") {}
  explicit CharacterBody2D(std::string nodeName)
      : CollisionObject2D(std::move(nodeName)) {}

  // Direct box constructor
  CharacterBody2D(const Vector2 &boxSize, const Color &color = Color::GOLD)
      : CollisionObject2D("CharacterBody2D") {
    shapeType = CollisionShapeType::Rectangle;
    shapeSize = boxSize;
    visualColor = color;
    hasVisualMesh = true;
  }

  // ===========================================================================
  // State Queries
  // ===========================================================================

  bool isOnFloor() const { return m_isOnFloor; }
  bool isOnWall() const { return m_isOnWall; }
  bool isOnCeiling() const { return m_isOnCeiling; }
  const Vector2 &getFloorNormal() const { return m_floorNormal; }
  const Vector2 &getWallNormal() const { return m_wallNormal; }

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

  // Returns number of collisions during the last moveAndSlide() call
  int getSlideCollisionCount() const {
    return static_cast<int>(m_slideCollisions.size());
  }

  // Returns a specific slide collision from the last moveAndSlide() call
  KinematicCollision2D getSlideCollision(int index) const {
    if (index >= 0 && index < static_cast<int>(m_slideCollisions.size())) {
      return m_slideCollisions[index];
    }
    return KinematicCollision2D{};
  }

  // ===========================================================================
  // Kinematic Swept Collision & Slide Algorithm
  // ===========================================================================

  // Sweeps the character along motion vector, moving until first collision
  KinematicCollision2D moveAndCollide(const Vector2 &motion, bool testOnly = false) {
    KinematicCollision2D result;
    if (motion.length_squared() < 0.00001f) {
      return result;
    }

    Vector2 pos0 = getGlobalPhysicsPosition();
    Vector2 halfSelf = getHalfExtents();

    float earliestTime = 1.0f;
    Vector2 bestNormal{0.0f, -1.0f};
    CollisionObject2D *bestCollider = nullptr;

    const auto &objects = PhysicsServer2D::getRegisteredObjects();
    for (CollisionObject2D *other : objects) {
      if (!other || other == this || !other->isGlobalVisible()) continue;

      // Ignore trigger sensors (Area2D) for solid body movement
      if (other->shapeType == CollisionShapeType::None) continue;
      if (other->isSensorBody()) continue;

      // Check collision layer & mask compatibility
      if ((collisionMask & other->collisionLayer) == 0 &&
          (collisionLayer & other->collisionMask) == 0) {
        continue;
      }

      Vector2 otherPos = other->getGlobalPhysicsPosition();
      Vector2 halfOther = other->getHalfExtents();

      // Expanded Minkowski obstacle bounds
      Vector2 minBound = otherPos - halfOther - halfSelf;
      Vector2 maxBound = otherPos + halfOther + halfSelf;

      // Swept AABB intersection test
      float tEntryX, tExitX, tEntryY, tExitY;

      if (motion.x > 0.0f) {
        tEntryX = (minBound.x - pos0.x) / motion.x;
        tExitX = (maxBound.x - pos0.x) / motion.x;
      } else if (motion.x < 0.0f) {
        tEntryX = (maxBound.x - pos0.x) / motion.x;
        tExitX = (minBound.x - pos0.x) / motion.x;
      } else {
        if (pos0.x <= minBound.x || pos0.x >= maxBound.x) continue;
        tEntryX = -std::numeric_limits<float>::infinity();
        tExitX = std::numeric_limits<float>::infinity();
      }

      if (motion.y > 0.0f) {
        tEntryY = (minBound.y - pos0.y) / motion.y;
        tExitY = (maxBound.y - pos0.y) / motion.y;
      } else if (motion.y < 0.0f) {
        tEntryY = (maxBound.y - pos0.y) / motion.y;
        tExitY = (minBound.y - pos0.y) / motion.y;
      } else {
        if (pos0.y <= minBound.y || pos0.y >= maxBound.y) continue;
        tEntryY = -std::numeric_limits<float>::infinity();
        tExitY = std::numeric_limits<float>::infinity();
      }

      float entryTime = std::max(tEntryX, tEntryY);
      float exitTime = std::min(tExitX, tExitY);

      // Check if collision occurs along the segment [0, 1]
      if (entryTime > exitTime || entryTime < 0.0f || entryTime >= earliestTime) {
        continue;
      }

      earliestTime = entryTime;
      bestCollider = other;

      if (tEntryX > tEntryY) {
        bestNormal = (motion.x < 0.0f) ? Vector2(1.0f, 0.0f) : Vector2(-1.0f, 0.0f);
      } else {
        bestNormal = (motion.y < 0.0f) ? Vector2(0.0f, 1.0f) : Vector2(0.0f, -1.0f);
      }
    }

    if (bestCollider != nullptr && earliestTime < 1.0f) {
      result.collided = true;
      result.travel = motion * earliestTime;
      result.remainder = motion * (1.0f - earliestTime);
      result.normal = bestNormal;
      result.collider = bestCollider;
      result.position = pos0 + result.travel;

      if (!testOnly) {
        transform.position += result.travel;
      }
      return result;
    }

    // No collision: move entire distance
    if (!testOnly) {
      transform.position += motion;
    }
    result.travel = motion;
    return result;
  }

  // Moves the character body based on current velocity, sliding along surfaces
  bool moveAndSlide() {
    float dt = Time::getFixedDeltaTime();
    if (dt <= 0.0f) dt = 1.0f / 60.0f;

    ensureBody();

    m_slideCollisions.clear();
    m_isOnFloor = false;
    m_isOnWall = false;
    m_isOnCeiling = false;
    m_floorNormal = {0.0f, -1.0f};
    m_wallNormal = {0.0f, 0.0f};

    Vector2 motion = velocity * dt;
    bool hasCollided = false;

    for (int slide = 0; slide < maxSlides; ++slide) {
      if (motion.length_squared() < 0.0001f) break;

      KinematicCollision2D col = moveAndCollide(motion);
      if (!col.collided) {
        break; // Moved safely without collision
      }

      hasCollided = true;
      m_slideCollisions.push_back(col);

      // Determine surface type based on collision normal and upDirection
      float dotUp = col.normal.dot(upDirection);
      if (dotUp >= 0.7f) {
        // Floor contact (surface normal pointing up, aligned with upDirection)
        m_isOnFloor = true;
        m_floorNormal = col.normal;
        if (velocity.y > 0.0f) {
          velocity.y = 0.0f;
        }
      } else if (dotUp <= -0.7f) {
        // Ceiling contact (surface normal pointing down, opposite to upDirection)
        m_isOnCeiling = true;
        if (velocity.y < 0.0f) {
          velocity.y = 0.0f;
        }
      } else {
        // Wall contact (surface normal horizontal)
        m_isOnWall = true;
        m_wallNormal = col.normal;
      }

      // Push dynamic RigidBody2D objects (only horizontally from the side)
      if (auto *rb = dynamic_cast<RigidBody2D *>(col.collider)) {
        if (std::abs(col.normal.x) > 0.7f && std::abs(velocity.x) > 0.0f) {
          Vector2 pushDir(-col.normal.x, 0.0f);
          float pushSpeed = std::abs(velocity.x) * 0.8f;
          rb->setLinearVelocity({pushDir.x * pushSpeed, rb->getLinearVelocity().y});
        }
      }

      // Slide remaining motion and velocity along the collision surface
      motion = col.remainder.slide(col.normal);
      velocity = velocity.slide(col.normal);
    }

    // Downward ground probe to stay snapped to floors/platforms
    if (motionMode == MotionMode::Grounded && !m_isOnFloor && velocity.y >= 0.0f) {
      probeFloor();
    }

    // Sync Box2D body transform for sensor queries and trigger detections
    if (isBodyValid()) {
      b2Body_SetTransform(m_bodyId, PhysicsServer2D::toMeters(transform.position), b2MakeRot(0.0f));
    }

    return hasCollided || m_isOnFloor;
  }

protected:
  void probeFloor() {
    Vector2 probeMotion = -upDirection * floorSnapLength;
    KinematicCollision2D probeCol = moveAndCollide(probeMotion, true);
    if (probeCol.collided) {
      float dotUp = probeCol.normal.dot(upDirection);
      if (dotUp >= 0.7f) {
        m_isOnFloor = true;
        m_floorNormal = probeCol.normal;
        // Snap player position flush onto the floor surface
        transform.position += probeCol.travel;
        if (velocity.y > 0.0f) {
          velocity.y = 0.0f;
        }
      }
    }
  }

  Vector2 getHalfExtents() const override {
    if (shapeType == CollisionShapeType::Circle) {
      return {shapeRadius, shapeRadius};
    }
    return shapeSize * 0.5f;
  }

  void createBody() override {
    b2WorldId worldId = PhysicsServer2D::getWorldId();
    if (!b2World_IsValid(worldId)) return;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_kinematicBody; // Kinematic body for Godot Character Controller
    bodyDef.fixedRotation = true;
    bodyDef.position = PhysicsServer2D::toMeters(transform.position);
    bodyDef.rotation = b2MakeRot(transform.rotation);
    bodyDef.userData = this;

    m_bodyId = b2CreateBody(worldId, &bodyDef);
  }

private:
  bool m_isOnFloor = false;
  bool m_isOnWall = false;
  bool m_isOnCeiling = false;
  Vector2 m_floorNormal{0.0f, -1.0f};
  Vector2 m_wallNormal{0.0f, 0.0f};
  std::vector<KinematicCollision2D> m_slideCollisions;
};
