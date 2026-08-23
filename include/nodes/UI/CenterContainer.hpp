#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/AspectRatioContainer.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include <algorithm>

// Center Container (inspired by Godot CenterContainer) centering child controls inside its bounds.
class CenterContainer : public Container {
public:
  bool useTopLeft = false;

  CenterContainer() : Container("CenterContainer") {}

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();

    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (!ctrl || !ctrl->visible) continue;

      Vector2 childSize = ctrl->customMinimumSize;
      if (childSize.x <= 0.0f) childSize.x = 100.0f;
      if (childSize.y <= 0.0f) childSize.y = 40.0f;

      float posX = (rect.size.x - childSize.x) * 0.5f;
      float posY = (rect.size.y - childSize.y) * 0.5f;

      ctrl->offsetLeft = posX;
      ctrl->offsetTop = posY;
      ctrl->offsetRight = posX + childSize.x;
      ctrl->offsetBottom = posY + childSize.y;
    }
  }
};

