#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "renderers/Renderer2D.hpp"

// Visual Divider Separator UI Node (inspired by Godot Separator)
class Separator : public Control {
public:
  bool vertical = false;
  float thickness = 1.0f;
  Color color = Color::from_rgba8(75, 80, 105);

  Separator() : Control("Separator") {
    mouseFilter = MouseFilter::Ignore;
    customMinimumSize = {4.0f, 4.0f};
  }

  explicit Separator(bool isVertical, float thick = 1.0f, std::string nodeName = "Separator")
      : Control(std::move(nodeName)), vertical(isVertical), thickness(thick) {
    mouseFilter = MouseFilter::Ignore;
    if (vertical) {
      customMinimumSize = {thickness + 6.0f, 16.0f};
    } else {
      customMinimumSize = {16.0f, thickness + 6.0f};
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    if (vertical) {
      float cx = rect.position.x + (rect.size.x - thickness) * 0.5f;
      Renderer2D::drawRectScreen(Vector2(cx, rect.position.y), Vector2(thickness, rect.size.y),
                                color * modulate, true);
    } else {
      float cy = rect.position.y + (rect.size.y - thickness) * 0.5f;
      Renderer2D::drawRectScreen(Vector2(rect.position.x, cy), Vector2(rect.size.x, thickness),
                                color * modulate, true);
    }
  }
};

// Horizontal Separator Line (inspired by Godot HSeparator)
class HSeparator : public Separator {
public:
  explicit HSeparator(float thick = 1.0f) : Separator(false, thick, "HSeparator") {}
};

// Vertical Separator Line (inspired by Godot VSeparator)
class VSeparator : public Separator {
public:
  explicit VSeparator(float thick = 1.0f) : Separator(true, thick, "VSeparator") {}
};
