#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/CheckBox.hpp"
#include "renderers/Font.hpp"
#include "renderers/Renderer2D.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <string>

enum class LinkUnderlineMode {
  Always,
  OnHover,
  Never
};

// Hyperlink Text Button UI Node (inspired by Godot LinkButton)
class LinkButton : public BaseButton {
public:
  std::string text;
  std::string uri;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f;
  LinkUnderlineMode underlineMode = LinkUnderlineMode::Always;

  Color fontColor = Color::from_rgba8(75, 160, 255);
  Color hoverFontColor = Color::from_rgba8(130, 200, 255);
  Color pressedFontColor = Color::from_rgba8(50, 120, 220);

  LinkButton() : BaseButton("LinkButton") {
    customMinimumSize = {80.0f, 24.0f};
    pressed.connect([this]() {
      if (!uri.empty()) {
        SDL_OpenURL(uri.c_str());
      }
    });
  }

  explicit LinkButton(std::string buttonText, std::string targetUri = "")
      : BaseButton("LinkButton"), text(std::move(buttonText)), uri(std::move(targetUri)) {
    customMinimumSize = {80.0f, 24.0f};
    pressed.connect([this]() {
      if (!uri.empty()) {
        SDL_OpenURL(uri.c_str());
      }
    });
  }

  void setText(std::string newText) {
    text = std::move(newText);
  }

  void drawControl() override {
    if (text.empty()) return;

    Ref<Font> activeFont = font ? font : getThemeFont("font", "LinkButton");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "LinkButton", 16));

    Vector2 textSize = f.getStringSize(text, activeSize);
    if (customMinimumSize.x < textSize.x + 8.0f) {
      customMinimumSize.x = textSize.x + 8.0f;
    }
    if (customMinimumSize.y < textSize.y + 4.0f) {
      customMinimumSize.y = textSize.y + 4.0f;
    }

    Rect2 rect = getGlobalRect();

    Color activeColor = fontColor;
    if (disabled) activeColor = Color::from_rgba8(120, 120, 130);
    else if (m_isDown && m_isHovered) activeColor = pressedFontColor;
    else if (m_isHovered) activeColor = hoverFontColor;

    float textX = rect.position.x;
    float textY = rect.position.y + (rect.size.y - textSize.y) * 0.5f;

    Renderer2D::drawText(text, Vector2(textX, textY), activeColor * modulate, activeSize, activeFont);

    // Draw Underline
    bool shouldUnderline = (underlineMode == LinkUnderlineMode::Always) ||
                           (underlineMode == LinkUnderlineMode::OnHover && m_isHovered);
    if (shouldUnderline) {
      float lineY = textY + textSize.y + 1.0f;
      Renderer2D::drawRectScreen(Vector2(textX, lineY), Vector2(textSize.x, 1.5f), activeColor * modulate, true);
    }
  }
};
