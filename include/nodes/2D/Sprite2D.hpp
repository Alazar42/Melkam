#pragma once

#include "helper/Rect2.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include <algorithm>
#include <memory>
#include <string>

// 2D Sprite Node (inspired by Godot Sprite2D) rendering a 2D texture or sprite sheet region.
class Sprite2D : public Node2D {
public:
  std::shared_ptr<Texture2D> texture = nullptr;
  Color tint = Color::WHITE;
  Vector2 size{0.0f, 0.0f};       // If {0,0}, uses natural texture/frame size
  Vector2 offset{0.0f, 0.0f};     // Offset in pixels
  bool centered = true;           // If true, sprite center is positioned at node transform
  bool flipH = false;             // Flip horizontally
  bool flipV = false;             // Flip vertically

  // Sprite Sheet Animation / Frame Grid
  int hframes = 1;                // Number of columns in the sprite sheet
  int vframes = 1;                // Number of rows in the sprite sheet
  int frame = 0;                  // Current active linear frame index [0 .. hframes*vframes - 1]

  // Region / Atlas Clipping
  bool regionEnabled = false;     // If true, only renders regionRect
  Rect2 regionRect{};             // Source rectangle within the texture

  Sprite2D() : Node2D("Sprite2D") {}

  explicit Sprite2D(std::shared_ptr<Texture2D> tex, const Color &colorTint = Color::WHITE)
      : Node2D("Sprite2D"), texture(std::move(tex)), tint(colorTint) {}

  explicit Sprite2D(const std::string &imagePath, const Color &colorTint = Color::WHITE)
      : Node2D("Sprite2D"), tint(colorTint) {
    texture = std::make_shared<Texture2D>(imagePath);
  }

  // Sets frame coordinates as (column, row)
  void setFrameCoords(int col, int row) {
    if (hframes > 0) {
      frame = std::clamp(row * hframes + col, 0, std::max(0, hframes * vframes - 1));
    }
  }

  // Returns current frame coordinates as Vector2(column, row)
  Vector2 getFrameCoords() const {
    if (hframes <= 0) return {0.0f, 0.0f};
    int col = frame % hframes;
    int row = frame / hframes;
    return {static_cast<float>(col), static_cast<float>(row)};
  }

  // Computes the active source rectangle from the texture
  Rect2 getSrcRect() const {
    if (!texture || !texture->isValid()) return Rect2();

    if (regionEnabled && regionRect.hasArea()) {
      return regionRect;
    }

    if (hframes > 1 || vframes > 1) {
      float frameW = static_cast<float>(texture->getWidth()) / static_cast<float>(std::max(1, hframes));
      float frameH = static_cast<float>(texture->getHeight()) / static_cast<float>(std::max(1, vframes));
      int maxFrames = std::max(1, hframes * vframes);
      int clFrame = std::clamp(frame, 0, maxFrames - 1);
      int col = clFrame % hframes;
      int row = clFrame / hframes;
      return Rect2(col * frameW, row * frameH, frameW, frameH);
    }

    return texture->getRect();
  }

  // Returns effective render size in world coordinates
  Vector2 getEffectiveSize() const {
    if (size.x > 0.0f && size.y > 0.0f) {
      return size;
    }
    Rect2 src = getSrcRect();
    return src.hasArea() ? src.size : (texture ? texture->getSize() : Vector2(0.0f, 0.0f));
  }

  // Renders the sprite at an explicit world position, rotation, and scale.
  void draw(const Vector2 &pos, float rot = 0.0f,
            const Vector2 &s = {1.0f, 1.0f}) const {
    if (!visible || !isGlobalVisible() || !texture || !texture->isValid()) return;

    Vector2 effSize = getEffectiveSize();
    Vector2 scaledSize = Vector2(effSize.x * s.x, effSize.y * s.y);

    Vector2 drawPos = pos + offset;
    if (centered) {
      drawPos -= scaledSize * 0.5f;
    }

    Rect2 srcRect = getSrcRect();
    Renderer2D::drawTextureRegion(*texture, srcRect, drawPos, scaledSize, tint,
                                  rot, flipH, flipV);
  }

  // Renders the sprite to the active 2D pass
  void onDraw() override {
    Transform2D global = getGlobalTransform();
    draw(global.position, global.rotation, global.scale);
  }
};
