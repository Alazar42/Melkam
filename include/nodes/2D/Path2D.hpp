#pragma once

#include "core/Memory.hpp"
#include "helper/color/Color.hpp"
#include "nodes/2D/Curve2D.hpp"
#include "nodes/2D/Node2D.hpp"
#include "renderers/Renderer2D.hpp"
#include <memory>

// 2D Spline Path Spatial Node (inspired by Godot Path2D)
class Path2D : public Node2D {
public:
  Ref<Curve2D> curve = nullptr;
  bool showDebug = false;
  Color debugColor = Color::from_rgba8(75, 180, 255, 180);

  float debugWidth = 2.0f;

  Path2D() : Node2D("Path2D") {
    curve = makeRef<Curve2D>();
  }

  explicit Path2D(Ref<Curve2D> curveResource) : Node2D("Path2D"), curve(std::move(curveResource)) {}

  void onDraw() override {
    if (!showDebug || !curve || curve->getPointCount() < 2) return;

    Transform2D globalTransform = getGlobalTransform();
    const auto &baked = curve->getBakedPoints();
    if (baked.size() < 2) return;

    for (size_t i = 0; i + 1 < baked.size(); ++i) {
      Vector2 p1 = globalTransform.transformPoint(baked[i]);
      Vector2 p2 = globalTransform.transformPoint(baked[i + 1]);
      Renderer2D::drawLine(p1, p2, debugColor, debugWidth);
    }
  }
};
