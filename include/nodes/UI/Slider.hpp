#pragma once

#include "nodes/UI/Range.hpp"
#include <algorithm>

// Base Slider UI Node (inspired by Godot Slider) for draggable numeric values.
class Slider : public Range {
public:
  // Signals
  Signal<> drag_started;
  Signal<bool> drag_ended;

  bool editable = true;
  bool scrollable = true;
  bool vertical = false;

  // Colors & Theme Styling
  Color trackColor = Color::from_rgba8(45, 48, 60);
  Color trackFillColor = Color::from_rgba8(66, 135, 245);
  Color grabberColor = Color::from_rgba8(230, 235, 245);
  Color grabberHoverColor = Color::WHITE;
  Color grabberBorderColor = Color::from_rgba8(30, 32, 40);
  float trackThickness = 6.0f;
  float grabberSize = 14.0f;

  Slider() : Range("Slider") {
    mouseFilter = MouseFilter::Stop;
  }

  explicit Slider(bool isVertical, std::string nodeName = "Slider")
      : Range(std::move(nodeName)), vertical(isVertical) {
    mouseFilter = MouseFilter::Stop;
    if (vertical) {
      customMinimumSize = {24.0f, 120.0f};
    } else {
      customMinimumSize = {120.0f, 24.0f};
    }
  }

  void onGuiInput(const InputEvent &event) override {
    if (!editable) return;

    if (event.type == InputEventType::MouseButton) {
      if (event.isPressed() && event.mouseButton == MouseButton::Left) {
        m_isDragging = true;
        drag_started.emit();
        updateValueFromMouse(event.mousePosition);
        const_cast<InputEvent &>(event).setHandled();
      } else if (!event.isPressed() && event.mouseButton == MouseButton::Left) {
        if (m_isDragging) {
          m_isDragging = false;
          drag_ended.emit(true);
          const_cast<InputEvent &>(event).setHandled();
        }
      }
    } else if (event.type == InputEventType::MouseMotion) {
      if (m_isDragging) {
        updateValueFromMouse(event.mousePosition);
        const_cast<InputEvent &>(event).setHandled();
      }
    } else if (event.type == InputEventType::MouseWheel && scrollable) {
      float delta = event.mouseScroll.y * step;
      if (delta == 0.0f) delta = (event.mouseScroll.y > 0.0f ? 1.0f : -1.0f) * step;
      setValue(value + delta);
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    float ratio = getRatio();

    Color trk = (trackColor.a > 0.0f)
                    ? trackColor
                    : getThemeColor("track_color", "Slider", Color::from_rgba8(45, 48, 60));
    Color trkFill = (trackFillColor.a > 0.0f)
                        ? trackFillColor
                        : getThemeColor("track_fill_color", "Slider", Color::from_rgba8(66, 135, 245));
    Color grab = (grabberColor.a > 0.0f)
                     ? grabberColor
                     : getThemeColor("grabber_color", "Slider", Color::from_rgba8(230, 235, 245));
    Color grabHov = (grabberHoverColor.a > 0.0f)
                        ? grabberHoverColor
                        : getThemeColor("grabber_hover_color", "Slider", Color::WHITE);
    Color grabBorder = (grabberBorderColor.a > 0.0f)
                           ? grabberBorderColor
                           : getThemeColor("grabber_border_color", "Slider", Color::from_rgba8(30, 32, 40));

    if (!vertical) {
      // Horizontal Slider Track
      float trackY = rect.position.y + (rect.size.y - trackThickness) * 0.5f;
      Vector2 trackPos{rect.position.x + grabberSize * 0.5f, trackY};
      Vector2 trackSize{rect.size.x - grabberSize, trackThickness};

      // 1. Background Track
      Renderer2D::drawRoundedRectScreen(trackPos, trackSize, trackThickness * 0.5f,
                                        trk * modulate);

      // 2. Active Fill Track
      if (ratio > 0.0f) {
        Vector2 fillSize{trackSize.x * ratio, trackThickness};
        Renderer2D::drawRoundedRectScreen(trackPos, fillSize, trackThickness * 0.5f,
                                          trkFill * modulate);
      }

      // 3. Grabber Handle
      float grabberX = trackPos.x + (trackSize.x * ratio) - grabberSize * 0.5f;
      float grabberY = rect.position.y + (rect.size.y - grabberSize) * 0.5f;
      Color activeGrabberColor = (m_isDragging || isHovered()) ? grabHov : grab;

      Renderer2D::drawRoundedRectScreen(Vector2(grabberX, grabberY),
                                        Vector2(grabberSize, grabberSize),
                                        grabberSize * 0.5f,
                                        activeGrabberColor * modulate,
                                        grabBorder * modulate, 1.5f);
    } else {
      // Vertical Slider Track
      float trackX = rect.position.x + (rect.size.x - trackThickness) * 0.5f;
      Vector2 trackPos{trackX, rect.position.y + grabberSize * 0.5f};
      Vector2 trackSize{trackThickness, rect.size.y - grabberSize};

      // 1. Background Track
      Renderer2D::drawRoundedRectScreen(trackPos, trackSize, trackThickness * 0.5f,
                                        trk * modulate);

      // 2. Active Fill Track (bottom-to-top)
      if (ratio > 0.0f) {
        float fillH = trackSize.y * ratio;
        Vector2 fillPos{trackPos.x, trackPos.y + trackSize.y - fillH};
        Renderer2D::drawRoundedRectScreen(fillPos, Vector2(trackThickness, fillH),
                                          trackThickness * 0.5f, trkFill * modulate);
      }

      // 3. Grabber Handle
      float grabberX = rect.position.x + (rect.size.x - grabberSize) * 0.5f;
      float grabberY = trackPos.y + trackSize.y - (trackSize.y * ratio) - grabberSize * 0.5f;
      Color activeGrabberColor = (m_isDragging || isHovered()) ? grabHov : grab;

      Renderer2D::drawRoundedRectScreen(Vector2(grabberX, grabberY),
                                        Vector2(grabberSize, grabberSize),
                                        grabberSize * 0.5f,
                                        activeGrabberColor * modulate,
                                        grabBorder * modulate, 1.5f);
    }
  }

protected:
  void updateValueFromMouse(const Vector2 &mousePos) {
    Rect2 rect = getGlobalRect();
    if (!vertical) {
      float usableW = rect.size.x - grabberSize;
      if (usableW <= 0.0f) return;
      float relX = mousePos.x - (rect.position.x + grabberSize * 0.5f);
      setRatio(relX / usableW);
    } else {
      float usableH = rect.size.y - grabberSize;
      if (usableH <= 0.0f) return;
      float relY = (rect.position.y + rect.size.y - grabberSize * 0.5f) - mousePos.y;
      setRatio(relY / usableH);
    }
  }

  bool m_isDragging = false;
};

// Horizontal Slider UI Node (inspired by Godot HSlider).
class HSlider : public Slider {
public:
  HSlider() : Slider(false, "HSlider") {}
};

// Vertical Slider UI Node (inspired by Godot VSlider).
class VSlider : public Slider {
public:
  VSlider() : Slider(true, "VSlider") {}
};
