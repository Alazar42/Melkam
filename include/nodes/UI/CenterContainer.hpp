#pragma once

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

// Aspect Ratio Container (inspired by Godot AspectRatioContainer) preserving child aspect ratio.
class AspectRatioContainer : public Container {
public:
  float ratio = 1.0f; // Aspect ratio (width / height)

  AspectRatioContainer() : Container("AspectRatioContainer") {}
  explicit AspectRatioContainer(float targetRatio)
      : Container("AspectRatioContainer"), ratio(targetRatio) {}

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();
    if (rect.size.y <= 0.0f || ratio <= 0.0f) return;

    float containerRatio = rect.size.x / rect.size.y;
    float fitW = rect.size.x;
    float fitH = rect.size.y;

    if (containerRatio > ratio) {
      fitW = rect.size.y * ratio;
    } else {
      fitH = rect.size.x / ratio;
    }

    float posX = (rect.size.x - fitW) * 0.5f;
    float posY = (rect.size.y - fitH) * 0.5f;

    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (!ctrl || !ctrl->visible) continue;

      ctrl->offsetLeft = posX;
      ctrl->offsetTop = posY;
      ctrl->offsetRight = posX + fitW;
      ctrl->offsetBottom = posY + fitH;
    }
  }
};
