#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "renderers/Texture2D.hpp"
#include <memory>

// 9-Slice Scalable Texture Panel (inspired by Godot NinePatchRect).
class NinePatchRect : public Control {
public:
  Ref<Texture2D> texture = nullptr;
  int patchMarginLeft = 0;
  int patchMarginTop = 0;
  int patchMarginRight = 0;
  int patchMarginBottom = 0;
  Rect2 regionRect;
  bool drawCenter = true;

  NinePatchRect() : Control("NinePatchRect") {
    mouseFilter = MouseFilter::Pass;
  }

  explicit NinePatchRect(Ref<Texture2D> tex, int margin = 0)
      : Control("NinePatchRect"), texture(std::move(tex)) {
    mouseFilter = MouseFilter::Pass;
    setAllMargins(margin);
  }

  void setAllMargins(int margin) {
    patchMarginLeft = margin;
    patchMarginTop = margin;
    patchMarginRight = margin;
    patchMarginBottom = margin;
  }

  void drawControl() override {
    if (!texture || !texture->isValid()) return;

    Rect2 dst = getGlobalRect();
    Vector2 srcSize = regionRect.hasArea() ? regionRect.size : texture->getSize();
    Vector2 srcOffset = regionRect.hasArea() ? regionRect.position : Vector2(0.0f, 0.0f);

    float ml = static_cast<float>(patchMarginLeft);
    float mt = static_cast<float>(patchMarginTop);
    float mr = static_cast<float>(patchMarginRight);
    float mb = static_cast<float>(patchMarginBottom);

    // Source grid coordinates
    float srcX[4] = {srcOffset.x, srcOffset.x + ml, srcOffset.x + srcSize.x - mr, srcOffset.x + srcSize.x};
    float srcY[4] = {srcOffset.y, srcOffset.y + mt, srcOffset.y + srcSize.y - mb, srcOffset.y + srcSize.y};

    // Destination grid coordinates
    float dstX[4] = {dst.position.x, dst.position.x + ml, dst.position.x + dst.size.x - mr, dst.position.x + dst.size.x};
    float dstY[4] = {dst.position.y, dst.position.y + mt, dst.position.y + dst.size.y - mb, dst.position.y + dst.size.y};

    // Render 3x3 9-patch grid
    for (int row = 0; row < 3; ++row) {
      for (int col = 0; col < 3; ++col) {
        if (row == 1 && col == 1 && !drawCenter) continue;

        Rect2 patchSrc(srcX[col], srcY[row], srcX[col + 1] - srcX[col], srcY[row + 1] - srcY[row]);
        Vector2 patchDstPos(dstX[col], dstY[row]);
        Vector2 patchDstSize(dstX[col + 1] - dstX[col], dstY[row + 1] - dstY[row]);

        if (patchSrc.size.x > 0 && patchSrc.size.y > 0 && patchDstSize.x > 0 && patchDstSize.y > 0) {
          Renderer2D::drawTextureScreen(*texture, patchSrc, patchDstPos, patchDstSize, modulate);
        }
      }
    }
  }
};

