#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "renderers/Font.hpp"
#include "renderers/Renderer2D.hpp"
#include <SDL3/SDL.h>
#include <algorithm>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

struct RichSpan {
  std::string text;
  Color color = Color::WHITE;
  float fontSize = 16.0f;
  bool bold = false;
  bool italic = false;
  bool underline = false;
  std::string url;
};

struct RichLine {
  std::vector<RichSpan> spans;
  float height = 0.0f;
};

// Rich Text BBCode-Enabled UI Node (inspired by Godot RichTextLabel)
class RichTextLabel : public Control {
public:
  // Signals
  Signal<std::string> meta_clicked;
  Signal<std::string> meta_hovered;

  std::string bbcode;
  bool bbcodeEnabled = true;
  bool autowrap = true;
  bool scrollActive = true;
  float scrollOffset = 0.0f;
  Ref<Font> font = nullptr;
  float defaultFontSize = 0.0f;
  Color defaultColor = Color(0, 0, 0, 0);

  RichTextLabel() : Control("RichTextLabel") {
    customMinimumSize = {200.0f, 60.0f};
    mouseFilter = MouseFilter::Stop;
    meta_clicked.connect([](const std::string &url) {
      if (!url.empty()) {
        SDL_OpenURL(url.c_str());
      }
    });
  }

  explicit RichTextLabel(std::string text)
      : Control("RichTextLabel"), bbcode(std::move(text)) {
    customMinimumSize = {200.0f, 60.0f};
    mouseFilter = MouseFilter::Stop;
    meta_clicked.connect([](const std::string &url) {
      if (!url.empty()) {
        SDL_OpenURL(url.c_str());
      }
    });
  }


  void setText(std::string text) {
    bbcode = std::move(text);
    m_needsRebuild = true;
  }

  void setBbcode(std::string text) {
    setText(std::move(text));
  }


  void appendText(const std::string &moreText) {
    bbcode += moreText;
    m_needsRebuild = true;
  }

  void clear() {
    bbcode.clear();
    m_lines.clear();
    m_needsRebuild = true;
  }

  void onGuiInput(const InputEvent &event) override {
    if (event.type == InputEventType::MouseWheel && scrollActive) {
      scrollOffset -= event.mouseScroll.y * 24.0f;
      scrollOffset = std::max(0.0f, scrollOffset);
      const_cast<InputEvent &>(event).setHandled();
    } else if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left && event.isPressed()) {
      if (!m_hoveredUrl.empty()) {
        meta_clicked.emit(m_hoveredUrl);
        const_cast<InputEvent &>(event).setHandled();
      }
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    Ref<Font> activeFont = font ? font : getThemeFont("font", "RichTextLabel");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float baseSize = (defaultFontSize > 0.0f) ? defaultFontSize : static_cast<float>(getThemeFontSize("font_size", "RichTextLabel", 16));
    Color baseCol = (defaultColor.a > 0.0f) ? defaultColor : getThemeColor("font_color", "RichTextLabel", Color::WHITE);

    if (m_needsRebuild || m_lastWidth != rect.size.x) {
      rebuildLayout(f, baseSize, baseCol, rect.size.x);
      m_needsRebuild = false;
      m_lastWidth = rect.size.x;
    }

    Vector2 mousePos = Input::getMousePosition();
    m_hoveredUrl.clear();

    float totalContentH = 0.0f;
    for (const auto &line : m_lines) totalContentH += line.height + 3.0f;

    float maxScroll = std::max(0.0f, totalContentH - rect.size.y);
    scrollOffset = std::clamp(scrollOffset, 0.0f, maxScroll);

    float currY = rect.position.y - scrollOffset;

    for (const auto &line : m_lines) {
      if (currY + line.height >= rect.position.y && currY <= rect.position.y + rect.size.y) {
        float currX = rect.position.x;
        for (const auto &span : line.spans) {
          Vector2 spanSize = f.getStringSize(span.text, span.fontSize);
          Rect2 spanRect(currX, currY, spanSize.x, line.height);

          bool isHover = spanRect.hasPoint(mousePos);
          if (isHover && !span.url.empty()) {
            m_hoveredUrl = span.url;
            meta_hovered.emit(span.url);
          }

          Color drawCol = span.color;
          if (!span.url.empty() && isHover) {
            drawCol = Color::from_rgba8(120, 180, 255);
          }

          float textY = currY + (line.height - span.fontSize) * 0.5f;
          Renderer2D::drawText(span.text, Vector2(currX, textY), drawCol * modulate, span.fontSize, activeFont);

          if (span.underline || !span.url.empty()) {
            Renderer2D::drawRectScreen(Vector2(currX, textY + span.fontSize + 1.0f), Vector2(spanSize.x, 1.5f), drawCol * modulate, true);
          }

          currX += spanSize.x;
        }
      }
      currY += line.height + 3.0f;
    }
  }

private:
  void rebuildLayout(const Font &f, float baseSize, const Color &baseCol, float maxW) {
    m_lines.clear();
    if (bbcode.empty()) return;

    std::vector<RichSpan> rawSpans = parseBBCode(bbcode, baseSize, baseCol);
    if (!autowrap || maxW <= 0.0f) {
      // Single line / raw line breaks
      RichLine curLine;
      curLine.height = baseSize;
      for (auto &span : rawSpans) {
        if (span.text == "\n") {
          m_lines.push_back(curLine);
          curLine = RichLine();
          curLine.height = baseSize;
        } else {
          curLine.height = std::max(curLine.height, span.fontSize);
          curLine.spans.push_back(span);
        }
      }
      if (!curLine.spans.empty()) m_lines.push_back(curLine);
      return;
    }

    // Auto-wrapping layout
    RichLine currentLine;
    currentLine.height = baseSize;
    float currentLineWidth = 0.0f;

    for (const auto &span : rawSpans) {
      if (span.text == "\n") {
        m_lines.push_back(currentLine);
        currentLine = RichLine();
        currentLine.height = baseSize;
        currentLineWidth = 0.0f;
        continue;
      }

      std::stringstream ss(span.text);
      std::string word;
      bool firstInSpan = true;

      while (ss >> word) {
        std::string piece = firstInSpan ? word : (" " + word);
        firstInSpan = false;
        Vector2 pieceSize = f.getStringSize(piece, span.fontSize);

        if (currentLineWidth + pieceSize.x > maxW && currentLineWidth > 0.0f) {
          m_lines.push_back(currentLine);
          currentLine = RichLine();
          currentLine.height = span.fontSize;
          currentLineWidth = 0.0f;
          piece = word;
          pieceSize = f.getStringSize(piece, span.fontSize);
        }

        RichSpan wordSpan = span;
        wordSpan.text = piece;
        currentLine.spans.push_back(wordSpan);
        currentLine.height = std::max(currentLine.height, span.fontSize);
        currentLineWidth += pieceSize.x;
      }
    }
    if (!currentLine.spans.empty()) {
      m_lines.push_back(currentLine);
    }
  }

  std::vector<RichSpan> parseBBCode(const std::string &src, float baseSize, const Color &baseCol) {
    std::vector<RichSpan> spans;
    RichSpan state;
    state.fontSize = baseSize;
    state.color = baseCol;

    std::vector<RichSpan> stateStack;

    size_t i = 0;
    std::string textBuf;

    auto flushBuf = [&]() {
      if (!textBuf.empty()) {
        RichSpan s = state;
        s.text = textBuf;
        spans.push_back(s);
        textBuf.clear();
      }
    };

    while (i < src.length()) {
      if (src[i] == '\n') {
        flushBuf();
        RichSpan newlineSpan;
        newlineSpan.text = "\n";
        newlineSpan.fontSize = state.fontSize;
        spans.push_back(newlineSpan);
        i++;
      } else if (src[i] == '[' && bbcodeEnabled) {
        size_t closeBracket = src.find(']', i);
        if (closeBracket != std::string::npos) {
          flushBuf();
          std::string tag = src.substr(i + 1, closeBracket - i - 1);
          i = closeBracket + 1;

          if (tag == "b") {
            stateStack.push_back(state);
            state.bold = true;
          } else if (tag == "/b") {
            if (!stateStack.empty()) { state = stateStack.back(); stateStack.pop_back(); }
          } else if (tag == "i") {
            stateStack.push_back(state);
            state.italic = true;
          } else if (tag == "/i") {
            if (!stateStack.empty()) { state = stateStack.back(); stateStack.pop_back(); }
          } else if (tag == "u") {
            stateStack.push_back(state);
            state.underline = true;
          } else if (tag == "/u") {
            if (!stateStack.empty()) { state = stateStack.back(); stateStack.pop_back(); }
          } else if (tag.rfind("color=", 0) == 0) {
            stateStack.push_back(state);
            std::string cStr = tag.substr(6);
            if (cStr == "red") state.color = Color::RED;
            else if (cStr == "green") state.color = Color::GREEN;
            else if (cStr == "blue") state.color = Color::from_rgba8(60, 140, 255);
            else if (cStr == "yellow") state.color = Color::YELLOW;
            else if (cStr == "cyan") state.color = Color::CYAN;
            else if (cStr == "white") state.color = Color::WHITE;
            else if (cStr == "gray") state.color = Color::from_rgba8(160, 160, 170);
            else if (cStr.rfind("#", 0) == 0 && cStr.length() == 7) {
              unsigned int hex = std::stoul(cStr.substr(1), nullptr, 16);
              state.color = Color::from_rgba8((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
            }
          } else if (tag == "/color") {
            if (!stateStack.empty()) { state = stateStack.back(); stateStack.pop_back(); }
          } else if (tag.rfind("font_size=", 0) == 0) {
            stateStack.push_back(state);
            state.fontSize = static_cast<float>(std::stoi(tag.substr(10)));
          } else if (tag == "/font_size") {
            if (!stateStack.empty()) { state = stateStack.back(); stateStack.pop_back(); }
          } else if (tag.rfind("url=", 0) == 0) {
            stateStack.push_back(state);
            state.url = tag.substr(4);
            state.color = Color::from_rgba8(75, 160, 255);
            state.underline = true;
          } else if (tag == "/url") {
            if (!stateStack.empty()) { state = stateStack.back(); stateStack.pop_back(); }
          }
        } else {
          textBuf += src[i++];
        }
      } else {
        textBuf += src[i++];
      }
    }
    flushBuf();
    return spans;
  }

  std::vector<RichLine> m_lines;
  bool m_needsRebuild = true;
  float m_lastWidth = 0.0f;
  std::string m_hoveredUrl;
};
