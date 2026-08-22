#pragma once

#include "nodes/UI/Control.hpp"

// Styled Background Panel Box Node (inspired by Godot Panel / PanelContainer).
class Panel : public Control {
public:
  Color backgroundColor = Color::from_rgba8(30, 32, 40, 230);
  Color borderColor = Color::from_rgba8(65, 70, 90);
  float borderWidth = 1.0f;
  float cornerRadius = 6.0f;

  Panel() : Control("Panel") {
    mouseFilter = MouseFilter::Pass;
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, cornerRadius,
                                      backgroundColor * modulate, borderColor * modulate,
                                      borderWidth);
  }
};

// Flat Solid Color Rectangle UI Node (inspired by Godot ColorRect).
class ColorRect : public Control {
public:
  Color color = Color::WHITE;

  ColorRect() : Control("ColorRect") {
    mouseFilter = MouseFilter::Pass;
  }

  explicit ColorRect(const Color &rectColor)
      : Control("ColorRect"), color(rectColor) {
    mouseFilter = MouseFilter::Pass;
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    Renderer2D::drawRectScreen(rect.position, rect.size, color * modulate, true);
  }
};
