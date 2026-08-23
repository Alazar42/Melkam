#pragma once

#include "nodes/2D/Node2D.hpp"
#include "physics/2D/PhysicsServer2D.hpp"
#include "renderers/Renderer2D.hpp"
#include <box2d/box2d.h>
#include <algorithm>
#include <vector>

enum class CollisionPolygon2DBuildMode {
  Solids,
  Segments
};

// Arbitrary 2D Polygon Collider Node (inspired by Godot CollisionPolygon2D)
class CollisionPolygon2D : public Node2D {
public:
  std::vector<Vector2> polygon;
  CollisionPolygon2DBuildMode buildMode = CollisionPolygon2DBuildMode::Solids;
  bool disabled = false;

  CollisionPolygon2D() : Node2D("CollisionPolygon2D") {}

  explicit CollisionPolygon2D(std::vector<Vector2> points)
      : Node2D("CollisionPolygon2D"), polygon(std::move(points)) {}

  void setPolygon(std::vector<Vector2> points) {
    polygon = std::move(points);
  }

  void onReady() override;

  b2ShapeId createBox2DShape(b2BodyId bodyId, const b2ShapeDef &shapeDef) {
    if (!b2Body_IsValid(bodyId) || polygon.size() < 3) return b2_nullShapeId;

    std::vector<b2Vec2> b2Points;
    b2Points.reserve(polygon.size());
    for (const auto &pt : polygon) {
      b2Points.push_back(PhysicsServer2D::toMeters(pt));
    }

    b2Hull hull = b2ComputeHull(b2Points.data(), static_cast<int32_t>(b2Points.size()));
    b2Polygon poly = b2MakePolygon(&hull, 0.0f);
    m_shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &poly);
    return m_shapeId;
  }

  void onDraw() override {
    if (!PhysicsServer2D::isDebugCollisions() || disabled || polygon.size() < 2) return;

    Transform2D global = getGlobalTransform();
    Color debugColor = disabled ? Color::from_rgba8(120, 120, 120, 120)
                                : Color::from_rgba8(80, 220, 120, 150);

    std::vector<Vector2> worldPts;
    worldPts.reserve(polygon.size());
    for (const auto &p : polygon) {
      worldPts.push_back(global.transformPoint(p));
    }
    Renderer2D::drawLines(worldPts, debugColor, true);
  }

private:
  b2ShapeId m_shapeId = b2_nullShapeId;
};
