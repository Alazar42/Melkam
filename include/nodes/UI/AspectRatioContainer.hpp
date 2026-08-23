#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include <algorithm>

enum class AspectStretchMode {
  Fit,
  Fill,
  WidthControlsHeight,
  HeightControlsWidth
};

// Container constraining child controls to a fixed aspect ratio (inspired by Godot AspectRatioContainer)
class AspectRatioContainer : public Container {
public:
  float ratio = 1.0f; // Aspect ratio (width / height)
  AspectStretchMode stretchMode = AspectStretchMode::Fit;
  BoxAlignment alignmentHorizontal = BoxAlignment::Center;
  BoxAlignment alignmentVertical = BoxAlignment::Center;

  AspectRatioContainer() : Container("AspectRatioContainer") {}
  explicit AspectRatioContainer(float targetRatio)
      : Container("AspectRatioContainer"), ratio(targetRatio) {}

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();
    if (rect.size.x <= 0.0f || rect.size.y <= 0.0f || ratio <= 0.0f) return;

    float targetW = rect.size.x;
    float targetH = rect.size.y;

    if (stretchMode == AspectStretchMode::Fit) {
      if (targetW / targetH > ratio) {
        targetW = targetH * ratio;
      } else {
        targetH = targetW / ratio;
      }
    } else if (stretchMode == AspectStretchMode::Fill) {
      if (targetW / targetH < ratio) {
        targetW = targetH * ratio;
      } else {
        targetH = targetW / ratio;
      }
    } else if (stretchMode == AspectStretchMode::WidthControlsHeight) {
      targetH = targetW / ratio;
    } else if (stretchMode == AspectStretchMode::HeightControlsWidth) {
      targetW = targetH * ratio;
    }

    float left = 0.0f;
    if (alignmentHorizontal == BoxAlignment::Center) left = (rect.size.x - targetW) * 0.5f;
    else if (alignmentHorizontal == BoxAlignment::End) left = rect.size.x - targetW;

    float top = 0.0f;
    if (alignmentVertical == BoxAlignment::Center) top = (rect.size.y - targetH) * 0.5f;
    else if (alignmentVertical == BoxAlignment::End) top = rect.size.y - targetH;

    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (!ctrl || !ctrl->visible) continue;

      ctrl->offsetLeft = left;
      ctrl->offsetTop = top;
      ctrl->offsetRight = left + targetW;
      ctrl->offsetBottom = top + targetH;
    }
  }
};
