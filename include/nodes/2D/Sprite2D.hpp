#pragma once

#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include <memory>
#include <string>

// 2D Sprite Node (inspired by Godot Sprite2D) rendering a Texture2D on the canvas.
class Sprite2D : public Node2D {
public:
  std::shared_ptr<Texture2D> texture = nullptr;
  Color tint = Color::WHITE;
  Vector2 size{0.0f, 0.0f};       // If {0,0}, uses natural texture size
  Vector2 origin{0.5f, 0.5f};     // Pivot point (0.5, 0.5 = center, 0, 0 = top-left)
  bool flipH = false;             // Flip horizontally
  bool flipV = false;             // Flip vertically

  Sprite2D() : Node2D("Sprite2D") {}

  explicit Sprite2D(std::shared_ptr<Texture2D> tex, const Color &colorTint = Color::WHITE)
      : Node2D("Sprite2D"), texture(std::move(tex)), tint(colorTint) {
    if (texture) {
      size = texture->getSize();
    }
  }

  // Returns effective render size of the sprite.
  Vector2 getEffectiveSize() const {
    if (size.x > 0.0f && size.y > 0.0f) {
      return size;
    }
    return texture ? texture->getSize() : Vector2(0.0f, 0.0f);
  }

  // Renders the sprite at an explicit world position, rotation, and scale.
  void draw(const Vector2 &position, float rot = 0.0f,
            const Vector2 &scale = {1.0f, 1.0f}) const {
    if (!visible || !texture || !texture->isValid()) return;

    Vector2 effSize = getEffectiveSize();
    Vector2 scaledSize = Vector2(effSize.x * scale.x, effSize.y * scale.y);
    Vector2 drawPos = position - Vector2(scaledSize.x * origin.x, scaledSize.y * origin.y);

    Renderer2D::drawTexture(*texture, drawPos, scaledSize, tint, rot, flipH, flipV);
  }

  // Renders the sprite to the active 2D renderer pass using its global transform.
  void onDraw() override {
    Transform2D global = getGlobalTransform();
    draw(global.position, global.rotation, global.scale);
  }
};
