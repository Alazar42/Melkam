#pragma once

#include "nodes/UI/Control.hpp"
#include "renderers/Font.hpp"
#include <algorithm>
#include <memory>
#include <string>

enum class HorizontalAlignment {
  Left,
  Center,
  Right
};

enum class VerticalAlignment {
  Top,
  Center,
  Bottom
};

// UI Label Node (inspired by Godot Label) for displaying styled text.
class Label : public Control {
public:
  std::string text;
  std::shared_ptr<Font> font = nullptr;
  float fontSize = 0.0f; // 0 = inherits from active theme
  Color fontColor = Color(0.0f, 0.0f, 0.0f, 0.0f); // transparent sentinel = inherits from active theme
  Color shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
  Vector2 shadowOffset{1.0f, 1.0f};

  HorizontalAlignment horizontalAlignment = HorizontalAlignment::Left;
  VerticalAlignment verticalAlignment = VerticalAlignment::Top;

  Label() : Control("Label") {
    mouseFilter = MouseFilter::Ignore;
  }

  explicit Label(std::string labelText, float size = 0.0f, const Color &color = Color(0, 0, 0, 0))
      : Control("Label"), text(std::move(labelText)), fontSize(size), fontColor(color) {
    mouseFilter = MouseFilter::Ignore;
  }

  void drawControl() override {
    if (text.empty()) return;

    Rect2 rect = getGlobalRect();
    std::shared_ptr<Font> activeFont = font ? font : getThemeFont("font", "Label");
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

    Vector2 textSize = f.getStringSize(text, activeSize);

    // Compute text position based on alignments
    float drawX = rect.position.x;
    if (horizontalAlignment == HorizontalAlignment::Center) {
      drawX = rect.position.x + (rect.size.x - textSize.x) * 0.5f;
    } else if (horizontalAlignment == HorizontalAlignment::Right) {
      drawX = rect.position.x + (rect.size.x - textSize.x);
    }

    float drawY = rect.position.y;
    if (verticalAlignment == VerticalAlignment::Center) {
      drawY = rect.position.y + (rect.size.y - textSize.y) * 0.5f;
    } else if (verticalAlignment == VerticalAlignment::Bottom) {
      drawY = rect.position.y + (rect.size.y - textSize.y);
    }

    // Optional text shadow
    if (activeShadow.a > 0.0f) {
      Renderer2D::drawText(text, Vector2(drawX + shadowOffset.x, drawY + shadowOffset.y),
                           activeShadow, activeSize, activeFont);
    }

    // Text foreground
    Renderer2D::drawText(text, Vector2(drawX, drawY), activeColor * modulate, activeSize, activeFont);
  }
};
