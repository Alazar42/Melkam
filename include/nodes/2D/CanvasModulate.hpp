#pragma once

#include "core/Node.hpp"
#include "helper/color/Color.hpp"
#include "renderers/Renderer2D.hpp"
#include "window.hpp"

// Global 2D Canvas Ambient Tinting Node (inspired by Godot CanvasModulate)
class CanvasModulate : public Node {
public:
  Color color = Color::WHITE;

  CanvasModulate() : Node("CanvasModulate") {}
  explicit CanvasModulate(Color modulateColor) : Node("CanvasModulate"), color(modulateColor) {}

  void onDraw() override {
    if (color == Color::WHITE || !visible) return;

    // Draws a screen-space ambient lighting tint layer
    Vector2 vpSize = Window::getViewportSize();
    Renderer2D::drawRectScreen(Vector2(0.0f, 0.0f), vpSize, color, true);
  }
};
