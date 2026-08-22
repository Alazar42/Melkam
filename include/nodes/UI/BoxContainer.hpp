#pragma once

#include "nodes/UI/Control.hpp"
#include <algorithm>
#include <vector>

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

// Linear Box Container (inspired by Godot BoxContainer) arranging items with separation.
class BoxContainer : public Container {
public:
  float separation = 8.0f;
  bool vertical = false;

  BoxContainer() : Container("BoxContainer") {}
  explicit BoxContainer(bool isVertical, float sep = 8.0f)
      : Container("BoxContainer"), separation(sep), vertical(isVertical) {}

  void fitChildControls() override {
    float offset = 0.0f;
    Rect2 rect = getGlobalRect();

    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (!ctrl || !ctrl->visible) continue;

      Vector2 minSize = ctrl->customMinimumSize;

      if (vertical) {
        float childH = (minSize.y > 0.0f) ? minSize.y : 32.0f;
        ctrl->offsetLeft = 0.0f;
        ctrl->offsetTop = offset;
        ctrl->offsetRight = rect.size.x;
        ctrl->offsetBottom = offset + childH;
        offset += childH + separation;
      } else {
        float childW = (minSize.x > 0.0f) ? minSize.x : 100.0f;
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
