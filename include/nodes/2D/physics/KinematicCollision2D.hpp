#pragma once

#include "helper/vectors/Vector2.hpp"
#include <cmath>

class Node2D;
class CollisionShape2D;

// Collision result data returned by CharacterBody2D movement (inspired by Godot's KinematicCollision2D).
struct KinematicCollision2D {
  bool collided = false;
  Vector2 position{0.0f, 0.0f};       // Point of contact in world space
  Vector2 normal{0.0f, -1.0f};        // Surface normal at contact point
  Vector2 travel{0.0f, 0.0f};        // Distance traveled before collision
  Vector2 remainder{0.0f, 0.0f};     // Remaining motion vector after collision
  float depth = 0.0f;                 // Penetration depth along normal
  Node2D *collider = nullptr;         // The object collided with
  CollisionShape2D *colliderShape = nullptr;
  Vector2 colliderVelocity{0.0f, 0.0f};

  // Helper queries matching Godot 4 API
  Vector2 getPosition() const { return position; }
  Vector2 getNormal() const { return normal; }
  Vector2 getTravel() const { return travel; }
  Vector2 getRemainder() const { return remainder; }
  float getDepth() const { return depth; }
  float getAngle(const Vector2 &upDirection = {0.0f, -1.0f}) const {
    return std::acos(std::clamp(normal.dot(upDirection), -1.0f, 1.0f));
  }
  Node2D *getCollider() const { return collider; }
};
