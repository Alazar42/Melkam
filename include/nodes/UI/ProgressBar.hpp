#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Range.hpp"
#include "renderers/Font.hpp"
#include <algorithm>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

// Progress / Health / Mana / Loading Bar UI Node (inspired by Godot ProgressBar).
class ProgressBar : public Range {
public:
  bool showPercentage = true;
  bool vertical = false;
  std::string customText; // If non-empty, overrides percentage display

  // Colors & Theme Styling
  Color fillColor = Color(0, 0, 0, 0);       // Sentinel (inherits from theme if empty)
  Color backgroundColor = Color(0, 0, 0, 0);
  Color borderColor = Color(0, 0, 0, 0);
  Color fontColor = Color(0, 0, 0, 0);
  float borderWidth = -1.0f;
  float cornerRadius = -1.0f;

  ProgressBar() : Range("ProgressBar") {
    customMinimumSize = {160.0f, 24.0f};
    mouseFilter = MouseFilter::Ignore;
    value = 100.0f;
    step = 0.01f;
  }

  explicit ProgressBar(bool isVertical, std::string nodeName = "ProgressBar")
      : Range(std::move(nodeName)), vertical(isVertical) {
    if (vertical) {
      customMinimumSize = {24.0f, 160.0f};
    } else {
      customMinimumSize = {160.0f, 24.0f};
    }
    mouseFilter = MouseFilter::Ignore;
    value = 100.0f;
    step = 0.01f;
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    Color bg = (backgroundColor.a > 0.0f)
                   ? backgroundColor
                   : getThemeColor("bg_color", "ProgressBar", Color::from_rgba8(35, 36, 45));
    Color fill = (fillColor.a > 0.0f)
                     ? fillColor
                     : getThemeColor("fill_color", "ProgressBar", Color::from_rgba8(52, 199, 89));
    Color border = (borderColor.a > 0.0f)
                       ? borderColor
                       : getThemeColor("border_color", "ProgressBar", Color::from_rgba8(80, 85, 105));
    Color textCol = (fontColor.a > 0.0f)
                        ? fontColor
                        : getThemeColor("font_color", "ProgressBar", Color::WHITE);
    float cr = (cornerRadius >= 0.0f)
                   ? cornerRadius
                   : static_cast<float>(getThemeConstant("corner_radius", "ProgressBar", 3));
    float bw = (borderWidth >= 0.0f)
                   ? borderWidth
                   : static_cast<float>(getThemeConstant("border_width", "ProgressBar", 1));

    // 1. Draw track background
    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, cr,
                                      bg * modulate, border * modulate,
                                      bw);

    // 2. Draw progress fill bar
    float ratio = getRatio();
    if (ratio > 0.0f) {
      float innerPad = std::max(1.0f, bw);
      if (!vertical) {
        float fillW = (rect.size.x - (innerPad * 2.0f)) * ratio;
        float fillH = rect.size.y - (innerPad * 2.0f);
        Vector2 fillPos = rect.position + Vector2(innerPad, innerPad);
        Renderer2D::drawRoundedRectScreen(fillPos, Vector2(fillW, fillH), cr,
                                          fill * modulate);
      } else {
        float fillW = rect.size.x - (innerPad * 2.0f);
        float fillH = (rect.size.y - (innerPad * 2.0f)) * ratio;
        Vector2 fillPos{rect.position.x + innerPad, rect.position.y + rect.size.y - innerPad - fillH};
        Renderer2D::drawRoundedRectScreen(fillPos, Vector2(fillW, fillH), cr,
                                          fill * modulate);
      }
    }

    // 3. Optional centered percentage text
    if (showPercentage || !customText.empty()) {
      std::string text = customText;
      if (text.empty()) {
        int percent = static_cast<int>(ratio * 100.0f + 0.5f);
        text = std::to_string(percent) + "%";
      }

      Ref<Font> f = getThemeFont("font", "ProgressBar");
      const Font &font = f ? *f : *Font::getDefaultFont();
      Vector2 textSize = font.getStringSize(text, 14.0f);

      float textX = rect.position.x + (rect.size.x - textSize.x) * 0.5f;
      float textY = rect.position.y + (rect.size.y - textSize.y) * 0.5f;

      Renderer2D::drawText(text, Vector2(textX, textY), textCol * modulate, 14.0f, f);
    }
  }
};

