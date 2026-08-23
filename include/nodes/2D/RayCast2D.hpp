#pragma once

#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/physics/CollisionObject2D.hpp"
#include "physics/2D/PhysicsServer2D.hpp"
#include "renderers/Renderer2D.hpp"
#include <box2d/box2d.h>

// 2D Spatial Physics Ray Cast Node (inspired by Godot RayCast2D)
class RayCast2D : public Node2D {
public:
  Vector2 targetPosition{0.0f, 50.0f};
  bool enabled = true;
  bool hitFromInside = false;
  uint32_t collisionMask = 0xFFFFFFFF;
  bool showDebug = true;
  Color debugHitColor = Color::from_rgba8(255, 60, 60, 220);
  Color debugMissColor = Color::from_rgba8(60, 220, 100, 200);

  RayCast2D() : Node2D("RayCast2D") {}

  explicit RayCast2D(const Vector2 &target) : Node2D("RayCast2D"), targetPosition(target) {}

  bool isColliding() const {
    return m_isColliding;
  }

  Vector2 getCollisionPoint() const {
    return m_collisionPoint;
  }

  Vector2 getCollisionNormal() const {
    return m_collisionNormal;
  }

  CollisionObject2D *getCollider() const {
    return m_collider;
  }

  void forceRaycastUpdate() {
    if (!enabled) return;

    b2WorldId worldId = PhysicsServer2D::getWorldId();
    if (!b2World_IsValid(worldId)) {
      m_isColliding = false;
      return;
    }

    Transform2D globalTrans = getGlobalTransform();
    Vector2 originPx = globalTrans.position;
    Vector2 targetPx = globalTrans.transformPoint(targetPosition);
    Vector2 translationPx = targetPx - originPx;

    b2Vec2 originMeters = PhysicsServer2D::toMeters(originPx);
    b2Vec2 translationMeters = PhysicsServer2D::toMeters(translationPx);

    b2QueryFilter filter = b2DefaultQueryFilter();
    filter.maskBits = collisionMask;

    b2RayResult result = b2World_CastRayClosest(worldId, originMeters, translationMeters, filter);

    if (result.hit) {
      m_isColliding = true;
      m_collisionPoint = PhysicsServer2D::toPixels(result.point);
      m_collisionNormal = Vector2(result.normal.x, result.normal.y);

      // Find collider associated with body
      b2BodyId hitBodyId = b2Shape_GetBody(result.shapeId);
      m_collider = static_cast<CollisionObject2D *>(b2Body_GetUserData(hitBodyId));
    } else {
      m_isColliding = false;
      m_collisionPoint = targetPx;
      m_collisionNormal = {0.0f, 0.0f};
      m_collider = nullptr;
    }
  }

  void onPhysicsProcess(float delta) override {
    (void)delta;
    if (enabled) {
      forceRaycastUpdate();
    }
  }

  void onDraw() override {
    if (!showDebug || !enabled) return;

    Transform2D globalTrans = getGlobalTransform();
    Vector2 originPx = globalTrans.position;
    Vector2 targetPx = globalTrans.transformPoint(targetPosition);

    if (m_isColliding) {
      Renderer2D::drawLine(originPx, m_collisionPoint, debugHitColor, 2.0f);
      // Draw normal
      Renderer2D::drawLine(m_collisionPoint, m_collisionPoint + m_collisionNormal * 16.0f, Color::GOLD, 2.0f);
      Renderer2D::drawCircle(m_collisionPoint, 4.0f, debugHitColor, true);
    } else {
      Renderer2D::drawLine(originPx, targetPx, debugMissColor, 1.5f);
    }
  }

private:
  bool m_isColliding = false;
  Vector2 m_collisionPoint{0.0f, 0.0f};
  Vector2 m_collisionNormal{0.0f, 0.0f};
  CollisionObject2D *m_collider = nullptr;
};
