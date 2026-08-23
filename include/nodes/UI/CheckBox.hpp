#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "nodes/UI/Icons.hpp"
#include "renderers/Font.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

class BaseButton;

// Radio button group manager for mutually exclusive toggle buttons
class ButtonGroup {
public:
  std::vector<BaseButton *> buttons;

  void addButton(BaseButton *btn);
  void removeButton(BaseButton *btn);
  void onButtonPressed(BaseButton *pressedBtn);
  BaseButton *getPressedButton() const;
};

// Base Button Node (inspired by Godot BaseButton) with push/toggle states, button groups, and signals.
class BaseButton : public Control {
public:
  enum class ActionMode {
    Press,
    Release
  };

  // Signals
  Signal<> pressed;
  Signal<> button_down;
  Signal<> button_up;
  Signal<bool> toggled;

  bool disabled = false;
  bool toggleMode = false;
  bool buttonPressed = false;
  ActionMode actionMode = ActionMode::Release;
  Ref<ButtonGroup> buttonGroup = nullptr;

  BaseButton() : Control("BaseButton") {
    mouseFilter = MouseFilter::Stop;
    focusMode = FocusMode::All;
  }
  explicit BaseButton(std::string name) : Control(std::move(name)) {
    mouseFilter = MouseFilter::Stop;
    focusMode = FocusMode::All;
  }

  ~BaseButton() override {
    setButtonGroup(nullptr);
  }

  void setButtonGroup(Ref<ButtonGroup> group) {
    if (buttonGroup) {
      buttonGroup->removeButton(this);
    }
    buttonGroup = std::move(group);
    if (buttonGroup) {
      buttonGroup->addButton(this);
    }
  }

  void setPressed(bool isPressed) {
    if (buttonPressed != isPressed) {
      buttonPressed = isPressed;
      if (buttonPressed && buttonGroup) {
        buttonGroup->onButtonPressed(this);
      }
      toggled.emit(buttonPressed);
    }
  }

  void setButtonPressed(bool isPressed) {
    setPressed(isPressed);
  }

  bool isPressed() const { return buttonPressed; }
  bool isButtonPressed() const { return buttonPressed; }


  void onGuiInput(const InputEvent &event) override {
    if (disabled) return;

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left) {
      if (event.isPressed()) {
        m_isDown = true;
        grabFocus();
        button_down.emit();
        if (actionMode == ActionMode::Press) {
          if (toggleMode) {
            setPressed(!buttonPressed);
          }
          pressed.emit();
        }
        const_cast<InputEvent &>(event).setHandled();
      } else {
        if (m_isDown) {
          m_isDown = false;
          button_up.emit();
          if (m_isHovered && actionMode == ActionMode::Release) {
            if (toggleMode) {
              setPressed(!buttonPressed);
            }
            pressed.emit();
          }
          const_cast<InputEvent &>(event).setHandled();
        }
      }
    } else if (event.type == InputEventType::Key && hasFocus()) {
      if (event.isPressed() && (event.key == Key::Space || event.key == Key::Enter)) {
        m_isDown = true;
        button_down.emit();
        if (toggleMode) {
          setPressed(!buttonPressed);
        }
        pressed.emit();
        const_cast<InputEvent &>(event).setHandled();
      } else if (!event.isPressed() && (event.key == Key::Space || event.key == Key::Enter)) {
        if (m_isDown) {
          m_isDown = false;
          button_up.emit();
          const_cast<InputEvent &>(event).setHandled();
        }
      }
    }
  }

protected:
  bool m_isDown = false;
};

inline void ButtonGroup::addButton(BaseButton *btn) {
  if (btn && std::find(buttons.begin(), buttons.end(), btn) == buttons.end()) {
    buttons.push_back(btn);
  }
}

inline void ButtonGroup::removeButton(BaseButton *btn) {
  auto it = std::find(buttons.begin(), buttons.end(), btn);
  if (it != buttons.end()) {
    buttons.erase(it);
  }
}

inline void ButtonGroup::onButtonPressed(BaseButton *pressedBtn) {
  for (auto *btn : buttons) {
    if (btn && btn != pressedBtn && btn->isPressed()) {
      btn->buttonPressed = false;
      btn->toggled.emit(false);
    }
  }
}

inline BaseButton *ButtonGroup::getPressedButton() const {
  for (auto *btn : buttons) {
    if (btn && btn->isPressed()) return btn;
  }
  return nullptr;
}

// CheckBox UI Node (inspired by Godot CheckBox) with checkmark indicator and text label.
class CheckBox : public BaseButton {
public:
  std::string text;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f; // 0 = inherits from theme

  // Colors & Theme Styling
  Color boxColor = Color(0, 0, 0, 0);
  Color boxCheckedColor = Color(0, 0, 0, 0);
  Color checkmarkColor = Color(0, 0, 0, 0);
  Color borderColor = Color(0, 0, 0, 0);
  Color fontColor = Color(0, 0, 0, 0);
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

    // 1. Resolve Theme Properties
    Color bColor = (boxColor.a > 0.0f) ? boxColor : getThemeColor("box_color", "CheckBox", Color::from_rgba8(35, 38, 48));
    Color bChecked = (boxCheckedColor.a > 0.0f) ? boxCheckedColor : getThemeColor("box_checked_color", "CheckBox", Color::from_rgba8(52, 120, 246));
    Color checkCol = (checkmarkColor.a > 0.0f) ? checkmarkColor : getThemeColor("checkmark_color", "CheckBox", Color::WHITE);
    Color border = (borderColor.a > 0.0f) ? borderColor : getThemeColor("border_color", "CheckBox", Color::from_rgba8(80, 85, 105));
    Color txtCol = (fontColor.a > 0.0f) ? fontColor : getThemeColor("font_color", "CheckBox", Color::WHITE);
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "CheckBox", 16));

    // 2. Draw Check Box Box
    float boxY = rect.position.y + (rect.size.y - boxSize) * 0.5f;
    Vector2 boxPos{rect.position.x, boxY};
    Color activeBoxColor = buttonPressed ? bChecked : bColor;
    Color activeBorder = (isHovered() || hasFocus()) ? Color::WHITE : border;

    Renderer2D::drawRoundedRectScreen(boxPos, Vector2(boxSize, boxSize), 4.0f,
                                      activeBoxColor * modulate, activeBorder * modulate, 1.5f);

    // 3. Draw Checkmark if checked
    if (buttonPressed) {
      Vector2 markCenter = boxPos + Vector2(boxSize * 0.5f, boxSize * 0.5f);
      Icons::draw(IconType::Check, markCenter, boxSize * 0.75f, checkCol * modulate, 2.0f);
    }


    // 4. Draw Label Text
    if (!text.empty()) {
      float textX = rect.position.x + boxSize + 10.0f;
      float textY = rect.position.y + (rect.size.y - activeSize) * 0.5f;
      Renderer2D::drawText(text, Vector2(textX, textY), txtCol * modulate, activeSize, font);
    }
  }
};

// CheckButton UI Node (inspired by Godot CheckButton) with modern toggle switch.
class CheckButton : public BaseButton {
public:
  std::string text;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f;

  // Colors & Theme Styling
  Color trackOffColor = Color(0, 0, 0, 0);
  Color trackOnColor = Color(0, 0, 0, 0);
  Color thumbColor = Color(0, 0, 0, 0);
  Color fontColor = Color(0, 0, 0, 0);
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

    Color offCol = (trackOffColor.a > 0.0f) ? trackOffColor : getThemeColor("track_off_color", "CheckButton", Color::from_rgba8(45, 48, 60));
    Color onCol = (trackOnColor.a > 0.0f) ? trackOnColor : getThemeColor("track_on_color", "CheckButton", Color::from_rgba8(52, 199, 89));
    Color thCol = (thumbColor.a > 0.0f) ? thumbColor : getThemeColor("thumb_color", "CheckButton", Color::WHITE);
    Color txtCol = (fontColor.a > 0.0f) ? fontColor : getThemeColor("font_color", "CheckButton", Color::WHITE);
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "CheckButton", 16));

    // 1. Draw Text on Left
    if (!text.empty()) {
      float textY = rect.position.y + (rect.size.y - activeSize) * 0.5f;
      Renderer2D::drawText(text, Vector2(rect.position.x, textY), txtCol * modulate, activeSize, font);
    }

    // 2. Draw Toggle Switch on Right
    float switchX = rect.position.x + rect.size.x - switchWidth;
    float switchY = rect.position.y + (rect.size.y - switchHeight) * 0.5f;
    Vector2 switchPos{switchX, switchY};
    Vector2 switchSize{switchWidth, switchHeight};

    Color activeTrackColor = buttonPressed ? onCol : offCol;
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
                                      thumbRadius, thCol * modulate);
  }
};

