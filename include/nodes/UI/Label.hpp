#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "renderers/Font.hpp"
#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

enum class HorizontalAlignment {
  Left,
  Center,
  Right,
  Fill
};

enum class VerticalAlignment {
  Top,
  Center,
  Bottom
};

// UI Label Node (inspired by Godot Label) for displaying single/multi-line styled text.
class Label : public Control {
public:
  std::string text;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f; // 0 = inherits from active theme
  Color fontColor = Color(0.0f, 0.0f, 0.0f, 0.0f); // transparent sentinel = inherits from active theme
  Color shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
  Vector2 shadowOffset{1.0f, 1.0f};
  Color outlineColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
  float outlineSize = 0.0f;
  float lineSpacing = 3.0f;
  bool autowrap = false;
  bool clipText = false;

  HorizontalAlignment horizontalAlignment = HorizontalAlignment::Left;
  VerticalAlignment verticalAlignment = VerticalAlignment::Top;

  Label() : Control("Label") {
    mouseFilter = MouseFilter::Ignore;
  }

  explicit Label(std::string labelText, float size = 0.0f, const Color &color = Color(0, 0, 0, 0))
      : Control("Label"), text(std::move(labelText)), fontSize(size), fontColor(color) {
    mouseFilter = MouseFilter::Ignore;
  }

  void setText(std::string newText) { text = std::move(newText); }
  const std::string &getText() const { return text; }

  void drawControl() override {
    if (text.empty()) return;

    Rect2 rect = getGlobalRect();
    Ref<Font> activeFont = font ? font : getThemeFont("font", "Label");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();

    float activeSize = (fontSize > 0.0f)
                           ? fontSize
                           : static_cast<float>(getThemeFontSize("font_size", "Label", 18));
    Color activeColor = (fontColor.a > 0.0f)
                            ? fontColor
                            : getThemeColor("font_color", "Label", Color::WHITE);
    Color activeShadow = (shadowColor.a > 0.0f)
                             ? shadowColor
                             : getThemeColor("shadow_color", "Label", Color(0.0f, 0.0f, 0.0f, 0.0f));

    // Split lines
    std::vector<std::string> lines;
    if (autowrap && rect.size.x > 0.0f) {
      lines = wrapText(text, f, activeSize, rect.size.x);
    } else {
      std::stringstream ss(text);
      std::string line;
      while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
      }
    }
    if (lines.empty()) return;

    float fontH = activeSize;
    float totalH = lines.size() * fontH + (lines.size() > 1 ? (lines.size() - 1) * lineSpacing : 0.0f);

    float startY = rect.position.y;
    if (verticalAlignment == VerticalAlignment::Center) {
      startY = rect.position.y + (rect.size.y - totalH) * 0.5f;
    } else if (verticalAlignment == VerticalAlignment::Bottom) {
      startY = rect.position.y + (rect.size.y - totalH);
    }

    for (size_t i = 0; i < lines.size(); ++i) {
      const auto &line = lines[i];
      Vector2 lineSize = f.getStringSize(line, activeSize);

      float drawX = rect.position.x;
      if (horizontalAlignment == HorizontalAlignment::Center) {
        drawX = rect.position.x + (rect.size.x - lineSize.x) * 0.5f;
      } else if (horizontalAlignment == HorizontalAlignment::Right) {
        drawX = rect.position.x + (rect.size.x - lineSize.x);
      }

      float drawY = startY + i * (fontH + lineSpacing);

      // Optional text outline
      if (outlineColor.a > 0.0f && outlineSize > 0.0f) {
        for (float ox = -outlineSize; ox <= outlineSize; ox += outlineSize) {
          for (float oy = -outlineSize; oy <= outlineSize; oy += outlineSize) {
            if (ox != 0.0f || oy != 0.0f) {
              Renderer2D::drawText(line, Vector2(drawX + ox, drawY + oy), outlineColor * modulate, activeSize, activeFont);
            }
          }
        }
      }

      // Optional text shadow
      if (activeShadow.a > 0.0f) {
        Renderer2D::drawText(line, Vector2(drawX + shadowOffset.x, drawY + shadowOffset.y),
                             activeShadow * modulate, activeSize, activeFont);
      }

      // Text foreground
      Renderer2D::drawText(line, Vector2(drawX, drawY), activeColor * modulate, activeSize, activeFont);
    }
  }

private:
  static std::vector<std::string> wrapText(const std::string &str, const Font &f, float size, float maxW) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string paragraph;

    while (std::getline(ss, paragraph, '\n')) {
      std::stringstream words(paragraph);
      std::string word;
      std::string currentLine;

      while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        Vector2 testSize = f.getStringSize(testLine, size);
        if (testSize.x > maxW && !currentLine.empty()) {
          result.push_back(currentLine);
          currentLine = word;
        } else {
          currentLine = testLine;
        }
      }
      if (!currentLine.empty()) {
        result.push_back(currentLine);
      }
    }
    return result;
  }
};
