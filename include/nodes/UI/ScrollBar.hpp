#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Range.hpp"
#include "renderers/Renderer2D.hpp"
#include <algorithm>

// Base ScrollBar UI Node (inspired by Godot ScrollBar)
class ScrollBar : public Range {
public:
  bool vertical = false;
  float pageSize = 0.0f; // Viewport page size (adjusts grabber size proportionally)

  Color trackColor = Color::from_rgba8(30, 32, 42);
  Color grabberColor = Color::from_rgba8(80, 85, 110);
  Color grabberHoverColor = Color::from_rgba8(110, 120, 150);
  Color grabberPressedColor = Color::from_rgba8(60, 130, 246);

  ScrollBar() : Range("ScrollBar") {
    mouseFilter = MouseFilter::Stop;
  }

  explicit ScrollBar(bool isVertical, std::string nodeName = "ScrollBar")
      : Range(std::move(nodeName)), vertical(isVertical) {
    mouseFilter = MouseFilter::Stop;
    if (vertical) {
      customMinimumSize = {12.0f, 100.0f};
    } else {
      customMinimumSize = {100.0f, 12.0f};
    }
  }

  void onGuiInput(const InputEvent &event) override {
    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left) {
      if (event.isPressed()) {
        m_isDragging = true;
        updateValueFromMouse(event.mousePosition);
        const_cast<InputEvent &>(event).setHandled();
      } else {
        m_isDragging = false;
      }
    } else if (event.type == InputEventType::MouseMotion && m_isDragging) {
      updateValueFromMouse(event.mousePosition);
      const_cast<InputEvent &>(event).setHandled();
    } else if (event.type == InputEventType::MouseWheel) {
      setValue(value - event.mouseScroll.y * step * 3.0f);
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    float ratio = getRatio();

    // 1. Draw Track
    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, 3.0f, trackColor * modulate);

    // 2. Compute Thumb Geometry
    Color activeGrabColor = m_isDragging ? grabberPressedColor : (isHovered() ? grabberHoverColor : grabberColor);

    if (!vertical) {
      float thumbW = std::max(20.0f, (pageSize > 0.0f && maxValue > minValue) ? (rect.size.x * (pageSize / (maxValue - minValue + pageSize))) : 30.0f);
      float usableW = rect.size.x - thumbW;
      float thumbX = rect.position.x + (usableW * ratio);
      Renderer2D::drawRoundedRectScreen(Vector2(thumbX, rect.position.y + 1.0f),
                                        Vector2(thumbW, rect.size.y - 2.0f), 2.0f,
                                        activeGrabColor * modulate);
    } else {
      float thumbH = std::max(20.0f, (pageSize > 0.0f && maxValue > minValue) ? (rect.size.y * (pageSize / (maxValue - minValue + pageSize))) : 30.0f);
      float usableH = rect.size.y - thumbH;
      float thumbY = rect.position.y + (usableH * ratio);
      Renderer2D::drawRoundedRectScreen(Vector2(rect.position.x + 1.0f, thumbY),
                                        Vector2(rect.size.x - 2.0f, thumbH), 2.0f,
                                        activeGrabColor * modulate);
    }
  }

private:
  void updateValueFromMouse(const Vector2 &mousePos) {
    Rect2 rect = getGlobalRect();
    if (!vertical) {
      float thumbW = std::max(20.0f, (pageSize > 0.0f && maxValue > minValue) ? (rect.size.x * (pageSize / (maxValue - minValue + pageSize))) : 30.0f);
      float usableW = rect.size.x - thumbW;
      if (usableW <= 0.0f) return;
      float relX = mousePos.x - (rect.position.x + thumbW * 0.5f);
      setRatio(relX / usableW);
    } else {
      float thumbH = std::max(20.0f, (pageSize > 0.0f && maxValue > minValue) ? (rect.size.y * (pageSize / (maxValue - minValue + pageSize))) : 30.0f);
      float usableH = rect.size.y - thumbH;
      if (usableH <= 0.0f) return;
      float relY = mousePos.y - (rect.position.y + thumbH * 0.5f);
      setRatio(relY / usableH);
    }
  }

  bool m_isDragging = false;
};

// Horizontal ScrollBar Node (inspired by Godot HScrollBar)
class HScrollBar : public ScrollBar {
public:
  HScrollBar() : ScrollBar(false, "HScrollBar") {}
};

// Vertical ScrollBar Node (inspired by Godot VScrollBar)
class VScrollBar : public ScrollBar {
public:
  VScrollBar() : ScrollBar(true, "VScrollBar") {}
};
