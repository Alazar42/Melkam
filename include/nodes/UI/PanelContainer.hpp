#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include "nodes/UI/StyleBox.hpp"
#include <algorithm>

// Container with styled panel background wrapping its child controls (inspired by Godot PanelContainer)
class PanelContainer : public Container {
public:
  Ref<StyleBox> styleBox = nullptr;

  PanelContainer() : Container("PanelContainer") {
    mouseFilter = MouseFilter::Pass;
  }

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();

    float ml = 6.0f, mt = 6.0f, mr = 6.0f, mb = 6.0f;
    Ref<StyleBox> activeStyle = styleBox ? styleBox : getThemeStylebox("panel", "Panel");
    if (activeStyle) {
      if (activeStyle->contentMarginLeft >= 0.0f) ml = activeStyle->contentMarginLeft;
      if (activeStyle->contentMarginTop >= 0.0f) mt = activeStyle->contentMarginTop;
      if (activeStyle->contentMarginRight >= 0.0f) mr = activeStyle->contentMarginRight;
      if (activeStyle->contentMarginBottom >= 0.0f) mb = activeStyle->contentMarginBottom;
    }

    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (!ctrl || !ctrl->visible) continue;

      ctrl->offsetLeft = ml;
      ctrl->offsetTop = mt;
      ctrl->offsetRight = std::max(ml, rect.size.x - mr);
      ctrl->offsetBottom = std::max(mt, rect.size.y - mb);
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    Ref<StyleBox> activeStyle = styleBox ? styleBox : getThemeStylebox("panel", "Panel");
    if (activeStyle) {
      activeStyle->draw(rect, modulate);
    } else {
      Color bg = getThemeColor("bg_color", "Panel", Color::from_rgba8(25, 28, 38, 230));
      Color border = getThemeColor("border_color", "Panel", Color::from_rgba8(65, 75, 105));
      Renderer2D::drawRoundedRectScreen(rect.position, rect.size, 6.0f, bg * modulate, border * modulate, 1.0f);
    }
  }
};
