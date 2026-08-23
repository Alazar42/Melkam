#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Range.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include <algorithm>
#include <memory>

enum class TextureFillMode {
  LeftToRight = 0,
  RightToLeft = 1,
  TopToBottom = 2,
  BottomToTop = 3,
  BilateralHorizontal = 4,
  BilateralVertical = 5
};

// Textured Progress Bar UI Node (inspired by Godot TextureProgressBar)
class TextureProgressBar : public Range {
public:
  Ref<Texture2D> textureUnder = nullptr;
  Ref<Texture2D> textureProgress = nullptr;
  Ref<Texture2D> textureOver = nullptr;

  TextureFillMode fillMode = TextureFillMode::LeftToRight;
  Color tintUnder = Color::WHITE;
  Color tintProgress = Color::WHITE;
  Color tintOver = Color::WHITE;

  float patchMarginLeft = 0.0f;
  float patchMarginTop = 0.0f;
  float patchMarginRight = 0.0f;
  float patchMarginBottom = 0.0f;
  bool ninePatchStretch = false;

  TextureProgressBar() : Range("TextureProgressBar") {
    customMinimumSize = {120.0f, 24.0f};
    mouseFilter = MouseFilter::Ignore;
    value = 100.0f;
    step = 0.01f;
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    float ratio = getRatio();

    // 1. Draw Texture Under
    if (textureUnder && textureUnder->isValid()) {
      drawTextureLayer(textureUnder.get(), rect, tintUnder * modulate, 1.0f, TextureFillMode::LeftToRight);
    }

    // 2. Draw Texture Progress
    if (textureProgress && textureProgress->isValid() && ratio > 0.0f) {
      drawTextureLayer(textureProgress.get(), rect, tintProgress * modulate, ratio, fillMode);
    }

    // 3. Draw Texture Over
    if (textureOver && textureOver->isValid()) {
      drawTextureLayer(textureOver.get(), rect, tintOver * modulate, 1.0f, TextureFillMode::LeftToRight);
    }
  }

private:
  void drawTextureLayer(Texture2D *tex, const Rect2 &rect, const Color &tint, float ratio, TextureFillMode mode) {
    float tw = static_cast<float>(tex->getWidth());
    float th = static_cast<float>(tex->getHeight());

    Rect2 srcRect(0.0f, 0.0f, tw, th);
    Rect2 dstRect = rect;

    if (mode == TextureFillMode::LeftToRight) {
      srcRect.size.x = tw * ratio;
      dstRect.size.x = rect.size.x * ratio;
    } else if (mode == TextureFillMode::RightToLeft) {
      float progressW = tw * ratio;
      srcRect.position.x = tw - progressW;
      srcRect.size.x = progressW;
      float dstW = rect.size.x * ratio;
      dstRect.position.x = rect.position.x + rect.size.x - dstW;
      dstRect.size.x = dstW;
    } else if (mode == TextureFillMode::TopToBottom) {
      srcRect.size.y = th * ratio;
      dstRect.size.y = rect.size.y * ratio;
    } else if (mode == TextureFillMode::BottomToTop) {
      float progressH = th * ratio;
      srcRect.position.y = th - progressH;
      srcRect.size.y = progressH;
      float dstH = rect.size.y * ratio;
      dstRect.position.y = rect.position.y + rect.size.y - dstH;
      dstRect.size.y = dstH;
    }

    if (srcRect.size.x > 0.0f && srcRect.size.y > 0.0f && dstRect.size.x > 0.0f && dstRect.size.y > 0.0f) {
      Renderer2D::drawTextureRegionScreen(tex, srcRect, dstRect.position, dstRect.size, tint);
    }
  }
};
