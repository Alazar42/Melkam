#pragma once

#include "nodes/UI/Control.hpp"
#include "renderers/Font.hpp"
#include <algorithm>
#include <string>

// Base Button Node (inspired by Godot BaseButton) with push/toggle states and signals.
class BaseButton : public Control {
public:
  // Signals
  Signal<> pressed;
  Signal<> button_down;
  Signal<> button_up;
  Signal<bool> toggled;

  bool disabled = false;
  bool toggleMode = false;
  bool buttonPressed = false;

  BaseButton() : Control("BaseButton") {
    mouseFilter = MouseFilter::Stop;
  }
  explicit BaseButton(std::string name) : Control(std::move(name)) {
    mouseFilter = MouseFilter::Stop;
  }

  void setPressed(bool isPressed) {
    if (buttonPressed != isPressed) {
      buttonPressed = isPressed;
      toggled.emit(buttonPressed);
    }
  }

  bool isPressed() const { return buttonPressed; }

  void onGuiInput(const InputEvent &event) override {
    if (disabled) return;

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left) {
      if (event.isPressed()) {
        m_isDown = true;
        button_down.emit();
        const_cast<InputEvent &>(event).setHandled();
      } else {
        if (m_isDown) {
          m_isDown = false;
          button_up.emit();
          if (m_isHovered) {
            if (toggleMode) {
              setPressed(!buttonPressed);
            }
            pressed.emit();
          }
          const_cast<InputEvent &>(event).setHandled();
        }
      }
    }
  }

protected:
  bool m_isDown = false;
};

// CheckBox UI Node (inspired by Godot CheckBox) with checkmark indicator and text label.
class CheckBox : public BaseButton {
public:
  std::string text;
  std::shared_ptr<Font> font = nullptr;
  float fontSize = 16.0f;

  // Colors & Theme Styling
  Color boxColor = Color::from_rgba8(35, 38, 48);
  Color boxCheckedColor = Color::from_rgba8(52, 120, 246);
  Color checkmarkColor = Color::WHITE;
  Color borderColor = Color::from_rgba8(80, 85, 105);
  Color fontColor = Color::WHITE;
  float boxSize = 20.0f;

  CheckBox() : BaseButton("CheckBox") {
    toggleMode = true;
    customMinimumSize = {140.0f, 28.0f};
  }

  explicit CheckBox(std::string labelText, bool defaultChecked = false)
      : BaseButton("CheckBox"), text(std::move(labelText)) {
    toggleMode = true;
    buttonPressed = defaultChecked;
    customMinimumSize = {140.0f, 28.0f};
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    // 1. Draw Check Box
    float boxY = rect.position.y + (rect.size.y - boxSize) * 0.5f;
    Vector2 boxPos{rect.position.x, boxY};
    Color activeBoxColor = buttonPressed ? boxCheckedColor : boxColor;
    Color activeBorder = isHovered() ? Color::WHITE : borderColor;

    Renderer2D::drawRoundedRectScreen(boxPos, Vector2(boxSize, boxSize), 4.0f,
                                      activeBoxColor * modulate, activeBorder * modulate, 1.5f);

    // 2. Draw Checkmark if checked
    if (buttonPressed) {
      float pad = 4.0f;
      Vector2 markPos = boxPos + Vector2(pad, pad);
      Vector2 markSize = Vector2(boxSize - pad * 2.0f, boxSize - pad * 2.0f);
      Renderer2D::drawRoundedRectScreen(markPos, markSize, 2.0f, checkmarkColor * modulate);
    }

    // 3. Draw Label Text
    if (!text.empty()) {
      float textX = rect.position.x + boxSize + 10.0f;
      float textY = rect.position.y + (rect.size.y - fontSize) * 0.5f;
      Renderer2D::drawText(text, Vector2(textX, textY), fontColor * modulate, fontSize, font);
    }
  }
};

// CheckButton UI Node (inspired by Godot CheckButton) with modern toggle switch.
class CheckButton : public BaseButton {
public:
  std::string text;
  std::shared_ptr<Font> font = nullptr;
  float fontSize = 16.0f;

  // Colors & Theme Styling
  Color trackOffColor = Color::from_rgba8(45, 48, 60);
  Color trackOnColor = Color::from_rgba8(52, 199, 89);
  Color thumbColor = Color::WHITE;
  Color fontColor = Color::WHITE;
  float switchWidth = 44.0f;
  float switchHeight = 24.0f;

  CheckButton() : BaseButton("CheckButton") {
    toggleMode = true;
    customMinimumSize = {160.0f, 30.0f};
  }

  explicit CheckButton(std::string labelText, bool defaultChecked = false)
      : BaseButton("CheckButton"), text(std::move(labelText)) {
    toggleMode = true;
    buttonPressed = defaultChecked;
    customMinimumSize = {160.0f, 30.0f};
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    // 1. Draw Text on Left
    if (!text.empty()) {
      float textY = rect.position.y + (rect.size.y - fontSize) * 0.5f;
      Renderer2D::drawText(text, Vector2(rect.position.x, textY), fontColor * modulate, fontSize, font);
    }

    // 2. Draw Toggle Switch on Right
    float switchX = rect.position.x + rect.size.x - switchWidth;
    float switchY = rect.position.y + (rect.size.y - switchHeight) * 0.5f;
    Vector2 switchPos{switchX, switchY};
    Vector2 switchSize{switchWidth, switchHeight};

    Color activeTrackColor = buttonPressed ? trackOnColor : trackOffColor;
    Renderer2D::drawRoundedRectScreen(switchPos, switchSize, switchHeight * 0.5f,
                                      activeTrackColor * modulate);

    // 3. Draw Sliding Thumb Knob
    float thumbPad = 2.0f;
    float thumbRadius = (switchHeight - thumbPad * 2.0f) * 0.5f;
    float thumbX = buttonPressed ? (switchX + switchWidth - switchHeight + thumbPad)
                                 : (switchX + thumbPad);
    float thumbY = switchY + thumbPad;
    float thumbD = thumbRadius * 2.0f;

    Renderer2D::drawRoundedRectScreen(Vector2(thumbX, thumbY), Vector2(thumbD, thumbD),
                                      thumbRadius, thumbColor * modulate);
  }
};
