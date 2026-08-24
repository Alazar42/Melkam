#pragma once

#include "nodes/3D/physics/CollisionObject3D.hpp"
#include "nodes/3D/physics/PhysicsServer3D.hpp"
#include <algorithm>
#include <cmath>

// Kinematic 3D Character Controller (inspired by Godot 4 CharacterBody3D)
class CharacterBody3D : public CollisionObject3D {
public:
  Vector3 velocity{0.0f, 0.0f, 0.0f};
  Vector3 upDirection{0.0f, 1.0f, 0.0f};
  float floorMaxAngle = 0.785398f; // 45 degrees in radians
  float maxSlides = 4;
  float characterRadius = 0.4f;
  float characterHeight = 1.8f;

  CharacterBody3D() : CollisionObject3D("CharacterBody3D") {}

  bool moveAndSlide(float delta = 1.0f / 60.0f) {
    return move_and_slide(delta);
  }

  // Godot 4 iconic move_and_slide kinematic movement & collision solver using Bullet 3
  bool move_and_slide(float delta = 1.0f / 60.0f) {
    m_onFloor = false;
    m_onWall = false;
    m_onCeiling = false;

    float halfH = characterHeight * 0.5f;
    Vector3 currentPos = getPosition();

    // 1. Horizontal Motion & Obstacle Collision (Crates, Walls, Pillars)
    Vector3 horizMotion(velocity.x * delta, 0.0f, velocity.z * delta);
    if (horizMotion.length_squared() > 0.00001f) {
      Vector3 horizTarget = currentPos + horizMotion;
      
      // Multi-height probe: knee level and torso level
      Vector3 heightOffsets[2] = {
        Vector3(0.0f, -halfH + 0.35f, 0.0f),
        Vector3(0.0f, 0.1f, 0.0f)
      };

      bool hitObstacle = false;
      Vector3 obstacleNormal;

      for (const auto &hOff : heightOffsets) {
        Vector3 rayStart = currentPos + hOff;
        Vector3 rayEnd = horizTarget + hOff + horizMotion.normalized() * (characterRadius + 0.05f);

        RayCastHit3D hit;
        if (PhysicsServer3D::get().raycast(rayStart, rayEnd, hit)) {
          float upDot = hit.normal.dot(upDirection);
          if (std::abs(upDot) <= std::cos(floorMaxAngle)) {
            hitObstacle = true;
            obstacleNormal = hit.normal;
            break;
          }
        }
      }

      if (hitObstacle) {
        m_onWall = true;
        Vector3 wallNormXZ = Vector3(obstacleNormal.x, 0.0f, obstacleNormal.z);
        if (wallNormXZ.length_squared() > 0.001f) {
          wallNormXZ = wallNormXZ.normalized();
          horizMotion = horizMotion - wallNormXZ * horizMotion.dot(wallNormXZ);
        }
        currentPos = currentPos + horizMotion;
      } else {
        currentPos = horizTarget;
      }
    }

    // 2. Vertical Ground Detection & Snapping (Prevents Jitter & Bouncing)
    float r = characterRadius * 0.55f;
    float snapDist = (velocity.y <= 0.05f) ? (std::abs(velocity.y * delta) + 0.25f) : 0.0f;

    Vector3 rayOffsets[5] = {
      Vector3(0.0f, 0.0f, 0.0f),
      Vector3(r, 0.0f, r),
      Vector3(-r, 0.0f, r),
      Vector3(r, 0.0f, -r),
      Vector3(-r, 0.0f, -r)
    };

    bool foundFloor = false;
    float highestFloorY = -99999.0f;

    if (velocity.y <= 0.05f) {
      for (const auto &off : rayOffsets) {
        Vector3 rayStart = currentPos + off;
        Vector3 rayEnd = currentPos + off - Vector3(0.0f, halfH + snapDist, 0.0f);

        RayCastHit3D hit;
        if (PhysicsServer3D::get().raycast(rayStart, rayEnd, hit)) {
          float floorCos = hit.normal.dot(upDirection);
          if (floorCos > std::cos(floorMaxAngle)) {
            if (hit.point.y > highestFloorY) {
              highestFloorY = hit.point.y;
              foundFloor = true;
            }
          }
        }
      }
    }

    if (foundFloor) {
      m_onFloor = true;
      velocity.y = 0.0f;
      currentPos.y = highestFloorY + halfH;
    } else {
      // Free falling through air / off edge into void
      m_onFloor = false;
      currentPos.y += velocity.y * delta;
    }

    setPosition(currentPos);
    return m_onFloor || m_onWall;
  }

  bool isOnFloor() const { return m_onFloor; }
  bool isOnWall() const { return m_onWall; }
  bool isOnCeiling() const { return m_onCeiling; }

  bool is_on_floor() const { return isOnFloor(); }
  bool is_on_wall() const { return isOnWall(); }
  bool is_on_ceiling() const { return isOnCeiling(); }

private:
  bool m_onFloor = false;
  bool m_onWall = false;
  bool m_onCeiling = false;
};
