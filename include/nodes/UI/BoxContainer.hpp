#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include <algorithm>
#include <memory>
#include <vector>

enum class BoxAlignment {
  Begin,
  Center,
  End
};

// Base Container Node (inspired by Godot Container) managing children layouts.
class Container : public Control {
public:
  Container() : Control("Container") {
    mouseFilter = MouseFilter::Pass;
  }
  explicit Container(std::string name) : Control(std::move(name)) {
    mouseFilter = MouseFilter::Pass;
  }

  void onProcess(float delta) override {
    (void)delta;
    queueSort();
  }

  virtual void queueSort() {
    fitChildControls();
  }

  virtual void fitChildControls() {}
};

// Linear Box Container (inspired by Godot BoxContainer) arranging items with separation & size flags.
class BoxContainer : public Container {
public:
  float separation = 8.0f;
  bool vertical = false;
  BoxAlignment alignment = BoxAlignment::Begin;

  BoxContainer() : Container("BoxContainer") {}
  explicit BoxContainer(bool isVertical, float sep = 8.0f)
      : Container("BoxContainer"), separation(sep), vertical(isVertical) {}

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();

    // 1. Gather visible child controls and compute total min size & expand weights
    std::vector<Control *> visibleChildren;
    float totalMinSize = 0.0f;
    float totalStretchRatio = 0.0f;

    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (!ctrl || !ctrl->visible) continue;

      visibleChildren.push_back(ctrl.get());
      Vector2 minSize = ctrl->customMinimumSize;

      if (vertical) {
        float minH = (minSize.y > 0.0f) ? minSize.y : 24.0f;
        totalMinSize += minH;
        if (static_cast<int>(ctrl->sizeFlagsVertical) & static_cast<int>(SizeFlags::Expand)) {
          totalStretchRatio += std::max(0.1f, ctrl->sizeFlagsStretchRatio);
        }
      } else {
        float minW = (minSize.x > 0.0f) ? minSize.x : 24.0f;
        totalMinSize += minW;
        if (static_cast<int>(ctrl->sizeFlagsHorizontal) & static_cast<int>(SizeFlags::Expand)) {
          totalStretchRatio += std::max(0.1f, ctrl->sizeFlagsStretchRatio);
        }
      }
    }

    if (visibleChildren.empty()) return;

    float totalGaps = (visibleChildren.size() > 1) ? (visibleChildren.size() - 1) * separation : 0.0f;
    float availableSpace = vertical ? (rect.size.y - totalGaps - totalMinSize) : (rect.size.x - totalGaps - totalMinSize);
    availableSpace = std::max(0.0f, availableSpace);

    float offset = 0.0f;
    if (totalStretchRatio == 0.0f) {
      if (alignment == BoxAlignment::Center) {
        offset = availableSpace * 0.5f;
      } else if (alignment == BoxAlignment::End) {
        offset = availableSpace;
      }
    }

    for (auto *ctrl : visibleChildren) {
      Vector2 minSize = ctrl->customMinimumSize;

      if (vertical) {
        float childH = (minSize.y > 0.0f) ? minSize.y : 24.0f;
        if (totalStretchRatio > 0.0f && (static_cast<int>(ctrl->sizeFlagsVertical) & static_cast<int>(SizeFlags::Expand))) {
          float extra = (std::max(0.1f, ctrl->sizeFlagsStretchRatio) / totalStretchRatio) * availableSpace;
          childH += extra;
        }

        ctrl->offsetLeft = 0.0f;
        ctrl->offsetTop = offset;
        ctrl->offsetRight = rect.size.x;
        ctrl->offsetBottom = offset + childH;
        offset += childH + separation;
      } else {
        float childW = (minSize.x > 0.0f) ? minSize.x : 24.0f;
        if (totalStretchRatio > 0.0f && (static_cast<int>(ctrl->sizeFlagsHorizontal) & static_cast<int>(SizeFlags::Expand))) {
          float extra = (std::max(0.1f, ctrl->sizeFlagsStretchRatio) / totalStretchRatio) * availableSpace;
          childW += extra;
        }

        ctrl->offsetLeft = offset;
        ctrl->offsetTop = 0.0f;
        ctrl->offsetRight = offset + childW;
        ctrl->offsetBottom = rect.size.y;
        offset += childW + separation;
      }
    }
  }
};

// Horizontal Box Container (arranges child Controls in a row)
class HBoxContainer : public BoxContainer {
public:
  explicit HBoxContainer(float sep = 8.0f) : BoxContainer(false, sep) {
    name = "HBoxContainer";
  }
};

// Vertical Box Container (arranges child Controls in a column)
class VBoxContainer : public BoxContainer {
public:
  explicit VBoxContainer(float sep = 8.0f) : BoxContainer(true, sep) {
    name = "VBoxContainer";
  }
};

// Margin Container adding padding around its child Control
class MarginContainer : public Container {
public:
  float marginLeft = 8.0f;
  float marginTop = 8.0f;
  float marginRight = 8.0f;
  float marginBottom = 8.0f;

  MarginContainer() : Container("MarginContainer") {}

  void setAllMargins(float margin) {
    marginLeft = margin;
    marginTop = margin;
    marginRight = margin;
    marginBottom = margin;
  }

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();
    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (!ctrl || !ctrl->visible) continue;

      ctrl->offsetLeft = marginLeft;
      ctrl->offsetTop = marginTop;
      ctrl->offsetRight = std::max(marginLeft, rect.size.x - marginRight);
      ctrl->offsetBottom = std::max(marginTop, rect.size.y - marginBottom);
    }
  }
};

