#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include <algorithm>
#include <vector>

// Multi-Column Grid Layout Container (inspired by Godot GridContainer).
class GridContainer : public Container {
public:
  int columns = 2;
  float horizontalSeparation = 8.0f;
  float verticalSeparation = 8.0f;

  GridContainer() : Container("GridContainer") {}
  explicit GridContainer(int colCount, float hSep = 8.0f, float vSep = 8.0f)
      : Container("GridContainer"), columns(std::max(1, colCount)),
        horizontalSeparation(hSep), verticalSeparation(vSep) {}

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();
    int cols = std::max(1, columns);

    // Calculate visible children
    std::vector<Ref<Control>> visibleControls;
    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (ctrl && ctrl->visible) {
        visibleControls.push_back(ctrl);
      }
    }

    if (visibleControls.empty()) return;

    float totalHSep = (cols - 1) * horizontalSeparation;
    float cellW = std::max(10.0f, (rect.size.x - totalHSep) / static_cast<float>(cols));
    float cellH = 36.0f; // Default row height

    for (size_t i = 0; i < visibleControls.size(); ++i) {
      int col = static_cast<int>(i % cols);
      int row = static_cast<int>(i / cols);

      float x = col * (cellW + horizontalSeparation);
      float y = row * (cellH + verticalSeparation);

      auto &ctrl = visibleControls[i];
      ctrl->offsetLeft = x;
      ctrl->offsetTop = y;
      ctrl->offsetRight = x + cellW;
      ctrl->offsetBottom = y + cellH;
    }
  }
};

