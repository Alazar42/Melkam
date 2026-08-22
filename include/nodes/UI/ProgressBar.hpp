#pragma once

#include "nodes/UI/Control.hpp"
#include "renderers/Font.hpp"
#include <algorithm>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

// Progress / Health / Mana / Loading Bar UI Node (inspired by Godot ProgressBar).
class ProgressBar : public Control {
public:
  // Signals
  Signal<float> value_changed;

  float minValue = 0.0f;
  float maxValue = 100.0f;
  float value = 100.0f;
  float step = 0.01f;
  bool showPercentage = true;

  // Colors & Theme Styling
  Color fillColor = Color::from_rgba8(52, 199, 89);       // Bright Health/Progress Green
  Color backgroundColor = Color::from_rgba8(35, 36, 45); // Dark track background
  Color borderColor = Color::from_rgba8(80, 85, 105);
  Color fontColor = Color::WHITE;
  float borderWidth = 1.0f;
  float cornerRadius = 3.0f;

  ProgressBar() : Control("ProgressBar") {
    customMinimumSize = {160.0f, 24.0f};
    mouseFilter = MouseFilter::Ignore;
  }

  void setValue(float val) {
    float clamped = std::clamp(val, minValue, maxValue);
    if (step > 0.0f) {
      clamped = std::round(clamped / step) * step;
    }
    if (clamped != value) {
      value = clamped;
      value_changed.emit(value);
    }
  }

  float getValue() const { return value; }

  // Returns progress ratio between [0.0 .. 1.0]
  float getRatio() const {
    if (maxValue <= minValue) return 0.0f;
    return std::clamp((value - minValue) / (maxValue - minValue), 0.0f, 1.0f);
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    // 1. Draw track background
    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, cornerRadius,
                                      backgroundColor * modulate, borderColor * modulate,
                                      borderWidth);

    // 2. Draw progress fill bar
    float ratio = getRatio();
    if (ratio > 0.0f) {
      float innerPad = borderWidth;
      float fillW = (rect.size.x - (innerPad * 2.0f)) * ratio;
      float fillH = rect.size.y - (innerPad * 2.0f);
      Vector2 fillPos = rect.position + Vector2(innerPad, innerPad);

      Renderer2D::drawRoundedRectScreen(fillPos, Vector2(fillW, fillH), cornerRadius,
                                        fillColor * modulate);
    }

    // 3. Optional centered percentage text
    if (showPercentage) {
      int percent = static_cast<int>(ratio * 100.0f + 0.5f);
      std::string text = std::to_string(percent) + "%";

      const Font &font = *Font::getDefaultFont();
      Vector2 textSize = font.getStringSize(text, 14.0f);

      float textX = rect.position.x + (rect.size.x - textSize.x) * 0.5f;
      float textY = rect.position.y + (rect.size.y - textSize.y) * 0.5f;

      Renderer2D::drawText(text, Vector2(textX, textY), fontColor * modulate, 14.0f);
    }
  }
};
