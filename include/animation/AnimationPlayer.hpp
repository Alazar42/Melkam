#pragma once

#include "animation/Animation.hpp"
#include "core/Memory.hpp"
#include "core/Node.hpp"
#include "core/Signal.hpp"
#include <string>
#include <unordered_map>

// Animation Playback Engine Node (inspired by Godot AnimationPlayer)
class AnimationPlayer : public Node {
public:
  Signal<std::string> animation_started;
  Signal<std::string> animation_finished;
  Signal<std::string> animation_changed;

  float speedScale = 1.0f;
  std::string autoplay = "";

  AnimationPlayer() : Node("AnimationPlayer") {}

  void addAnimation(const std::string &name, Ref<Animation> anim) {
    if (anim) {
      anim->name = name;
      m_animations[name] = std::move(anim);
    }
  }

  Ref<Animation> getAnimation(const std::string &name) const {
    auto it = m_animations.find(name);
    if (it != m_animations.end()) return it->second;
    return nullptr;
  }

  bool hasAnimation(const std::string &name) const {
    return m_animations.find(name) != m_animations.end();
  }

  void play(const std::string &name = "", float customSpeed = 1.0f) {
    std::string toPlay = name.empty() ? m_currentAnimName : name;
    if (toPlay.empty()) return;

    auto it = m_animations.find(toPlay);
    if (it == m_animations.end() || !it->second) return;

    if (m_currentAnimName != toPlay) {
      m_currentAnimName = toPlay;
      m_currentAnim = it->second;
      m_currentTime = 0.0f;
      m_pingPongForward = true;
      animation_changed.emit(m_currentAnimName);
    }

    m_customSpeed = customSpeed;
    m_playing = true;
    animation_started.emit(m_currentAnimName);
    applyCurrentState();
  }

  void pause() {
    m_playing = false;
  }

  void stop() {
    m_playing = false;
    m_currentTime = 0.0f;
    applyCurrentState();
  }

  void seek(float time, bool update = true) {
    if (!m_currentAnim) return;
    m_currentTime = std::clamp(time, 0.0f, m_currentAnim->length);
    if (update) {
      applyCurrentState();
    }
  }

  bool isPlaying() const { return m_playing; }
  const std::string &getCurrentAnimation() const { return m_currentAnimName; }
  float getCurrentAnimationPosition() const { return m_currentTime; }
  float getCurrentAnimationLength() const { return m_currentAnim ? m_currentAnim->length : 0.0f; }

  void onReady() override {
    if (!autoplay.empty() && hasAnimation(autoplay)) {
      play(autoplay);
    }
  }

  void onProcess(float delta) override {
    if (!m_playing || !m_currentAnim || m_currentAnim->length <= 0.0f) return;

    float stepDelta = delta * speedScale * m_customSpeed;
    float len = m_currentAnim->length;

    switch (m_currentAnim->loopMode) {
    case AnimationLoopMode::None: {
      m_currentTime += stepDelta;
      if (m_currentTime >= len) {
        m_currentTime = len;
        m_playing = false;
        applyCurrentState();
        animation_finished.emit(m_currentAnimName);
        return;
      }
      break;
    }
    case AnimationLoopMode::Linear: {
      m_currentTime += stepDelta;
      if (m_currentTime >= len) {
        m_currentTime = std::fmod(m_currentTime, len);
        animation_finished.emit(m_currentAnimName);
      }
      break;
    }
    case AnimationLoopMode::PingPong: {
      if (m_pingPongForward) {
        m_currentTime += stepDelta;
        if (m_currentTime >= len) {
          m_currentTime = len;
          m_pingPongForward = false;
        }
      } else {
        m_currentTime -= stepDelta;
        if (m_currentTime <= 0.0f) {
          m_currentTime = 0.0f;
          m_pingPongForward = true;
          animation_finished.emit(m_currentAnimName);
        }
      }
      break;
    }
    }

    applyCurrentState();
  }

private:
  void applyCurrentState() {
    if (!m_currentAnim) return;

    Node *parent = getParent();
    if (!parent) parent = this;

    for (const auto &track : m_currentAnim->tracks) {
      Node *targetNode = parent;
      if (!track.nodePath.empty()) {
        targetNode = parent->findChild(track.nodePath).get();
      }
      if (targetNode) {
        track.apply(m_currentTime, targetNode);
      }
    }
  }

  std::unordered_map<std::string, Ref<Animation>> m_animations;
  Ref<Animation> m_currentAnim = nullptr;
  std::string m_currentAnimName = "";
  float m_currentTime = 0.0f;
  float m_customSpeed = 1.0f;
  bool m_playing = false;
  bool m_pingPongForward = true;
};
