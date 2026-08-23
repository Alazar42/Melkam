#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include <algorithm>

// Word-Wrap / Flexbox Layout Container (inspired by Godot FlowContainer)
class FlowContainer : public Container {
public:
  float hSeparation = 8.0f;
  float vSeparation = 8.0f;
  bool vertical = false;

  FlowContainer() : Container("FlowContainer") {}
  explicit FlowContainer(bool isVertical, float hSep = 8.0f, float vSep = 8.0f)
      : Container("FlowContainer"), hSeparation(hSep), vSeparation(vSep), vertical(isVertical) {}

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();
    if (rect.size.x <= 0.0f || rect.size.y <= 0.0f) return;

    if (!vertical) {
      // Horizontal flow (wraps to next row when line width is exceeded)
      float currX = 0.0f;
      float currY = 0.0f;
      float rowHeight = 0.0f;

      for (const auto &child : getChildren()) {
        auto ctrl = std::dynamic_pointer_cast<Control>(child);
        if (!ctrl || !ctrl->visible) continue;

        Vector2 minSize = ctrl->customMinimumSize;
        float itemW = (minSize.x > 0.0f) ? minSize.x : 60.0f;
        float itemH = (minSize.y > 0.0f) ? minSize.y : 30.0f;

        if (currX + itemW > rect.size.x && currX > 0.0f) {
          currX = 0.0f;
          currY += rowHeight + vSeparation;
          rowHeight = 0.0f;
        }

        ctrl->offsetLeft = currX;
        ctrl->offsetTop = currY;
        ctrl->offsetRight = currX + itemW;
        ctrl->offsetBottom = currY + itemH;

        rowHeight = std::max(rowHeight, itemH);
        currX += itemW + hSeparation;
      }
    } else {
      // Vertical flow (wraps to next column when column height is exceeded)
      float currX = 0.0f;
      float currY = 0.0f;
      float colWidth = 0.0f;

      for (const auto &child : getChildren()) {
        auto ctrl = std::dynamic_pointer_cast<Control>(child);
        if (!ctrl || !ctrl->visible) continue;

        Vector2 minSize = ctrl->customMinimumSize;
        float itemW = (minSize.x > 0.0f) ? minSize.x : 60.0f;
        float itemH = (minSize.y > 0.0f) ? minSize.y : 30.0f;

        if (currY + itemH > rect.size.y && currY > 0.0f) {
          currY = 0.0f;
          currX += colWidth + hSeparation;
          colWidth = 0.0f;
        }

        ctrl->offsetLeft = currX;
        ctrl->offsetTop = currY;
        ctrl->offsetRight = currX + itemW;
        ctrl->offsetBottom = currY + itemH;

        colWidth = std::max(colWidth, itemW);
        currY += itemH + vSeparation;
      }
    }
  }
};

// Horizontal Flow Container Node (inspired by Godot HFlowContainer)
class HFlowContainer : public FlowContainer {
public:
  explicit HFlowContainer(float hSep = 8.0f, float vSep = 8.0f)
      : FlowContainer(false, hSep, vSep) {
    name = "HFlowContainer";
  }
};

// Vertical Flow Container Node (inspired by Godot VFlowContainer)
class VFlowContainer : public FlowContainer {
public:
  explicit VFlowContainer(float hSep = 8.0f, float vSep = 8.0f)
      : FlowContainer(true, hSep, vSep) {
    name = "VFlowContainer";
  }
};
