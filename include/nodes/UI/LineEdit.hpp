#pragma once

#include "nodes/UI/Control.hpp"
#include "renderers/Font.hpp"
#include "time.hpp"
#include <algorithm>
#include <string>

// Single-Line Interactive Text Input UI Node (inspired by Godot LineEdit).
class LineEdit : public Control {
public:
  // Signals
  Signal<std::string> text_changed;
  Signal<std::string> text_submitted;

  std::string text;
  std::string placeholderText;
  int maxLength = 0;
  bool editable = true;
  bool secret = false;
  std::shared_ptr<Font> font = nullptr;
  float fontSize = 16.0f;

  // Colors & Theme Styling
  Color backgroundColor = Color::from_rgba8(30, 32, 42);
  Color borderColor = Color::from_rgba8(75, 80, 105);
  Color focusBorderColor = Color::from_rgba8(52, 120, 246);
  Color fontColor = Color::WHITE;
  Color placeholderColor = Color::from_rgba8(120, 125, 145);
  Color cursorColor = Color::WHITE;
  float borderWidth = 1.0f;
  float cornerRadius = 4.0f;

  LineEdit() : Control("LineEdit") {
    customMinimumSize = {180.0f, 34.0f};
    mouseFilter = MouseFilter::Stop;
  }

  explicit LineEdit(std::string placeholder)
      : Control("LineEdit"), placeholderText(std::move(placeholder)) {
    customMinimumSize = {180.0f, 34.0f};
    mouseFilter = MouseFilter::Stop;
  }

  void setText(std::string newText) {
    if (text != newText) {
      text = std::move(newText);
      m_cursorPos = std::clamp(m_cursorPos, 0, static_cast<int>(text.length()));
      text_changed.emit(text);
    }
  }

  const std::string &getText() const { return text; }

  void clear() {
    setText("");
  }

  void onProcess(float delta) override {
    if (m_isFocused) {
      m_blinkTimer += delta;
      if (m_blinkTimer >= 0.5f) {
        m_blinkTimer = 0.0f;
        m_showCursor = !m_showCursor;
      }
    }
  }

  void onGuiInput(const InputEvent &event) override {
    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left) {
      if (event.isPressed()) {
        m_isFocused = true;
        m_blinkTimer = 0.0f;
        m_showCursor = true;
        m_cursorPos = static_cast<int>(text.length());
        const_cast<InputEvent &>(event).setHandled();
      }
    }
  }

  void onInput(const InputEvent &event) override {
    Control::onInput(event);

    if (!m_isFocused || !editable) return;

    if (event.type == InputEventType::MouseButton && event.isPressed()) {
      if (!getGlobalRect().hasPoint(event.mousePosition)) {
        m_isFocused = false;
      }
    } else if (event.type == InputEventType::Key && event.isPressed()) {
      handleKeyPress(event);
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    // 1. Draw Background & Focus Border
    Color activeBorder = m_isFocused ? focusBorderColor : borderColor;
    float activeBorderWidth = m_isFocused ? 1.5f : borderWidth;

    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, cornerRadius,
                                      backgroundColor * modulate, activeBorder * modulate,
                                      activeBorderWidth);

    // 2. Compute Display Text
    std::string displayText = text;
    if (secret) {
      displayText = std::string(text.length(), '*');
    }

    const Font &activeFont = font ? *font : *Font::getDefaultFont();
    float textX = rect.position.x + 10.0f;
    float textY = rect.position.y + (rect.size.y - fontSize) * 0.5f;

    if (displayText.empty() && !placeholderText.empty() && !m_isFocused) {
      // Draw Placeholder Text
      Renderer2D::drawText(placeholderText, Vector2(textX, textY),
                           placeholderColor * modulate, fontSize, font);
    } else if (!displayText.empty()) {
      // Draw Input Text
      Renderer2D::drawText(displayText, Vector2(textX, textY),
                           fontColor * modulate, fontSize, font);
    }

    // 3. Draw Blinking Cursor when Focused
    if (m_isFocused && m_showCursor) {
      std::string textBeforeCursor = displayText.substr(0, m_cursorPos);
      Vector2 textSize = activeFont.getStringSize(textBeforeCursor, fontSize);

      float cursorX = textX + textSize.x;
      float cursorH = fontSize * 1.1f;
      float cursorY = rect.position.y + (rect.size.y - cursorH) * 0.5f;

      Renderer2D::drawRectScreen(Vector2(cursorX, cursorY), Vector2(2.0f, cursorH),
                                cursorColor * modulate, true);
    }
  }

private:
  void handleKeyPress(const InputEvent &ev) {
    if (ev.key == Key::Backspace) {
      if (m_cursorPos > 0 && !text.empty()) {
        text.erase(m_cursorPos - 1, 1);
        m_cursorPos--;
        text_changed.emit(text);
      }
    } else if (ev.key == Key::Delete) {
      if (m_cursorPos < static_cast<int>(text.length())) {
        text.erase(m_cursorPos, 1);
        text_changed.emit(text);
      }
    } else if (ev.key == Key::Left) {
      if (m_cursorPos > 0) m_cursorPos--;
    } else if (ev.key == Key::Right) {
      if (m_cursorPos < static_cast<int>(text.length())) m_cursorPos++;
    } else if (ev.key == Key::Home) {
      m_cursorPos = 0;
    } else if (ev.key == Key::End) {
      m_cursorPos = static_cast<int>(text.length());
    } else if (ev.key == Key::Enter) {
      text_submitted.emit(text);
      m_isFocused = false;
    } else {
      // Character typing
      char c = keyToChar(ev.key, ev.shift);
      if (c != 0) {
        if (maxLength == 0 || static_cast<int>(text.length()) < maxLength) {
          text.insert(m_cursorPos, 1, c);
          m_cursorPos++;
          text_changed.emit(text);
        }
      }
    }
    m_blinkTimer = 0.0f;
    m_showCursor = true;
  }

  static char keyToChar(Key key, bool shift) {
    int k = static_cast<int>(key);
    // Letters A..Z (scancodes 4..29)
    if (k >= static_cast<int>(Key::A) && k <= static_cast<int>(Key::Z)) {
      char base = 'a' + (k - static_cast<int>(Key::A));
      return shift ? static_cast<char>(std::toupper(base)) : base;
    }
    // Numbers 1..9, 0 (scancodes 30..39)
    if (k >= static_cast<int>(Key::Num1) && k <= static_cast<int>(Key::Num9)) {
      if (!shift) return '1' + (k - static_cast<int>(Key::Num1));
      const char shiftNums[] = "!@#$%^&*(";
      return shiftNums[k - static_cast<int>(Key::Num1)];
    }
    if (k == static_cast<int>(Key::Num0)) return shift ? ')' : '0';
    if (k == static_cast<int>(Key::Space)) return ' ';
    if (k == static_cast<int>(Key::Minus)) return shift ? '_' : '-';
    if (k == static_cast<int>(Key::Equals)) return shift ? '+' : '=';
    if (k == static_cast<int>(Key::Period)) return shift ? '>' : '.';
    if (k == static_cast<int>(Key::Comma)) return shift ? '<' : ',';
    if (k == static_cast<int>(Key::Slash)) return shift ? '?' : '/';
    return 0;
  }

  bool m_isFocused = false;
  int m_cursorPos = 0;
  float m_blinkTimer = 0.0f;
  bool m_showCursor = true;
};
