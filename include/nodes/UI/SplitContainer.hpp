#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include "renderers/Renderer2D.hpp"
#include <algorithm>

// Two-Pane Interactive Splitter Container (inspired by Godot SplitContainer)
class SplitContainer : public Container {
public:
  // Signals
  Signal<float> dragged;

  bool vertical = false;
  float splitOffset = 0.0f; // Pixel offset from center
  float splitBarThickness = 8.0f;
  bool collapsed = false;
  bool draggable = true;

  Color splitBarColor = Color::from_rgba8(45, 50, 68);
  Color splitBarHoverColor = Color::from_rgba8(75, 120, 220);
  Color splitBarDragColor = Color::from_rgba8(52, 140, 255);

  SplitContainer() : Container("SplitContainer") {
    mouseFilter = MouseFilter::Stop;
  }

  explicit SplitContainer(bool isVertical, std::string nodeName = "SplitContainer")
      : Container(std::move(nodeName)), vertical(isVertical) {
    mouseFilter = MouseFilter::Stop;
  }

  void onGuiInput(const InputEvent &event) override {
    if (!draggable) return;

    Rect2 barRect = getSplitBarRect();

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left) {
      if (event.isPressed() && barRect.hasPoint(event.mousePosition)) {
        m_isDragging = true;
        m_dragStartPos = event.mousePosition;
        m_dragStartOffset = splitOffset;
        const_cast<InputEvent &>(event).setHandled();
      } else if (!event.isPressed() && m_isDragging) {
        m_isDragging = false;
        const_cast<InputEvent &>(event).setHandled();
      }
    } else if (event.type == InputEventType::MouseMotion && m_isDragging) {
      Rect2 rect = getGlobalRect();
      if (!vertical) {
        float delta = event.mousePosition.x - m_dragStartPos.x;
        float maxOffset = (rect.size.x - splitBarThickness) * 0.5f - 20.0f;
        splitOffset = std::clamp(m_dragStartOffset + delta, -maxOffset, maxOffset);
      } else {
        float delta = event.mousePosition.y - m_dragStartPos.y;
        float maxOffset = (rect.size.y - splitBarThickness) * 0.5f - 20.0f;
        splitOffset = std::clamp(m_dragStartOffset + delta, -maxOffset, maxOffset);
      }
      queueSort();
      dragged.emit(splitOffset);
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();

    std::vector<Ref<Control>> controls;
    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (ctrl && ctrl->visible) controls.push_back(ctrl);
    }
    if (controls.empty()) return;

    if (controls.size() == 1) {
      controls[0]->offsetLeft = 0.0f;
      controls[0]->offsetTop = 0.0f;
      controls[0]->offsetRight = rect.size.x;
      controls[0]->offsetBottom = rect.size.y;
      return;
    }

    if (!vertical) {
      float center = (rect.size.x - splitBarThickness) * 0.5f + splitOffset;
      center = std::clamp(center, 20.0f, rect.size.x - splitBarThickness - 20.0f);

      // Left Child
      controls[0]->offsetLeft = 0.0f;
      controls[0]->offsetTop = 0.0f;
      controls[0]->offsetRight = center;
      controls[0]->offsetBottom = rect.size.y;

      // Right Child
      controls[1]->offsetLeft = center + splitBarThickness;
      controls[1]->offsetTop = 0.0f;
      controls[1]->offsetRight = rect.size.x;
      controls[1]->offsetBottom = rect.size.y;
    } else {
      float center = (rect.size.y - splitBarThickness) * 0.5f + splitOffset;
      center = std::clamp(center, 20.0f, rect.size.y - splitBarThickness - 20.0f);

      // Top Child
      controls[0]->offsetLeft = 0.0f;
      controls[0]->offsetTop = 0.0f;
      controls[0]->offsetRight = rect.size.x;
      controls[0]->offsetBottom = center;

      // Bottom Child
      controls[1]->offsetLeft = 0.0f;
      controls[1]->offsetTop = center + splitBarThickness;
      controls[1]->offsetRight = rect.size.x;
      controls[1]->offsetBottom = rect.size.y;
    }
  }

  void drawControl() override {
    Rect2 barRect = getSplitBarRect();
    Vector2 mousePos = Input::getMousePosition();

    Color activeCol = m_isDragging ? splitBarDragColor : (barRect.hasPoint(mousePos) ? splitBarHoverColor : splitBarColor);
    Renderer2D::drawRoundedRectScreen(barRect.position, barRect.size, 2.0f, activeCol * modulate);

    // Draw grab dots/handle in the center of the bar
    if (!vertical) {
      float cy = barRect.position.y + barRect.size.y * 0.5f;
      float cx = barRect.position.x + barRect.size.x * 0.5f;
      for (int i = -2; i <= 2; ++i) {
        Renderer2D::drawCircle(Vector2(cx, cy + i * 5.0f), 1.5f, Color::from_rgba8(200, 210, 230, 180));
      }
    } else {
      float cy = barRect.position.y + barRect.size.y * 0.5f;
      float cx = barRect.position.x + barRect.size.x * 0.5f;
      for (int i = -2; i <= 2; ++i) {
        Renderer2D::drawCircle(Vector2(cx + i * 5.0f, cy), 1.5f, Color::from_rgba8(200, 210, 230, 180));
      }
    }
  }

  Rect2 getSplitBarRect() const {
    Rect2 rect = getGlobalRect();
    if (!vertical) {
      float center = (rect.size.x - splitBarThickness) * 0.5f + splitOffset;
      center = std::clamp(center, 20.0f, rect.size.x - splitBarThickness - 20.0f);
      return Rect2(rect.position.x + center, rect.position.y, splitBarThickness, rect.size.y);
    } else {
      float center = (rect.size.y - splitBarThickness) * 0.5f + splitOffset;
      center = std::clamp(center, 20.0f, rect.size.y - splitBarThickness - 20.0f);
      return Rect2(rect.position.x, rect.position.y + center, rect.size.x, splitBarThickness);
    }
  }

private:
  bool m_isDragging = false;
  Vector2 m_dragStartPos{0.0f, 0.0f};
  float m_dragStartOffset = 0.0f;
};

// Horizontal Split Container Node (inspired by Godot HSplitContainer)
class HSplitContainer : public SplitContainer {
public:
  HSplitContainer() : SplitContainer(false, "HSplitContainer") {}
};

// Vertical Split Container Node (inspired by Godot VSplitContainer)
class VSplitContainer : public SplitContainer {
public:
  VSplitContainer() : SplitContainer(true, "VSplitContainer") {}
};
