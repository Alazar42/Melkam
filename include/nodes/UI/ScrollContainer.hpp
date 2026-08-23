#pragma once

#include "nodes/UI/BoxContainer.hpp"
#include <algorithm>

// Scrollable View Container (inspired by Godot ScrollContainer).
class ScrollContainer : public Container {
public:
  float scrollHorizontal = 0.0f;
  float scrollVertical = 0.0f;
  bool horizontalScrollbar = true;
  bool verticalScrollbar = true;
  float scrollbarThickness = 8.0f;

  // Colors & Theme Styling
  Color scrollbarTrackColor = Color::from_rgba8(30, 32, 40, 180);
  Color scrollbarThumbColor = Color::from_rgba8(90, 95, 120, 220);

  ScrollContainer() : Container("ScrollContainer") {
    clipContents = true;
    mouseFilter = MouseFilter::Stop;
  }

  void onGuiInput(const InputEvent &event) override {
    if (event.type == InputEventType::MouseWheel) {
      scrollVertical -= event.mouseScroll.y * 30.0f;
      if (scrollVertical < 0.0f) scrollVertical = 0.0f;
      queueSort();
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();

    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (!ctrl || !ctrl->visible) continue;

      Vector2 childMin = ctrl->customMinimumSize;
      float childW = std::max(rect.size.x, childMin.x);
      float childH = std::max(rect.size.y, childMin.y);

      // Max scroll clamp
      float maxScrollY = std::max(0.0f, childH - rect.size.y);
      scrollVertical = std::clamp(scrollVertical, 0.0f, maxScrollY);

      ctrl->offsetLeft = -scrollHorizontal;
      ctrl->offsetTop = -scrollVertical;
      ctrl->offsetRight = -scrollHorizontal + childW;
      ctrl->offsetBottom = -scrollVertical + childH;
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    // Render vertical scrollbar if content exceeds container height
    if (verticalScrollbar) {
      float trackX = rect.position.x + rect.size.x - scrollbarThickness;
      Vector2 trackPos{trackX, rect.position.y};
      Vector2 trackSize{scrollbarThickness, rect.size.y};

      Renderer2D::drawRoundedRectScreen(trackPos, trackSize, scrollbarThickness * 0.5f,
                                        scrollbarTrackColor * modulate);

      // Thumb
      float thumbH = std::max(20.0f, rect.size.y * 0.4f);
      float thumbY = rect.position.y + (rect.size.y - thumbH) * 0.0f; // proportional
      Renderer2D::drawRoundedRectScreen(Vector2(trackX, thumbY),
                                        Vector2(scrollbarThickness, thumbH),
                                        scrollbarThickness * 0.5f,
                                        scrollbarThumbColor * modulate);
    }
  }
};
