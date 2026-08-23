#pragma once

#include "nodes/UI/CheckBox.hpp"
#include "renderers/Font.hpp"
#include "renderers/Texture2D.hpp"
#include <algorithm>
#include <memory>
#include <string>

// Interactive Push Button Node (inspired by Godot Button) with hover, pressed styles, icons, and textures.
class Button : public BaseButton {
public:
  // Text & Visuals
  std::string text;
  std::shared_ptr<Font> font = nullptr;
  float fontSize = 0.0f; // 0 = inherits from active theme
  bool flat = false;

  // Icon Support (Godot Button.icon)
  std::shared_ptr<Texture2D> icon = nullptr;
  Vector2 iconSize{0.0f, 0.0f};

  // Optional Texture Skins (Godot StyleBoxTexture)
  std::shared_ptr<Texture2D> textureNormal = nullptr;
  std::shared_ptr<Texture2D> textureHover = nullptr;
  std::shared_ptr<Texture2D> texturePressed = nullptr;
  std::shared_ptr<Texture2D> textureDisabled = nullptr;
  float patchMarginLeft = 0.0f;
  float patchMarginTop = 0.0f;
  float patchMarginRight = 0.0f;
  float patchMarginBottom = 0.0f;

  // Colors & Theme Styling (Color(0,0,0,0) sentinels inherit from active theme)
  Color normalColor = Color(0, 0, 0, 0);
  Color hoverColor = Color(0, 0, 0, 0);
  Color pressedColor = Color(0, 0, 0, 0);
  Color disabledColor = Color(0, 0, 0, 0);
  Color fontColor = Color(0, 0, 0, 0);
  Color borderColor = Color(0, 0, 0, 0);
  float borderWidth = -1.0f;
  float cornerRadius = -1.0f;

  Button() : BaseButton("Button") {
    customMinimumSize = {120.0f, 36.0f};
  }

  explicit Button(std::string buttonText)
      : BaseButton("Button"), text(std::move(buttonText)) {
    customMinimumSize = {120.0f, 36.0f};
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    // 1. Check if textured background is used
    std::shared_ptr<Texture2D> activeTex = nullptr;
    if (disabled && textureDisabled) activeTex = textureDisabled;
    else if (m_isDown && m_isHovered && texturePressed) activeTex = texturePressed;
    else if (m_isHovered && textureHover) activeTex = textureHover;
    else if (textureNormal) activeTex = textureNormal;
    else activeTex = getThemeTexture("texture", "Button");

    if (activeTex && activeTex->isValid()) {
      if (patchMarginLeft > 0.0f || patchMarginTop > 0.0f ||
          patchMarginRight > 0.0f || patchMarginBottom > 0.0f) {
        drawNinePatch(activeTex.get(), rect);
      } else {
        Renderer2D::drawTextureScreen(activeTex.get(), rect.position, rect.size, modulate);
      }
    } else if (!flat) {
      Color norm = (normalColor.a > 0.0f)
                       ? normalColor
                       : getThemeColor("normal_color", "Button", Color::from_rgba8(45, 52, 75));
      Color hov = (hoverColor.a > 0.0f)
                      ? hoverColor
                      : getThemeColor("hover_color", "Button", Color::from_rgba8(65, 80, 120));
      Color press = (pressedColor.a > 0.0f)
                        ? pressedColor
                        : getThemeColor("pressed_color", "Button", Color::from_rgba8(30, 38, 55));
      Color dis = (disabledColor.a > 0.0f)
                      ? disabledColor
                      : getThemeColor("disabled_color", "Button", Color::from_rgba8(30, 30, 35));
      Color border = (borderColor.a > 0.0f)
                         ? borderColor
                         : getThemeColor("border_color", "Button", Color::from_rgba8(90, 100, 130));
      float cr = (cornerRadius >= 0.0f)
                     ? cornerRadius
                     : static_cast<float>(getThemeConstant("corner_radius", "Button", 4));
      float bw = (borderWidth >= 0.0f)
                     ? borderWidth
                     : static_cast<float>(getThemeConstant("border_width", "Button", 1));

      Color activeBg = norm;
      Color activeBorder = border;

      if (disabled) activeBg = dis;
      else if (m_isDown && m_isHovered) activeBg = press;
      else if (m_isHovered) activeBg = hov;

      Renderer2D::drawRoundedRectScreen(rect.position, rect.size, cr,
                                        activeBg * modulate, activeBorder * modulate,
                                        bw);
    }

    // 2. Render Icon + Label
    std::shared_ptr<Font> activeFont = font ? font : getThemeFont("font", "Button");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f)
                           ? fontSize
                           : static_cast<float>(getThemeFontSize("font_size", "Button", 18));
    Color txtCol = (fontColor.a > 0.0f)
                       ? fontColor
                       : getThemeColor("font_color", "Button", Color::WHITE);
    Color activeFontColor = disabled ? Color::from_rgba8(120, 120, 130) : txtCol;

    Vector2 textSize = text.empty() ? Vector2{0.0f, 0.0f} : f.getStringSize(text, activeSize);
    float totalContentW = textSize.x;
    float iconW = 0.0f, iconH = 0.0f;

    if (icon && icon->isValid()) {
      iconH = (iconSize.y > 0.0f) ? iconSize.y : std::min(rect.size.y - 10.0f, activeSize + 4.0f);
      iconW = (iconSize.x > 0.0f) ? iconSize.x : iconH;
      totalContentW += iconW + (text.empty() ? 0.0f : 8.0f);
    }

    float startX = rect.position.x + (rect.size.x - totalContentW) * 0.5f;
    float centerY = rect.position.y + (rect.size.y - std::max(textSize.y, iconH)) * 0.5f;

    if (m_isDown && m_isHovered) {
      centerY += 1.0f;
    }

    if (icon && icon->isValid()) {
      Renderer2D::drawTextureScreen(icon.get(), Vector2(startX, centerY + (std::max(textSize.y, iconH) - iconH) * 0.5f),
                                   Vector2(iconW, iconH), modulate);
      startX += iconW + 8.0f;
    }

    if (!text.empty()) {
      float textY = rect.position.y + (rect.size.y - textSize.y) * 0.5f;
      if (m_isDown && m_isHovered) textY += 1.0f;
      Renderer2D::drawText(text, Vector2(startX, textY), activeFontColor * modulate,
                           activeSize, activeFont);
    }
  }

private:
  void drawNinePatch(Texture2D *tex, const Rect2 &rect) {
    float tw = static_cast<float>(tex->getWidth());
    float th = static_cast<float>(tex->getHeight());

    float ml = std::clamp(patchMarginLeft, 0.0f, tw * 0.5f);
    float mr = std::clamp(patchMarginRight, 0.0f, tw * 0.5f);
    float mt = std::clamp(patchMarginTop, 0.0f, th * 0.5f);
    float mb = std::clamp(patchMarginBottom, 0.0f, th * 0.5f);

    float srcXs[4] = {0.0f, ml, tw - mr, tw};
    float srcYs[4] = {0.0f, mt, th - mb, th};

    float dstXs[4] = {rect.position.x, rect.position.x + ml, rect.position.x + rect.size.x - mr, rect.position.x + rect.size.x};
    float dstYs[4] = {rect.position.y, rect.position.y + mt, rect.position.y + rect.size.y - mb, rect.position.y + rect.size.y};

    for (int row = 0; row < 3; ++row) {
      for (int col = 0; col < 3; ++col) {
        Rect2 srcR(srcXs[col], srcYs[row], srcXs[col + 1] - srcXs[col], srcYs[row + 1] - srcYs[row]);
        Rect2 dstR(dstXs[col], dstYs[row], dstXs[col + 1] - dstXs[col], dstYs[row + 1] - dstYs[row]);

        if (srcR.size.x > 0.0f && srcR.size.y > 0.0f && dstR.size.x > 0.0f && dstR.size.y > 0.0f) {
          Renderer2D::drawTextureRegionScreen(tex, srcR, dstR.position, dstR.size, modulate);
        }
      }
    }
  }
};
