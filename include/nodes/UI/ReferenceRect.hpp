#pragma once

#include "nodes/UI/Control.hpp"

// Debug Boundary Rectangle UI Node (inspired by Godot ReferenceRect).
class ReferenceRect : public Control {
public:
  Color borderColor = Color::from_rgba8(255, 0, 100);
  float borderWidth = 1.0f;
  bool editorOnly = false;

  ReferenceRect() : Control("ReferenceRect") {
    mouseFilter = MouseFilter::Ignore;
  }

  explicit ReferenceRect(const Color &color, float width = 1.0f)
      : Control("ReferenceRect"), borderColor(color), borderWidth(width) {
    mouseFilter = MouseFilter::Ignore;
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    Renderer2D::drawRectScreen(rect.position, rect.size, borderColor * modulate, false);
  }
};
