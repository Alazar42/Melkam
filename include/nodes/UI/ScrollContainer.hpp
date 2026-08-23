#pragma once

#include "core/Memory.hpp"
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
  Color scrollbarThumbHoverColor = Color::from_rgba8(120, 130, 160, 240);

  ScrollContainer() : Container("ScrollContainer") {
    clipContents = true;
    mouseFilter = MouseFilter::Stop;
  }

  void onGuiInput(const InputEvent &event) override {
    Rect2 rect = getGlobalRect();

    if (event.type == InputEventType::MouseWheel) {
      scrollVertical -= event.mouseScroll.y * 30.0f;
      scrollVertical = std::max(0.0f, scrollVertical);
      queueSort();
      const_cast<InputEvent &>(event).setHandled();
    } else if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left) {
      if (event.isPressed()) {
        float trackX = rect.position.x + rect.size.x - scrollbarThickness;
        Rect2 vBarRect(trackX, rect.position.y, scrollbarThickness, rect.size.y);

        if (verticalScrollbar && vBarRect.hasPoint(event.mousePosition)) {
          m_isDraggingV = true;
          m_dragStartMouseY = event.mousePosition.y;
          m_dragStartScrollY = scrollVertical;
          const_cast<InputEvent &>(event).setHandled();
        }
      } else {
        m_isDraggingV = false;
      }
    } else if (event.type == InputEventType::MouseMotion && m_isDraggingV) {
      float delta = event.mousePosition.y - m_dragStartMouseY;
      float maxScrollY = std::max(0.0f, m_contentHeight - rect.size.y);
      if (maxScrollY > 0.0f) {
        float scrollRatio = delta / std::max(1.0f, rect.size.y);
        scrollVertical = std::clamp(m_dragStartScrollY + scrollRatio * m_contentHeight, 0.0f, maxScrollY);
        queueSort();
      }
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();
    m_contentHeight = 0.0f;
    m_contentWidth = 0.0f;

    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (!ctrl || !ctrl->visible) continue;

      Vector2 childMin = ctrl->customMinimumSize;
      float childW = std::max(rect.size.x, childMin.x);
      float childH = std::max(rect.size.y, childMin.y);

      m_contentWidth = std::max(m_contentWidth, childW);
      m_contentHeight = std::max(m_contentHeight, childH);

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
    if (verticalScrollbar && m_contentHeight > rect.size.y) {
      float trackX = rect.position.x + rect.size.x - scrollbarThickness;
      Vector2 trackPos{trackX, rect.position.y};
      Vector2 trackSize{scrollbarThickness, rect.size.y};

      Renderer2D::drawRoundedRectScreen(trackPos, trackSize, scrollbarThickness * 0.5f,
                                        scrollbarTrackColor * modulate);

      // Proportional Thumb
      float thumbH = std::clamp((rect.size.y / m_contentHeight) * rect.size.y, 20.0f, rect.size.y);
      float maxScrollY = m_contentHeight - rect.size.y;
      float ratio = (maxScrollY > 0.0f) ? (scrollVertical / maxScrollY) : 0.0f;
      float thumbY = rect.position.y + (rect.size.y - thumbH) * ratio;

      Vector2 mousePos = Input::getMousePosition();
      Rect2 thumbRect(trackX, thumbY, scrollbarThickness, thumbH);
      Color activeThumbColor = (m_isDraggingV || thumbRect.hasPoint(mousePos)) ? scrollbarThumbHoverColor : scrollbarThumbColor;

      Renderer2D::drawRoundedRectScreen(Vector2(trackX, thumbY),
                                        Vector2(scrollbarThickness, thumbH),
                                        scrollbarThickness * 0.5f,
                                        activeThumbColor * modulate);
    }
  }

private:
  float m_contentHeight = 0.0f;
  float m_contentWidth = 0.0f;
  bool m_isDraggingV = false;
  float m_dragStartMouseY = 0.0f;
  float m_dragStartScrollY = 0.0f;
};

