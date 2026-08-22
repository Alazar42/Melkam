#pragma once

#include "core/Signal.hpp"
#include "helper/Rect2.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/SpriteFrames.hpp"
#include "renderers/Renderer2D.hpp"
#include <algorithm>
#include <memory>
#include <string>

// 2D Animated Sprite Node (inspired by Godot AnimatedSprite2D) playing frame animations from SpriteFrames.
class AnimatedSprite2D : public Node2D {
public:
  // Signals
  Signal<> animation_finished;
  Signal<> animation_changed;
  Signal<> frame_changed;
  Signal<> animation_looped;

  // Animation resource
  std::shared_ptr<SpriteFrames> spriteFrames = nullptr;

  // Playback Properties
  std::string animation = "default";
  int frame = 0;
  float speedScale = 1.0f;
  bool autoplay = false;

  // Visual Properties
  Color tint = Color::WHITE;
  Vector2 offset{0.0f, 0.0f};
  bool centered = true;
  bool flipH = false;
  bool flipV = false;

  AnimatedSprite2D() : Node2D("AnimatedSprite2D") {}

  explicit AnimatedSprite2D(std::shared_ptr<SpriteFrames> frames,
                            std::string defaultAnim = "default")
      : Node2D("AnimatedSprite2D"), spriteFrames(std::move(frames)),
        animation(std::move(defaultAnim)) {}

  void onReady() override {
    if (autoplay && !animation.empty()) {
      play(animation);
    }
  }

  // Starts playing an animation track.
  void play(const std::string &animName = "", float customScale = 1.0f) {
    if (!animName.empty() && animName != animation) {
      animation = animName;
      frame = 0;
      m_frameTimer = 0.0f;
      animation_changed.emit();
    }
    speedScale = customScale;
    m_isPlaying = true;
  }

  // Pauses animation playback at the current frame.
  void pause() {
    m_isPlaying = false;
  }

  // Stops animation playback and resets to frame 0.
  void stop() {
    m_isPlaying = false;
    frame = 0;
    m_frameTimer = 0.0f;
  }

  // Returns true if animation is actively playing.
  bool isPlaying() const { return m_isPlaying; }

  // Sets active animation track name.
  void setAnimation(const std::string &animName) {
    if (animation != animName) {
      animation = animName;
      frame = 0;
      m_frameTimer = 0.0f;
      animation_changed.emit();
    }
  }

  // Sets current frame index.
  void setFrame(int frameIdx) {
    if (!spriteFrames) return;
    int count = spriteFrames->getFrameCount(animation);
    if (count > 0) {
      int newFrame = std::clamp(frameIdx, 0, count - 1);
      if (newFrame != frame) {
        frame = newFrame;
        frame_changed.emit();
      }
    }
  }

  // Returns effective render size in world coordinates.
  Vector2 getEffectiveSize() const {
    if (!spriteFrames) return {0.0f, 0.0f};
    const SpriteFrame *sf = spriteFrames->getFrame(animation, frame);
    if (!sf || !sf->texture || !sf->texture->isValid()) return {0.0f, 0.0f};

    if (sf->region.hasArea()) {
      return sf->region.size;
    }
    return sf->texture->getSize();
  }

  // Updates animation frame progression.
  void onProcess(float delta) override {
    if (!m_isPlaying || !spriteFrames) return;

    int totalFrames = spriteFrames->getFrameCount(animation);
    if (totalFrames <= 0) return;

    float fps = spriteFrames->getAnimationSpeed(animation);
    if (fps <= 0.0f) return;

    const SpriteFrame *sf = spriteFrames->getFrame(animation, frame);
    float frameDuration = (sf ? sf->duration : 1.0f) / (fps * std::abs(speedScale));

    m_frameTimer += delta;
    if (m_frameTimer >= frameDuration) {
      m_frameTimer -= frameDuration;
      int nextFrame = frame + (speedScale >= 0.0f ? 1 : -1);

      if (nextFrame >= totalFrames) {
        if (spriteFrames->getAnimationLoop(animation)) {
          frame = 0;
          animation_looped.emit();
          frame_changed.emit();
        } else {
          frame = totalFrames - 1;
          m_isPlaying = false;
          animation_finished.emit();
        }
      } else if (nextFrame < 0) {
        if (spriteFrames->getAnimationLoop(animation)) {
          frame = totalFrames - 1;
          animation_looped.emit();
          frame_changed.emit();
        } else {
          frame = 0;
          m_isPlaying = false;
          animation_finished.emit();
        }
      } else {
        frame = nextFrame;
        frame_changed.emit();
      }
    }
  }

  // Renders active frame to the 2D canvas.
  void onDraw() override {
    if (!visible || !isGlobalVisible() || !spriteFrames) return;

    const SpriteFrame *sf = spriteFrames->getFrame(animation, frame);
    if (!sf || !sf->texture || !sf->texture->isValid()) return;

    Transform2D global = getGlobalTransform();
    Vector2 effSize = getEffectiveSize();
    Vector2 scaledSize = Vector2(effSize.x * global.scale.x, effSize.y * global.scale.y);

    Vector2 drawPos = global.position + offset;
    if (centered) {
      drawPos -= scaledSize * 0.5f;
    }

    Rect2 srcRect = sf->region.hasArea() ? sf->region : sf->texture->getRect();
    Renderer2D::drawTextureRegion(*sf->texture, srcRect, drawPos, scaledSize, tint,
                                  global.rotation, flipH, flipV);
  }

private:
  bool m_isPlaying = false;
  float m_frameTimer = 0.0f;
};
