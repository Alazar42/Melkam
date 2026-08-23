#pragma once

#include "helper/color/Color.hpp"
#include "nodes/2D/Node2D.hpp"
#include "renderers/Renderer2D.hpp"

// 2D Spatial Position / Socket / Anchor Marker Node (inspired by Godot Marker2D / Position2D)
class Marker2D : public Node2D {
public:
  float gizmoExtents = 12.0f;
  Color gizmoColor = Color::from_rgba8(255, 75, 75, 220);
  bool showGizmo = false;


  Marker2D() : Node2D("Marker2D") {}

  void onDraw() override {
    if (!showGizmo) return;

    Transform2D globalTransform = getGlobalTransform();
    Vector2 p = globalTransform.position;
    float ext = gizmoExtents * globalTransform.scale.x;

    // Crosshair gizmo
    Renderer2D::drawLine(Vector2(p.x - ext, p.y), Vector2(p.x + ext, p.y), gizmoColor, 1.5f);
    Renderer2D::drawLine(Vector2(p.x, p.y - ext), Vector2(p.x, p.y + ext), gizmoColor, 1.5f);
  }
};

using Position2D = Marker2D;
