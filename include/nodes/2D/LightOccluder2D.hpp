#pragma once

#include "core/Memory.hpp"
#include "helper/color/Color.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/OccluderPolygon2D.hpp"
#include "renderers/Renderer2D.hpp"

// 2D Light Shadow Caster Node (inspired by Godot LightOccluder2D)
class LightOccluder2D : public Node2D {
public:
  Ref<OccluderPolygon2D> occluder = nullptr;
  int occluderLightMask = 1;
  bool showDebug = false;
  Color debugColor = Color::from_rgba8(255, 60, 60, 160);

  LightOccluder2D() : Node2D("LightOccluder2D") {}

  explicit LightOccluder2D(Ref<OccluderPolygon2D> occluderPolygon)
      : Node2D("LightOccluder2D"), occluder(std::move(occluderPolygon)) {}

  void onDraw() override {
    if (!showDebug || !occluder || occluder->getPointCount() < 2) return;

    Transform2D globalTransform = getGlobalTransform();
    std::vector<Vector2> worldPoints;
    worldPoints.reserve(occluder->polygon.size());

    for (const auto &p : occluder->polygon) {
      worldPoints.push_back(globalTransform.transformPoint(p));
    }

    Renderer2D::drawLines(worldPoints, debugColor, occluder->closed);
  }
};
