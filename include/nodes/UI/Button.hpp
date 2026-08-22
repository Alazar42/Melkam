#pragma once

#include "nodes/UI/Control.hpp"
#include "renderers/Font.hpp"
#include <algorithm>
#include <memory>
#include <string>

// Interactive Push Button Node (inspired by Godot Button) with hover, pressed styles and signals.
class Button : public Control {
public:
  // Signals
  Signal<> pressed;
  Signal<> button_down;
  Signal<> button_up;

  // Text & Visuals
  std::string text;
  std::shared_ptr<Font> font = nullptr;
  float fontSize = 18.0f;
  bool disabled = false;
  bool flat = false;

  // Colors & Theme Styling
  Color normalColor = Color::from_rgba8(50, 52, 64);
  Color hoverColor = Color::from_rgba8(70, 75, 95);
  Color pressedColor = Color::from_rgba8(32, 34, 45);
  Color disabledColor = Color::from_rgba8(30, 30, 35);
  Color fontColor = Color::WHITE;
  Color borderColor = Color::from_rgba8(95, 100, 125);
  float borderWidth = 1.0f;
  float cornerRadius = 4.0f;

  Button() : Control("Button") {
    customMinimumSize = {120.0f, 36.0f};
  }

  explicit Button(std::string buttonText)
      : Control("Button"), text(std::move(buttonText)) {
    customMinimumSize = {120.0f, 36.0f};
  }

  void onGuiInput(const InputEvent &event) override {
    if (disabled) return;

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left) {
      if (event.isPressed()) {
        m_isPressed = true;
        button_down.emit();
        const_cast<InputEvent &>(event).setHandled();
      } else {
        if (m_isPressed) {
          m_isPressed = false;
          button_up.emit();
          if (m_isHovered) {
            pressed.emit();
          }
          const_cast<InputEvent &>(event).setHandled();
        }
      }
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    // Determine active background & border color
    Color activeBg = normalColor;
    Color activeBorder = borderColor;

    if (disabled) {
      activeBg = disabledColor;
    } else if (m_isPressed && m_isHovered) {
      activeBg = pressedColor;
    } else if (m_isHovered) {
      activeBg = hoverColor;
    }

    // Render button background
    if (!flat) {
      Renderer2D::drawRoundedRectScreen(rect.position, rect.size, cornerRadius,
                                        activeBg * modulate, activeBorder * modulate,
                                        borderWidth);
    }

    // Render centered label text
    if (!text.empty()) {
      const Font &activeFont = font ? *font : *Font::getDefaultFont();
      Vector2 textSize = activeFont.getStringSize(text, fontSize);

      float textX = rect.position.x + (rect.size.x - textSize.x) * 0.5f;
      float textY = rect.position.y + (rect.size.y - textSize.y) * 0.5f;

      if (m_isPressed && m_isHovered) {
        textY += 1.0f; // Slight pressed text depression effect
      }

      Color activeFontColor = disabled ? Color::from_rgba8(120, 120, 130) : fontColor;
      Renderer2D::drawText(text, Vector2(textX, textY), activeFontColor * modulate,
                           fontSize, font);
    }
  }

private:
  bool m_isPressed = false;
};
