#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "renderers/Font.hpp"
#include "time.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
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
  bool clearButtonEnabled = false;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f;

  // Colors & Theme Styling
  Color backgroundColor = Color(0, 0, 0, 0);
  Color borderColor = Color(0, 0, 0, 0);
  Color focusBorderColor = Color(0, 0, 0, 0);
  Color fontColor = Color(0, 0, 0, 0);
  Color placeholderColor = Color(0, 0, 0, 0);
  Color cursorColor = Color(0, 0, 0, 0);
  Color selectionColor = Color::from_rgba8(52, 120, 246, 120);
  float borderWidth = -1.0f;
  float cornerRadius = -1.0f;

  LineEdit() : Control("LineEdit") {
    customMinimumSize = {180.0f, 34.0f};
    mouseFilter = MouseFilter::Stop;
    focusMode = FocusMode::All;
  }

  explicit LineEdit(std::string placeholder)
      : Control("LineEdit"), placeholderText(std::move(placeholder)) {
    customMinimumSize = {180.0f, 34.0f};
    mouseFilter = MouseFilter::Stop;
    focusMode = FocusMode::All;
  }

  void setText(std::string newText) {
    if (text != newText) {
      text = std::move(newText);
      m_cursorPos = std::clamp(m_cursorPos, 0, static_cast<int>(text.length()));
      deselect();
      text_changed.emit(text);
    }
  }

  const std::string &getText() const { return text; }

  void clear() {
    setText("");
  }

  void selectAll() {
    m_selectionStart = 0;
    m_selectionEnd = static_cast<int>(text.length());
    m_cursorPos = m_selectionEnd;
  }

  void deselect() {
    m_selectionStart = -1;
    m_selectionEnd = -1;
  }

  bool hasSelection() const {
    return m_selectionStart >= 0 && m_selectionEnd >= 0 && m_selectionStart != m_selectionEnd;
  }

  std::string getSelectedText() const {
    if (!hasSelection()) return "";
    int start = std::min(m_selectionStart, m_selectionEnd);
    int len = std::abs(m_selectionEnd - m_selectionStart);
    return text.substr(start, len);
  }

  void deleteSelection() {
    if (!hasSelection()) return;
    int start = std::min(m_selectionStart, m_selectionEnd);
    int len = std::abs(m_selectionEnd - m_selectionStart);
    text.erase(start, len);
    m_cursorPos = start;
    deselect();
    text_changed.emit(text);
  }

  void onProcess(float delta) override {
    if (hasFocus()) {
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
        grabFocus();
        m_blinkTimer = 0.0f;
        m_showCursor = true;

        // Check if clear button clicked
        Rect2 rect = getGlobalRect();
        if (clearButtonEnabled && !text.empty()) {
          Rect2 clearRect(rect.position.x + rect.size.x - 24.0f, rect.position.y + (rect.size.y - 20.0f) * 0.5f, 20.0f, 20.0f);
          if (clearRect.hasPoint(event.mousePosition)) {
            clear();
            const_cast<InputEvent &>(event).setHandled();
            return;
          }
        }

        m_cursorPos = getCursorPosFromMouse(event.mousePosition.x);
        m_selectionStart = m_cursorPos;
        m_selectionEnd = m_cursorPos;
        m_isMouseSelecting = true;
        const_cast<InputEvent &>(event).setHandled();
      } else {
        m_isMouseSelecting = false;
      }
    } else if (event.type == InputEventType::MouseMotion && m_isMouseSelecting) {
      m_cursorPos = getCursorPosFromMouse(event.mousePosition.x);
      m_selectionEnd = m_cursorPos;
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void onInput(const InputEvent &event) override {
    Control::onInput(event);

    if (!hasFocus() || !editable) return;

    if (event.type == InputEventType::MouseButton && event.isPressed()) {
      if (!getGlobalRect().hasPoint(event.mousePosition)) {
        releaseFocus();
        deselect();
      }
    } else if (event.type == InputEventType::Key && event.isPressed()) {
      handleKeyPress(event);
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    Color bg = (backgroundColor.a > 0.0f) ? backgroundColor : getThemeColor("bg_color", "LineEdit", Color::from_rgba8(30, 32, 42));
    Color border = (borderColor.a > 0.0f) ? borderColor : getThemeColor("border_color", "LineEdit", Color::from_rgba8(75, 80, 105));
    Color focusBorder = (focusBorderColor.a > 0.0f) ? focusBorderColor : getThemeColor("focus_border_color", "LineEdit", Color::from_rgba8(52, 120, 246));
    Color txtCol = (fontColor.a > 0.0f) ? fontColor : getThemeColor("font_color", "LineEdit", Color::WHITE);
    Color phCol = (placeholderColor.a > 0.0f) ? placeholderColor : getThemeColor("placeholder_color", "LineEdit", Color::from_rgba8(120, 125, 145));
    Color curCol = (cursorColor.a > 0.0f) ? cursorColor : getThemeColor("cursor_color", "LineEdit", Color::WHITE);
    float cr = (cornerRadius >= 0.0f) ? cornerRadius : static_cast<float>(getThemeConstant("corner_radius", "LineEdit", 4));
    float bw = (borderWidth >= 0.0f) ? borderWidth : static_cast<float>(getThemeConstant("border_width", "LineEdit", 1));
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "LineEdit", 16));

    // 1. Draw Background & Focus Border
    Color activeBorder = hasFocus() ? focusBorder : border;
    float activeBorderWidth = hasFocus() ? 1.5f : bw;

    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, cr,
                                      bg * modulate, activeBorder * modulate,
                                      activeBorderWidth);

    // 2. Compute Display Text
    std::string displayText = text;
    if (secret) {
      displayText = std::string(text.length(), '*');
    }

    Ref<Font> activeFont = font ? font : getThemeFont("font", "LineEdit");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float textX = rect.position.x + 10.0f;
    float textY = rect.position.y + (rect.size.y - activeSize) * 0.5f;

    // 3. Draw Selection Highlight if present
    if (hasSelection() && !displayText.empty()) {
      int selMin = std::clamp(std::min(m_selectionStart, m_selectionEnd), 0, static_cast<int>(displayText.length()));
      int selMax = std::clamp(std::max(m_selectionStart, m_selectionEnd), 0, static_cast<int>(displayText.length()));

      float selStartX = textX + f.getStringSize(displayText.substr(0, selMin), activeSize).x;
      float selW = f.getStringSize(displayText.substr(selMin, selMax - selMin), activeSize).x;
      float selH = activeSize * 1.15f;
      float selY = rect.position.y + (rect.size.y - selH) * 0.5f;

      Renderer2D::drawRectScreen(Vector2(selStartX, selY), Vector2(selW, selH), selectionColor * modulate, true);
    }

    if (displayText.empty() && !placeholderText.empty() && !hasFocus()) {
      // Draw Placeholder Text
      Renderer2D::drawText(placeholderText, Vector2(textX, textY),
                           phCol * modulate, activeSize, activeFont);
    } else if (!displayText.empty()) {
      // Draw Input Text
      Renderer2D::drawText(displayText, Vector2(textX, textY),
                           txtCol * modulate, activeSize, activeFont);
    }

    // 4. Draw Blinking Cursor when Focused
    if (hasFocus() && m_showCursor) {
      std::string textBeforeCursor = displayText.substr(0, std::clamp(m_cursorPos, 0, static_cast<int>(displayText.length())));
      Vector2 textSize = f.getStringSize(textBeforeCursor, activeSize);

      float cursorX = textX + textSize.x;
      float cursorH = activeSize * 1.1f;
      float cursorY = rect.position.y + (rect.size.y - cursorH) * 0.5f;

      Renderer2D::drawRectScreen(Vector2(cursorX, cursorY), Vector2(2.0f, cursorH),
                                curCol * modulate, true);
    }

    // 5. Clear Button
    if (clearButtonEnabled && !text.empty()) {
      float cbSize = 16.0f;
      float cbX = rect.position.x + rect.size.x - cbSize - 8.0f;
      float cbY = rect.position.y + (rect.size.y - cbSize) * 0.5f;
      Renderer2D::drawCircle(Vector2(cbX + cbSize * 0.5f, cbY + cbSize * 0.5f), cbSize * 0.5f,
                             Color::from_rgba8(80, 85, 105), true);
      Renderer2D::drawText("x", Vector2(cbX + 4.5f, cbY + 1.0f), Color::WHITE, 12.0f, activeFont);
    }
  }

private:
  int getCursorPosFromMouse(float mouseX) {
    Ref<Font> activeFont = font ? font : getThemeFont("font", "LineEdit");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f) ? fontSize : 16.0f;

    float textX = getGlobalRect().position.x + 10.0f;
    float relX = mouseX - textX;
    if (relX <= 0.0f) return 0;

    std::string displayText = secret ? std::string(text.length(), '*') : text;
    for (size_t i = 1; i <= displayText.length(); ++i) {
      float w = f.getStringSize(displayText.substr(0, i), activeSize).x;
      if (w >= relX) {
        return static_cast<int>(i);
      }
    }
    return static_cast<int>(displayText.length());
  }

  void handleKeyPress(const InputEvent &ev) {
    bool ctrl = ev.ctrl;

    // Clipboard: Copy
    if (ctrl && ev.key == Key::C) {
      if (hasSelection()) {
        SDL_SetClipboardText(getSelectedText().c_str());
      }
      return;
    }

    // Clipboard: Cut
    if (ctrl && ev.key == Key::X) {
      if (hasSelection()) {
        SDL_SetClipboardText(getSelectedText().c_str());
        deleteSelection();
      }
      return;
    }

    // Clipboard: Paste
    if (ctrl && ev.key == Key::V) {
      if (SDL_HasClipboardText()) {
        char *clip = SDL_GetClipboardText();
        if (clip) {
          if (hasSelection()) deleteSelection();
          text.insert(m_cursorPos, clip);
          m_cursorPos += static_cast<int>(std::strlen(clip));
          SDL_free(clip);
          text_changed.emit(text);
        }
      }
      return;
    }

    // Select All (Ctrl+A)
    if (ctrl && ev.key == Key::A) {
      selectAll();
      return;
    }

    if (ev.key == Key::Backspace) {
      if (hasSelection()) {
        deleteSelection();
      } else if (m_cursorPos > 0 && !text.empty()) {
        text.erase(m_cursorPos - 1, 1);
        m_cursorPos--;
        text_changed.emit(text);
      }
    } else if (ev.key == Key::Delete) {
      if (hasSelection()) {
        deleteSelection();
      } else if (m_cursorPos < static_cast<int>(text.length())) {
        text.erase(m_cursorPos, 1);
        text_changed.emit(text);
      }
    } else if (ev.key == Key::Left) {
      if (ev.shift) {
        if (!hasSelection()) m_selectionStart = m_cursorPos;
        if (m_cursorPos > 0) m_cursorPos--;
        m_selectionEnd = m_cursorPos;
      } else {
        deselect();
        if (m_cursorPos > 0) m_cursorPos--;
      }
    } else if (ev.key == Key::Right) {
      if (ev.shift) {
        if (!hasSelection()) m_selectionStart = m_cursorPos;
        if (m_cursorPos < static_cast<int>(text.length())) m_cursorPos++;
        m_selectionEnd = m_cursorPos;
      } else {
        deselect();
        if (m_cursorPos < static_cast<int>(text.length())) m_cursorPos++;
      }
    } else if (ev.key == Key::Home) {
      if (ev.shift) {
        if (!hasSelection()) m_selectionStart = m_cursorPos;
        m_cursorPos = 0;
        m_selectionEnd = 0;
      } else {
        deselect();
        m_cursorPos = 0;
      }
    } else if (ev.key == Key::End) {
      if (ev.shift) {
        if (!hasSelection()) m_selectionStart = m_cursorPos;
        m_cursorPos = static_cast<int>(text.length());
        m_selectionEnd = m_cursorPos;
      } else {
        deselect();
        m_cursorPos = static_cast<int>(text.length());
      }
    } else if (ev.key == Key::Enter) {
      text_submitted.emit(text);
      releaseFocus();
    } else if (!ctrl) {
      // Character typing
      char c = keyToChar(ev.key, ev.shift);
      if (c != 0) {
        if (hasSelection()) deleteSelection();
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

  int m_cursorPos = 0;
  int m_selectionStart = -1;
  int m_selectionEnd = -1;
  bool m_isMouseSelecting = false;
  float m_blinkTimer = 0.0f;
  bool m_showCursor = true;
};

