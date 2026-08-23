#pragma once

#include "animation/Tween.hpp"
#include "core/Memory.hpp"
#include "core/Node.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/Sprite2D.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <vector>

enum class AnimationLoopMode {
  None,
  Linear,
  PingPong
};

enum class AnimationTrackType {
  Position,
  Rotation,
  Scale,
  Color,
  Frame,
  Visible
};

template <typename T>
struct AnimationKeyframe {
  float time = 0.0f;
  T value{};
  TransitionType transition = TransitionType::Linear;
  EaseType ease = EaseType::InOut;
};

// Keyframe Animation Track
class AnimationTrack {
public:
  std::string nodePath;
  AnimationTrackType type = AnimationTrackType::Position;

  std::vector<AnimationKeyframe<Vector2>> vec2Keys;
  std::vector<AnimationKeyframe<float>> floatKeys;
  std::vector<AnimationKeyframe<Color>> colorKeys;
  std::vector<AnimationKeyframe<int>> intKeys;
  std::vector<AnimationKeyframe<bool>> boolKeys;

  void addKeyframe(float time, const Vector2 &val, TransitionType trans = TransitionType::Linear, EaseType ease = EaseType::InOut) {
    vec2Keys.push_back({time, val, trans, ease});
    std::sort(vec2Keys.begin(), vec2Keys.end(), [](const auto &a, const auto &b) { return a.time < b.time; });
  }

  void addKeyframe(float time, float val, TransitionType trans = TransitionType::Linear, EaseType ease = EaseType::InOut) {
    floatKeys.push_back({time, val, trans, ease});
    std::sort(floatKeys.begin(), floatKeys.end(), [](const auto &a, const auto &b) { return a.time < b.time; });
  }

  void addKeyframe(float time, const Color &val, TransitionType trans = TransitionType::Linear, EaseType ease = EaseType::InOut) {
    colorKeys.push_back({time, val, trans, ease});
    std::sort(colorKeys.begin(), colorKeys.end(), [](const auto &a, const auto &b) { return a.time < b.time; });
  }

  void addKeyframe(float time, int val) {
    intKeys.push_back({time, val, TransitionType::Linear, EaseType::InOut});
    std::sort(intKeys.begin(), intKeys.end(), [](const auto &a, const auto &b) { return a.time < b.time; });
  }

  void addKeyframe(float time, bool val) {
    boolKeys.push_back({time, val, TransitionType::Linear, EaseType::InOut});
    std::sort(boolKeys.begin(), boolKeys.end(), [](const auto &a, const auto &b) { return a.time < b.time; });
  }

  void apply(float time, Node *targetNode) const {
    if (!targetNode) return;

    switch (type) {
    case AnimationTrackType::Position:
      if (auto *n2d = dynamic_cast<Node2D *>(targetNode)) {
        n2d->setPosition(sampleVec2(vec2Keys, time));
      }
      break;
    case AnimationTrackType::Rotation:
      if (auto *n2d = dynamic_cast<Node2D *>(targetNode)) {
        n2d->setRotation(sampleFloat(floatKeys, time));
      }
      break;
    case AnimationTrackType::Scale:
      if (auto *n2d = dynamic_cast<Node2D *>(targetNode)) {
        n2d->setScale(sampleVec2(vec2Keys, time));
      }
      break;
    case AnimationTrackType::Frame:
      if (auto *sprite = dynamic_cast<Sprite2D *>(targetNode)) {
        sprite->frame = sampleInt(intKeys, time);
      }
      break;
    case AnimationTrackType::Visible:
      targetNode->visible = sampleBool(boolKeys, time);
      break;
    case AnimationTrackType::Color:
      if (auto *sprite = dynamic_cast<Sprite2D *>(targetNode)) {
        sprite->tint = sampleColor(colorKeys, time);
      }
      break;
    }
  }

private:
  static Vector2 sampleVec2(const std::vector<AnimationKeyframe<Vector2>> &keys, float time) {
    if (keys.empty()) return {0.0f, 0.0f};
    if (time <= keys.front().time) return keys.front().value;
    if (time >= keys.back().time) return keys.back().value;

    for (size_t i = 0; i + 1 < keys.size(); ++i) {
      if (time >= keys[i].time && time <= keys[i + 1].time) {
        float span = keys[i + 1].time - keys[i].time;
        float t = (span > 0.0001f) ? (time - keys[i].time) / span : 0.0f;
        float weight = TweenEasing::evaluate(t, keys[i].transition, keys[i].ease);
        return keys[i].value.lerp(keys[i + 1].value, weight);
      }
    }
    return keys.back().value;
  }

  static float sampleFloat(const std::vector<AnimationKeyframe<float>> &keys, float time) {
    if (keys.empty()) return 0.0f;
    if (time <= keys.front().time) return keys.front().value;
    if (time >= keys.back().time) return keys.back().value;

    for (size_t i = 0; i + 1 < keys.size(); ++i) {
      if (time >= keys[i].time && time <= keys[i + 1].time) {
        float span = keys[i + 1].time - keys[i].time;
        float t = (span > 0.0001f) ? (time - keys[i].time) / span : 0.0f;
        float weight = TweenEasing::evaluate(t, keys[i].transition, keys[i].ease);
        return keys[i].value + (keys[i + 1].value - keys[i].value) * weight;
      }
    }
    return keys.back().value;
  }

  static Color sampleColor(const std::vector<AnimationKeyframe<Color>> &keys, float time) {
    if (keys.empty()) return Color::WHITE;
    if (time <= keys.front().time) return keys.front().value;
    if (time >= keys.back().time) return keys.back().value;

    for (size_t i = 0; i + 1 < keys.size(); ++i) {
      if (time >= keys[i].time && time <= keys[i + 1].time) {
        float span = keys[i + 1].time - keys[i].time;
        float t = (span > 0.0001f) ? (time - keys[i].time) / span : 0.0f;
        float weight = TweenEasing::evaluate(t, keys[i].transition, keys[i].ease);
        return keys[i].value.lerp(keys[i + 1].value, weight);
      }
    }
    return keys.back().value;
  }

  static int sampleInt(const std::vector<AnimationKeyframe<int>> &keys, float time) {
    if (keys.empty()) return 0;
    if (time < keys.front().time) return keys.front().value;
    for (int i = static_cast<int>(keys.size()) - 1; i >= 0; --i) {
      if (time >= keys[i].time) {
        return keys[i].value;
      }
    }
    return keys.back().value;
  }

  static bool sampleBool(const std::vector<AnimationKeyframe<bool>> &keys, float time) {
    if (keys.empty()) return true;
    for (int i = static_cast<int>(keys.size()) - 1; i >= 0; --i) {
      if (time >= keys[i].time) {
        return keys[i].value;
      }
    }
    return keys.back().value;
  }
};

// Animation Resource (inspired by Godot Animation)
class Animation {
public:
  std::string name;
  float length = 1.0f;
  float step = 1.0f / 30.0f;
  AnimationLoopMode loopMode = AnimationLoopMode::None;
  std::vector<AnimationTrack> tracks;

  Animation() = default;
  explicit Animation(std::string animName, float duration = 1.0f, AnimationLoopMode loop = AnimationLoopMode::None)
      : name(std::move(animName)), length(duration), loopMode(loop) {}

  int addTrack(AnimationTrackType type, std::string targetNodePath = "") {
    AnimationTrack track;
    track.type = type;
    track.nodePath = std::move(targetNodePath);
    tracks.push_back(std::move(track));
    return static_cast<int>(tracks.size()) - 1;
  }

  void trackInsertKey(int trackIdx, float time, const Vector2 &val, TransitionType trans = TransitionType::Linear, EaseType ease = EaseType::InOut) {
    if (trackIdx >= 0 && trackIdx < static_cast<int>(tracks.size())) {
      tracks[trackIdx].addKeyframe(time, val, trans, ease);
    }
  }

  void trackInsertKey(int trackIdx, float time, float val, TransitionType trans = TransitionType::Linear, EaseType ease = EaseType::InOut) {
    if (trackIdx >= 0 && trackIdx < static_cast<int>(tracks.size())) {
      tracks[trackIdx].addKeyframe(time, val, trans, ease);
    }
  }

  void trackInsertKey(int trackIdx, float time, const Color &val, TransitionType trans = TransitionType::Linear, EaseType ease = EaseType::InOut) {
    if (trackIdx >= 0 && trackIdx < static_cast<int>(tracks.size())) {
      tracks[trackIdx].addKeyframe(time, val, trans, ease);
    }
  }

  void trackInsertKey(int trackIdx, float time, int val) {
    if (trackIdx >= 0 && trackIdx < static_cast<int>(tracks.size())) {
      tracks[trackIdx].addKeyframe(time, val);
    }
  }
};
