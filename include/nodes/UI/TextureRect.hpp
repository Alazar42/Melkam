#pragma once

#include "nodes/UI/Control.hpp"
#include "renderers/Texture2D.hpp"
#include <memory>

enum class TextureRectStretchMode {
  ScaleOnExpand,
  Keep,
  KeepCentered,
  KeepAspect,
  KeepAspectCentered
};

// UI Texture Display Node (inspired by Godot TextureRect).
class TextureRect : public Control {
public:
  std::shared_ptr<Texture2D> texture = nullptr;
  TextureRectStretchMode stretchMode = TextureRectStretchMode::KeepAspectCentered;
  bool flipH = false;
  bool flipV = false;

  TextureRect() : Control("TextureRect") {
    mouseFilter = MouseFilter::Pass;
  }

  explicit TextureRect(std::shared_ptr<Texture2D> tex)
      : Control("TextureRect"), texture(std::move(tex)) {
    mouseFilter = MouseFilter::Pass;
  }

  void drawControl() override {
    if (!texture || !texture->isValid()) return;

    Rect2 rect = getGlobalRect();
    Vector2 imgSize = texture->getSize();

    Vector2 drawPos = rect.position;
    Vector2 drawSize = rect.size;

    switch (stretchMode) {
    case TextureRectStretchMode::ScaleOnExpand:
      drawSize = rect.size;
      break;
    case TextureRectStretchMode::Keep:
      drawSize = imgSize;
      break;
    case TextureRectStretchMode::KeepCentered:
      drawSize = imgSize;
      drawPos = rect.position + (rect.size - drawSize) * 0.5f;
      break;
    case TextureRectStretchMode::KeepAspect: {
      float scale = std::min(rect.size.x / imgSize.x, rect.size.y / imgSize.y);
      drawSize = imgSize * scale;
      break;
    }
    case TextureRectStretchMode::KeepAspectCentered: {
      float scale = std::min(rect.size.x / imgSize.x, rect.size.y / imgSize.y);
      drawSize = imgSize * scale;
      drawPos = rect.position + (rect.size - drawSize) * 0.5f;
      break;
    }
    }

    Renderer2D::drawTextureScreen(*texture, Rect2(), drawPos, drawSize, modulate);
  }
};
