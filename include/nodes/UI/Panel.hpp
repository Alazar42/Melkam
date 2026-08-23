#pragma once

#include "nodes/UI/Control.hpp"
#include "renderers/Texture2D.hpp"
#include <algorithm>
#include <memory>

// Styled Background Panel Box Node (inspired by Godot Panel / PanelContainer & StyleBoxTexture).
class Panel : public Control {
public:
  std::shared_ptr<Texture2D> texture = nullptr;
  float patchMarginLeft = 0.0f;
  float patchMarginTop = 0.0f;
  float patchMarginRight = 0.0f;
  float patchMarginBottom = 0.0f;
  bool drawCenter = true;

  Color backgroundColor = Color(0.0f, 0.0f, 0.0f, 0.0f); // Default transparent sentinel (uses active theme)
  Color borderColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
  float borderWidth = -1.0f;
  float cornerRadius = -1.0f;

  Panel() : Control("Panel") {
    mouseFilter = MouseFilter::Pass;
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    std::shared_ptr<Texture2D> activeTex = texture ? texture : getThemeTexture("texture", "Panel");

    if (activeTex && activeTex->isValid()) {
      if (patchMarginLeft > 0.0f || patchMarginTop > 0.0f ||
          patchMarginRight > 0.0f || patchMarginBottom > 0.0f) {
        drawNinePatch(activeTex.get(), rect);
      } else {
        Renderer2D::drawTextureScreen(activeTex.get(), rect.position, rect.size, modulate);
      }
      return;
    }

    Color bg = (backgroundColor.a > 0.0f)
                   ? backgroundColor
                   : getThemeColor("bg_color", "Panel", Color::from_rgba8(25, 28, 38, 230));
    Color border = (borderColor.a > 0.0f)
                       ? borderColor
                       : getThemeColor("border_color", "Panel", Color::from_rgba8(65, 75, 105));
    float cr = (cornerRadius >= 0.0f)
                   ? cornerRadius
                   : static_cast<float>(getThemeConstant("corner_radius", "Panel", 6));
    float bw = (borderWidth >= 0.0f)
                   ? borderWidth
                   : static_cast<float>(getThemeConstant("border_width", "Panel", 1));

    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, cr,
                                      bg * modulate, border * modulate,
                                      bw);
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
        if (row == 1 && col == 1 && !drawCenter) continue;

        Rect2 srcR(srcXs[col], srcYs[row], srcXs[col + 1] - srcXs[col], srcYs[row + 1] - srcYs[row]);
        Rect2 dstR(dstXs[col], dstYs[row], dstXs[col + 1] - dstXs[col], dstYs[row + 1] - dstYs[row]);

        if (srcR.size.x > 0.0f && srcR.size.y > 0.0f && dstR.size.x > 0.0f && dstR.size.y > 0.0f) {
          Renderer2D::drawTextureRegionScreen(tex, srcR, dstR.position, dstR.size, modulate);
        }
      }
    }
  }
};

// Flat Solid Color Rectangle UI Node (inspired by Godot ColorRect).
class ColorRect : public Control {
public:
  Color color = Color::WHITE;

  ColorRect() : Control("ColorRect") {
    mouseFilter = MouseFilter::Pass;
  }

  explicit ColorRect(const Color &rectColor)
      : Control("ColorRect"), color(rectColor) {
    mouseFilter = MouseFilter::Pass;
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    Renderer2D::drawRectScreen(rect.position, rect.size, color * modulate, true);
  }
};
