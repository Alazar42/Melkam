#pragma once

#include "nodes/UI/CheckBox.hpp"
#include "renderers/Texture2D.hpp"
#include <memory>

// Texture-Driven Push Button Node (inspired by Godot TextureButton).
class TextureButton : public BaseButton {
public:
  std::shared_ptr<Texture2D> textureNormal = nullptr;
  std::shared_ptr<Texture2D> texturePressed = nullptr;
  std::shared_ptr<Texture2D> textureHover = nullptr;
  std::shared_ptr<Texture2D> textureDisabled = nullptr;
  std::shared_ptr<Texture2D> textureFocused = nullptr;

  bool ignoreTextureSize = false;
  bool flipH = false;
  bool flipV = false;

  TextureButton() : BaseButton("TextureButton") {}

  explicit TextureButton(std::shared_ptr<Texture2D> normalTex)
      : BaseButton("TextureButton"), textureNormal(std::move(normalTex)) {
    if (textureNormal && textureNormal->isValid() && !ignoreTextureSize) {
      customMinimumSize = textureNormal->getSize();
    }
  }

  void drawControl() override {
    std::shared_ptr<Texture2D> activeTex = textureNormal;

    if (disabled && textureDisabled) {
      activeTex = textureDisabled;
    } else if (m_isDown && m_isHovered && texturePressed) {
      activeTex = texturePressed;
    } else if (m_isHovered && textureHover) {
      activeTex = textureHover;
    }

    if (activeTex && activeTex->isValid()) {
      Rect2 rect = getGlobalRect();
      Vector2 drawSize = ignoreTextureSize ? rect.size : activeTex->getSize();
      Renderer2D::drawTextureScreen(*activeTex, Rect2(), rect.position, drawSize, modulate);
    }
  }
};
