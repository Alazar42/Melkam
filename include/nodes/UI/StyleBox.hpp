#pragma once

#include "core/Memory.hpp"
#include "helper/Rect2.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include <algorithm>
#include <cmath>
#include <string>

// =============================================================================
// MelkamEngine StyleBox System (inspired by Godot StyleBox Architecture)
// =============================================================================

// Base Abstract StyleBox Class for customizable UI skinning and backgrounds
class StyleBox {
public:
  virtual ~StyleBox() = default;

  float contentMarginLeft = -1.0f;
  float contentMarginTop = -1.0f;
  float contentMarginRight = -1.0f;
  float contentMarginBottom = -1.0f;

  void setContentMarginAll(float margin) {
    contentMarginLeft = margin;
    contentMarginTop = margin;
    contentMarginRight = margin;
    contentMarginBottom = margin;
  }

  virtual void draw(const Rect2 &rect, const Color &modulate = Color::WHITE) const = 0;
};

// Flat Color / Border / Rounded Box Style (inspired by Godot StyleBoxFlat)
class StyleBoxFlat : public StyleBox {
public:
  Color backgroundColor = Color::from_rgba8(35, 40, 52);
  Color borderColor = Color::from_rgba8(75, 85, 110);
  
  float borderWidthLeft = 1.0f;
  float borderWidthTop = 1.0f;
  float borderWidthRight = 1.0f;
  float borderWidthBottom = 1.0f;

  float cornerRadiusTopLeft = 4.0f;
  float cornerRadiusTopRight = 4.0f;
  float cornerRadiusBottomRight = 4.0f;
  float cornerRadiusBottomLeft = 4.0f;

  Color shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
  Vector2 shadowOffset{0.0f, 2.0f};
  float shadowSize = 0.0f;

  bool drawCenter = true;

  StyleBoxFlat() = default;
  explicit StyleBoxFlat(const Color &bg, float radius = 4.0f, float borderW = 1.0f, const Color &border = Color(0,0,0,0))
      : backgroundColor(bg) {
    setCornerRadiusAll(radius);
    setBorderWidthAll(borderW);
    if (border.a > 0.0f) borderColor = border;
  }

  void setBorderWidthAll(float width) {
    borderWidthLeft = width;
    borderWidthTop = width;
    borderWidthRight = width;
    borderWidthBottom = width;
  }

  void setCornerRadiusAll(float radius) {
    cornerRadiusTopLeft = radius;
    cornerRadiusTopRight = radius;
    cornerRadiusBottomRight = radius;
    cornerRadiusBottomLeft = radius;
  }

  void draw(const Rect2 &rect, const Color &mod = Color::WHITE) const override {
    // 1. Draw Shadow if configured
    if (shadowColor.a > 0.0f && (shadowSize > 0.0f || shadowOffset.length_squared() > 0.0f)) {
      Rect2 shadowRect(rect.position.x + shadowOffset.x - shadowSize,
                       rect.position.y + shadowOffset.y - shadowSize,
                       rect.size.x + shadowSize * 2.0f,
                       rect.size.y + shadowSize * 2.0f);
      float avgRadius = (cornerRadiusTopLeft + cornerRadiusTopRight + cornerRadiusBottomRight + cornerRadiusBottomLeft) * 0.25f;
      Renderer2D::drawRoundedRectScreen(shadowRect.position, shadowRect.size, avgRadius,
                                        shadowColor * mod);
    }

    // 2. Draw Main Rounded Rectangle with Border
    float maxBorder = std::max({borderWidthLeft, borderWidthTop, borderWidthRight, borderWidthBottom});
    float avgRadius = (cornerRadiusTopLeft + cornerRadiusTopRight + cornerRadiusBottomRight + cornerRadiusBottomLeft) * 0.25f;

    Color fill = drawCenter ? (backgroundColor * mod) : Color(0.0f, 0.0f, 0.0f, 0.0f);
    Color border = (maxBorder > 0.0f) ? (borderColor * mod) : Color(0.0f, 0.0f, 0.0f, 0.0f);

    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, avgRadius, fill, border, maxBorder);
  }
};

// 9-Slice Textured GUI Skin Style (inspired by Godot StyleBoxTexture)
class StyleBoxTexture : public StyleBox {
public:
  Ref<Texture2D> texture = nullptr;
  float patchMarginLeft = 0.0f;
  float patchMarginTop = 0.0f;
  float patchMarginRight = 0.0f;
  float patchMarginBottom = 0.0f;

  float expandMarginLeft = 0.0f;
  float expandMarginTop = 0.0f;
  float expandMarginRight = 0.0f;
  float expandMarginBottom = 0.0f;

  Color modulateColor = Color::WHITE;
  bool drawCenter = true;

  StyleBoxTexture() = default;
  explicit StyleBoxTexture(Ref<Texture2D> tex, float margin = 0.0f)
      : texture(std::move(tex)) {
    setPatchMarginAll(margin);
  }

  void setPatchMarginAll(float margin) {
    patchMarginLeft = margin;
    patchMarginTop = margin;
    patchMarginRight = margin;
    patchMarginBottom = margin;
  }

  void setExpandMarginAll(float margin) {
    expandMarginLeft = margin;
    expandMarginTop = margin;
    expandMarginRight = margin;
    expandMarginBottom = margin;
  }

  void draw(const Rect2 &rect, const Color &mod = Color::WHITE) const override {
    if (!texture || !texture->isValid()) return;

    Rect2 drawRect(rect.position.x - expandMarginLeft,
                   rect.position.y - expandMarginTop,
                   rect.size.x + expandMarginLeft + expandMarginRight,
                   rect.size.y + expandMarginTop + expandMarginBottom);

    Color finalTint = modulateColor * mod;

    if (patchMarginLeft <= 0.0f && patchMarginTop <= 0.0f &&
        patchMarginRight <= 0.0f && patchMarginBottom <= 0.0f) {
      Renderer2D::drawTextureScreen(texture.get(), drawRect.position, drawRect.size, finalTint);
      return;
    }

    float tw = static_cast<float>(texture->getWidth());
    float th = static_cast<float>(texture->getHeight());

    float ml = std::clamp(patchMarginLeft, 0.0f, tw * 0.5f);
    float mr = std::clamp(patchMarginRight, 0.0f, tw * 0.5f);
    float mt = std::clamp(patchMarginTop, 0.0f, th * 0.5f);
    float mb = std::clamp(patchMarginBottom, 0.0f, th * 0.5f);

    float srcXs[4] = {0.0f, ml, tw - mr, tw};
    float srcYs[4] = {0.0f, mt, th - mb, th};

    float dstXs[4] = {drawRect.position.x, drawRect.position.x + ml,
                      drawRect.position.x + drawRect.size.x - mr, drawRect.position.x + drawRect.size.x};
    float dstYs[4] = {drawRect.position.y, drawRect.position.y + mt,
                      drawRect.position.y + drawRect.size.y - mb, drawRect.position.y + drawRect.size.y};

    for (int row = 0; row < 3; ++row) {
      for (int col = 0; col < 3; ++col) {
        if (row == 1 && col == 1 && !drawCenter) continue;

        Rect2 srcR(srcXs[col], srcYs[row], srcXs[col + 1] - srcXs[col], srcYs[row + 1] - srcYs[row]);
        Rect2 dstR(dstXs[col], dstYs[row], dstXs[col + 1] - dstXs[col], dstYs[row + 1] - dstYs[row]);

        if (srcR.size.x > 0.0f && srcR.size.y > 0.0f && dstR.size.x > 0.0f && dstR.size.y > 0.0f) {
          Renderer2D::drawTextureRegionScreen(texture.get(), srcR, dstR.position, dstR.size, finalTint);
        }
      }
    }
  }
};

// Line Style for Separators (inspired by Godot StyleBoxLine)
class StyleBoxLine : public StyleBox {
public:
  Color color = Color::from_rgba8(80, 85, 110);
  float thickness = 1.0f;
  bool vertical = false;

  StyleBoxLine() = default;
  explicit StyleBoxLine(const Color &c, float thick = 1.0f, bool isVertical = false)
      : color(c), thickness(thick), vertical(isVertical) {}

  void draw(const Rect2 &rect, const Color &mod = Color::WHITE) const override {
    if (vertical) {
      float x = rect.position.x + (rect.size.x - thickness) * 0.5f;
      Renderer2D::drawRectScreen(Vector2(x, rect.position.y), Vector2(thickness, rect.size.y),
                                color * mod, true);
    } else {
      float y = rect.position.y + (rect.size.y - thickness) * 0.5f;
      Renderer2D::drawRectScreen(Vector2(rect.position.x, y), Vector2(rect.size.x, thickness),
                                color * mod, true);
    }
  }
};

// Empty / Transparent Padding Style (inspired by Godot StyleBoxEmpty)
class StyleBoxEmpty : public StyleBox {
public:
  void draw(const Rect2 &, const Color &) const override {}
};
