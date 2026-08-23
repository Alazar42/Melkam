#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/LineEdit.hpp"
#include "nodes/UI/Range.hpp"
#include "renderers/Font.hpp"
#include "renderers/Renderer2D.hpp"
#include <algorithm>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

// Numeric Stepper Input UI Node (inspired by Godot SpinBox)
class SpinBox : public Range {
public:
  std::string prefix;
  std::string suffix;
  bool editable = true;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f;

  Color backgroundColor = Color(0, 0, 0, 0);
  Color borderColor = Color(0, 0, 0, 0);
  Color fontColor = Color(0, 0, 0, 0);
  Color buttonColor = Color::from_rgba8(45, 52, 70);
  Color buttonHoverColor = Color::from_rgba8(65, 75, 100);

  SpinBox() : Range("SpinBox") {
    customMinimumSize = {120.0f, 34.0f};
    mouseFilter = MouseFilter::Stop;
    focusMode = FocusMode::All;
    minValue = 0.0f;
    maxValue = 100.0f;
    step = 1.0f;
    value = 0.0f;
  }

  void onGuiInput(const InputEvent &event) override {
    if (!editable) return;

    Rect2 rect = getGlobalRect();
    float btnW = 22.0f;
    float btnH = rect.size.y * 0.5f;

    Rect2 upRect(rect.position.x + rect.size.x - btnW, rect.position.y, btnW, btnH);
    Rect2 downRect(rect.position.x + rect.size.x - btnW, rect.position.y + btnH, btnW, btnH);

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left && event.isPressed()) {
      grabFocus();
      if (upRect.hasPoint(event.mousePosition)) {
        setValue(value + step);
        const_cast<InputEvent &>(event).setHandled();
      } else if (downRect.hasPoint(event.mousePosition)) {
        setValue(value - step);
        const_cast<InputEvent &>(event).setHandled();
      }
    } else if (event.type == InputEventType::MouseWheel) {
      setValue(value + event.mouseScroll.y * step);
      const_cast<InputEvent &>(event).setHandled();
    } else if (event.type == InputEventType::Key && event.isPressed() && hasFocus()) {
      if (event.key == Key::Up) {
        setValue(value + step);
        const_cast<InputEvent &>(event).setHandled();
      } else if (event.key == Key::Down) {
        setValue(value - step);
        const_cast<InputEvent &>(event).setHandled();
      }
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    Color bg = (backgroundColor.a > 0.0f) ? backgroundColor : getThemeColor("bg_color", "LineEdit", Color::from_rgba8(30, 32, 42));
    Color border = (borderColor.a > 0.0f) ? borderColor : getThemeColor("border_color", "LineEdit", Color::from_rgba8(75, 80, 105));
    Color focusBorder = getThemeColor("focus_border_color", "LineEdit", Color::from_rgba8(52, 120, 246));
    Color txtCol = (fontColor.a > 0.0f) ? fontColor : getThemeColor("font_color", "LineEdit", Color::WHITE);
    Ref<Font> activeFont = font ? font : getThemeFont("font", "SpinBox");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "SpinBox", 16));

    // 1. Draw Input Box Background & Border
    Color activeBorder = hasFocus() ? focusBorder : border;
    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, 4.0f, bg * modulate, activeBorder * modulate, hasFocus() ? 1.5f : 1.0f);

    // 2. Draw Numeric Text with Prefix / Suffix
    std::ostringstream oss;
    oss << prefix;
    if (rounded || step == std::floor(step)) {
      oss << static_cast<int>(value + 0.5f);
    } else {
      oss << std::fixed << std::setprecision(2) << value;
    }
    oss << suffix;
    std::string valStr = oss.str();

    float textX = rect.position.x + 8.0f;
    float textY = rect.position.y + (rect.size.y - activeSize) * 0.5f;
    Renderer2D::drawText(valStr, Vector2(textX, textY), txtCol * modulate, activeSize, activeFont);

    // 3. Draw Up/Down Stepper Buttons
    float btnW = 20.0f;
    float btnH = (rect.size.y - 4.0f) * 0.5f;
    Vector2 mousePos = Input::getMousePosition();

    Rect2 upRect(rect.position.x + rect.size.x - btnW - 2.0f, rect.position.y + 2.0f, btnW, btnH);
    Rect2 downRect(rect.position.x + rect.size.x - btnW - 2.0f, rect.position.y + 2.0f + btnH, btnW, btnH);

    Color upCol = upRect.hasPoint(mousePos) ? buttonHoverColor : buttonColor;
    Color downCol = downRect.hasPoint(mousePos) ? buttonHoverColor : buttonColor;

    Renderer2D::drawRoundedRectScreen(upRect.position, upRect.size, 2.0f, upCol * modulate);
    Renderer2D::drawText("^", Vector2(upRect.position.x + 6.0f, upRect.position.y - 1.0f), Color::WHITE, 12.0f, activeFont);

    Renderer2D::drawRoundedRectScreen(downRect.position, downRect.size, 2.0f, downCol * modulate);
    Renderer2D::drawText("v", Vector2(downRect.position.x + 6.0f, downRect.position.y - 2.0f), Color::WHITE, 10.0f, activeFont);
  }
};
