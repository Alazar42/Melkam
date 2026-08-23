#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "renderers/Font.hpp"
#include "renderers/Renderer2D.hpp"
#include "time.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Multi-Line Text Editor UI Node (inspired by Godot TextEdit)
class TextEdit : public Control {
public:
  // Signals
  Signal<> text_changed;
  Signal<> cursor_changed;

  std::vector<std::string> lines;
  int cursorLine = 0;
  int cursorColumn = 0;

  bool editable = true;
  bool showLineNumbers = true;
  bool highlightCurrentLine = true;
  float lineSpacing = 4.0f;
  float scrollVertical = 0.0f;
  float scrollHorizontal = 0.0f;

  Ref<Font> font = nullptr;
  float fontSize = 0.0f;

  // Colors & Theme Styling
  Color backgroundColor = Color(0, 0, 0, 0);
  Color gutterColor = Color::from_rgba8(24, 26, 34);
  Color gutterTextColor = Color::from_rgba8(100, 105, 125);
  Color currentLineColor = Color::from_rgba8(40, 44, 58, 100);
  Color fontColor = Color(0, 0, 0, 0);
  Color borderColor = Color(0, 0, 0, 0);
  Color focusBorderColor = Color(0, 0, 0, 0);
  Color cursorColor = Color(0, 0, 0, 0);
  Color selectionColor = Color::from_rgba8(52, 120, 246, 120);

  TextEdit() : Control("TextEdit") {
    customMinimumSize = {240.0f, 140.0f};
    mouseFilter = MouseFilter::Stop;
    focusMode = FocusMode::All;
    lines.push_back("");
  }

  void setText(const std::string &fullText) {
    lines.clear();
    std::stringstream ss(fullText);
    std::string line;
    while (std::getline(ss, line, '\n')) {
      lines.push_back(line);
    }
    if (lines.empty()) lines.push_back("");
    cursorLine = 0;
    cursorColumn = 0;
    text_changed.emit();
    cursor_changed.emit();
  }

  std::string getText() const {
    std::string result;
    for (size_t i = 0; i < lines.size(); ++i) {
      result += lines[i];
      if (i + 1 < lines.size()) result += "\n";
    }
    return result;
  }

  void clear() {
    setText("");
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
    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left && event.isPressed()) {
      grabFocus();
      m_blinkTimer = 0.0f;
      m_showCursor = true;

      // Calculate clicked line and column
      Rect2 rect = getGlobalRect();
      Ref<Font> activeFont = font ? font : getThemeFont("font", "TextEdit");
      const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
      float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "TextEdit", 15));
      float lineH = activeSize + lineSpacing;
      float gutterW = showLineNumbers ? 40.0f : 0.0f;

      float relY = (event.mousePosition.y - rect.position.y) + scrollVertical;
      cursorLine = std::clamp(static_cast<int>(relY / lineH), 0, static_cast<int>(lines.size()) - 1);

      float relX = (event.mousePosition.x - (rect.position.x + gutterW + 8.0f)) + scrollHorizontal;
      cursorColumn = 0;
      if (relX > 0.0f && cursorLine < static_cast<int>(lines.size())) {
        const auto &l = lines[cursorLine];
        for (size_t i = 1; i <= l.length(); ++i) {
          if (f.getStringSize(l.substr(0, i), activeSize).x >= relX) {
            cursorColumn = static_cast<int>(i);
            break;
          }
          cursorColumn = static_cast<int>(l.length());
        }
      }
      cursor_changed.emit();
      const_cast<InputEvent &>(event).setHandled();
    } else if (event.type == InputEventType::MouseWheel) {
      scrollVertical -= event.mouseScroll.y * 24.0f;
      scrollVertical = std::max(0.0f, scrollVertical);
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void onInput(const InputEvent &event) override {
    Control::onInput(event);

    if (!hasFocus() || !editable) return;

    if (event.type == InputEventType::MouseButton && event.isPressed()) {
      if (!getGlobalRect().hasPoint(event.mousePosition)) {
        releaseFocus();
      }
    } else if (event.type == InputEventType::Key && event.isPressed()) {
      handleKeyPress(event);
      const_cast<InputEvent &>(event).setHandled();
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    Color bg = (backgroundColor.a > 0.0f) ? backgroundColor : getThemeColor("bg_color", "LineEdit", Color::from_rgba8(28, 30, 40));
    Color border = (borderColor.a > 0.0f) ? borderColor : getThemeColor("border_color", "LineEdit", Color::from_rgba8(65, 70, 90));
    Color focusBorder = (focusBorderColor.a > 0.0f) ? focusBorderColor : getThemeColor("focus_border_color", "LineEdit", Color::from_rgba8(52, 120, 246));
    Color txtCol = (fontColor.a > 0.0f) ? fontColor : getThemeColor("font_color", "LineEdit", Color::WHITE);
    Color curCol = (cursorColor.a > 0.0f) ? cursorColor : getThemeColor("cursor_color", "LineEdit", Color::WHITE);

    Ref<Font> activeFont = font ? font : getThemeFont("font", "TextEdit");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "TextEdit", 15));
    float lineH = activeSize + lineSpacing;
    float gutterW = showLineNumbers ? 40.0f : 0.0f;

    // 1. Draw Main Frame & Gutter
    Color activeBorder = hasFocus() ? focusBorder : border;
    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, 4.0f, bg * modulate, activeBorder * modulate, hasFocus() ? 1.5f : 1.0f);

    if (showLineNumbers) {
      Renderer2D::drawRectScreen(rect.position, Vector2(gutterW, rect.size.y), gutterColor * modulate, true);
      Renderer2D::drawRectScreen(Vector2(rect.position.x + gutterW, rect.position.y), Vector2(1.0f, rect.size.y), border * modulate, true);
    }

    // 2. Draw Current Line Highlight
    if (highlightCurrentLine && hasFocus()) {
      float highlightY = rect.position.y + (cursorLine * lineH) - scrollVertical;
      if (highlightY >= rect.position.y && highlightY < rect.position.y + rect.size.y) {
        Renderer2D::drawRectScreen(Vector2(rect.position.x + gutterW + 1.0f, highlightY),
                                  Vector2(rect.size.x - gutterW - 2.0f, lineH),
                                  currentLineColor * modulate, true);
      }
    }

    // 3. Draw Lines & Line Numbers
    for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
      float lineY = rect.position.y + (i * lineH) - scrollVertical;
      if (lineY + lineH < rect.position.y || lineY > rect.position.y + rect.size.y) continue;

      if (showLineNumbers) {
        std::string numStr = std::to_string(i + 1);
        Vector2 numSize = f.getStringSize(numStr, activeSize - 2.0f);
        float numX = rect.position.x + gutterW - numSize.x - 6.0f;
        Renderer2D::drawText(numStr, Vector2(numX, lineY + (lineH - activeSize) * 0.5f),
                             gutterTextColor * modulate, activeSize - 2.0f, activeFont);
      }

      float textX = rect.position.x + gutterW + 8.0f - scrollHorizontal;
      float textY = lineY + (lineH - activeSize) * 0.5f;
      Renderer2D::drawText(lines[i], Vector2(textX, textY), txtCol * modulate, activeSize, activeFont);

      // Draw Cursor if on this line
      if (hasFocus() && m_showCursor && i == cursorLine) {
        std::string beforeCursor = lines[i].substr(0, std::clamp(cursorColumn, 0, static_cast<int>(lines[i].length())));
        float cursorX = textX + f.getStringSize(beforeCursor, activeSize).x;
        Renderer2D::drawRectScreen(Vector2(cursorX, textY), Vector2(2.0f, activeSize), curCol * modulate, true);
      }
    }
  }

private:
  void handleKeyPress(const InputEvent &ev) {
    bool ctrl = ev.ctrl;

    // Clipboard: Paste
    if (ctrl && ev.key == Key::V) {
      if (SDL_HasClipboardText()) {
        char *clip = SDL_GetClipboardText();
        if (clip) {
          std::string textToPaste(clip);
          insertText(textToPaste);
          SDL_free(clip);
        }
      }
      return;
    }

    if (ev.key == Key::Enter) {
      if (cursorLine >= 0 && cursorLine < static_cast<int>(lines.size())) {
        std::string remainder = lines[cursorLine].substr(cursorColumn);
        lines[cursorLine] = lines[cursorLine].substr(0, cursorColumn);
        lines.insert(lines.begin() + cursorLine + 1, remainder);
        cursorLine++;
        cursorColumn = 0;
        text_changed.emit();
        cursor_changed.emit();
      }
    } else if (ev.key == Key::Backspace) {
      if (cursorColumn > 0) {
        lines[cursorLine].erase(cursorColumn - 1, 1);
        cursorColumn--;
        text_changed.emit();
        cursor_changed.emit();
      } else if (cursorLine > 0) {
        cursorColumn = static_cast<int>(lines[cursorLine - 1].length());
        lines[cursorLine - 1] += lines[cursorLine];
        lines.erase(lines.begin() + cursorLine);
        cursorLine--;
        text_changed.emit();
        cursor_changed.emit();
      }
    } else if (ev.key == Key::Delete) {
      if (cursorColumn < static_cast<int>(lines[cursorLine].length())) {
        lines[cursorLine].erase(cursorColumn, 1);
        text_changed.emit();
      } else if (cursorLine + 1 < static_cast<int>(lines.size())) {
        lines[cursorLine] += lines[cursorLine + 1];
        lines.erase(lines.begin() + cursorLine + 1);
        text_changed.emit();
      }
    } else if (ev.key == Key::Up) {
      if (cursorLine > 0) {
        cursorLine--;
        cursorColumn = std::clamp(cursorColumn, 0, static_cast<int>(lines[cursorLine].length()));
        cursor_changed.emit();
      }
    } else if (ev.key == Key::Down) {
      if (cursorLine + 1 < static_cast<int>(lines.size())) {
        cursorLine++;
        cursorColumn = std::clamp(cursorColumn, 0, static_cast<int>(lines[cursorLine].length()));
        cursor_changed.emit();
      }
    } else if (ev.key == Key::Left) {
      if (cursorColumn > 0) {
        cursorColumn--;
      } else if (cursorLine > 0) {
        cursorLine--;
        cursorColumn = static_cast<int>(lines[cursorLine].length());
      }
      cursor_changed.emit();
    } else if (ev.key == Key::Right) {
      if (cursorColumn < static_cast<int>(lines[cursorLine].length())) {
        cursorColumn++;
      } else if (cursorLine + 1 < static_cast<int>(lines.size())) {
        cursorLine++;
        cursorColumn = 0;
      }
      cursor_changed.emit();
    } else if (ev.key == Key::Home) {
      cursorColumn = 0;
      cursor_changed.emit();
    } else if (ev.key == Key::End) {
      cursorColumn = static_cast<int>(lines[cursorLine].length());
      cursor_changed.emit();
    } else if (!ctrl) {
      char c = LineEditChar(ev.key, ev.shift);
      if (c != 0) {
        lines[cursorLine].insert(cursorColumn, 1, c);
        cursorColumn++;
        text_changed.emit();
        cursor_changed.emit();
      }
    }

    m_blinkTimer = 0.0f;
    m_showCursor = true;
  }

  void insertText(const std::string &txt) {
    for (char c : txt) {
      if (c == '\n') {
        std::string remainder = lines[cursorLine].substr(cursorColumn);
        lines[cursorLine] = lines[cursorLine].substr(0, cursorColumn);
        lines.insert(lines.begin() + cursorLine + 1, remainder);
        cursorLine++;
        cursorColumn = 0;
      } else if (c != '\r') {
        lines[cursorLine].insert(cursorColumn, 1, c);
        cursorColumn++;
      }
    }
    text_changed.emit();
    cursor_changed.emit();
  }

  static char LineEditChar(Key key, bool shift) {
    int k = static_cast<int>(key);
    if (k >= static_cast<int>(Key::A) && k <= static_cast<int>(Key::Z)) {
      char base = 'a' + (k - static_cast<int>(Key::A));
      return shift ? static_cast<char>(std::toupper(base)) : base;
    }
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

  float m_blinkTimer = 0.0f;
  bool m_showCursor = true;
};
