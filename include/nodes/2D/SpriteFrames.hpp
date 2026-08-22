#pragma once

#include "helper/Rect2.hpp"
#include "renderers/Texture2D.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Single frame in a SpriteFrames animation track.
struct SpriteFrame {
  std::shared_ptr<Texture2D> texture = nullptr;
  Rect2 region{};            // If hasArea(), uses sub-region; otherwise uses whole texture
  float duration = 1.0f;     // Relative duration multiplier (1.0 = normal frame time)

  SpriteFrame() = default;
  explicit SpriteFrame(std::shared_ptr<Texture2D> tex, const Rect2 &reg = Rect2(),
                       float dur = 1.0f)
      : texture(std::move(tex)), region(reg), duration(dur) {}
};

// Animation track containing frames, speed, and looping settings.
struct SpriteAnimationTrack {
  std::string name = "default";
  float speed = 10.0f;       // Frames per second (FPS)
  bool loop = true;          // Loop animation when reaching the end
  std::vector<SpriteFrame> frames;

  SpriteAnimationTrack() = default;
  explicit SpriteAnimationTrack(std::string trackName, float fps = 10.0f, bool isLooped = true)
      : name(std::move(trackName)), speed(fps), loop(isLooped) {}
};

// Animation Library Resource (inspired by Godot SpriteFrames) containing multiple animation tracks.
class SpriteFrames {
public:
  SpriteFrames() {
    addAnimation("default");
  }

  // Adds a new animation track.
  void addAnimation(const std::string &animName, float fps = 10.0f, bool loop = true) {
    if (m_tracks.find(animName) == m_tracks.end()) {
      m_tracks[animName] = SpriteAnimationTrack(animName, fps, loop);
    }
  }

  // Returns true if the animation track exists.
  bool hasAnimation(const std::string &animName) const {
    return m_tracks.find(animName) != m_tracks.end();
  }

  // Removes an animation track.
  void removeAnimation(const std::string &animName) {
    m_tracks.erase(animName);
  }

  // Returns list of all available animation names.
  std::vector<std::string> getAnimationNames() const {
    std::vector<std::string> names;
    names.reserve(m_tracks.size());
    for (const auto &[name, _] : m_tracks) {
      names.push_back(name);
    }
    return names;
  }

  // Sets playback speed in frames per second (FPS).
  void setAnimationSpeed(const std::string &animName, float fps) {
    auto it = m_tracks.find(animName);
    if (it != m_tracks.end()) {
      it->second.speed = std::max(0.1f, fps);
    }
  }

  // Returns playback speed in FPS.
  float getAnimationSpeed(const std::string &animName) const {
    auto it = m_tracks.find(animName);
    return (it != m_tracks.end()) ? it->second.speed : 10.0f;
  }

  // Enables or disables looping for an animation track.
  void setAnimationLoop(const std::string &animName, bool loop) {
    auto it = m_tracks.find(animName);
    if (it != m_tracks.end()) {
      it->second.loop = loop;
    }
  }

  // Returns true if the animation loops.
  bool getAnimationLoop(const std::string &animName) const {
    auto it = m_tracks.find(animName);
    return (it != m_tracks.end()) ? it->second.loop : true;
  }

  // Adds a frame to an animation track.
  void addFrame(const std::string &animName, std::shared_ptr<Texture2D> texture,
                const Rect2 &region = Rect2(), float duration = 1.0f) {
    if (!hasAnimation(animName)) {
      addAnimation(animName);
    }
    m_tracks[animName].frames.emplace_back(std::move(texture), region, duration);
  }

  // Slices a sprite sheet grid and adds all frames to an animation track in a single call.
  void addSpriteSheetAnimation(const std::string &animName,
                               std::shared_ptr<Texture2D> sheet,
                               int hframes, int vframes,
                               int startFrame = 0, int endFrame = -1,
                               float fps = 10.0f, bool loop = true) {
    if (!sheet || !sheet->isValid() || hframes <= 0 || vframes <= 0) return;

    addAnimation(animName, fps, loop);

    float frameW = static_cast<float>(sheet->getWidth()) / static_cast<float>(hframes);
    float frameH = static_cast<float>(sheet->getHeight()) / static_cast<float>(vframes);
    int totalFrames = hframes * vframes;
    int last = (endFrame >= 0) ? std::min(endFrame, totalFrames - 1) : (totalFrames - 1);

    for (int i = startFrame; i <= last; ++i) {
      int col = i % hframes;
      int row = i / hframes;
      Rect2 region(col * frameW, row * frameH, frameW, frameH);
      addFrame(animName, sheet, region);
    }
  }

  // Returns number of frames in the specified animation track.
  int getFrameCount(const std::string &animName) const {
    auto it = m_tracks.find(animName);
    return (it != m_tracks.end()) ? static_cast<int>(it->second.frames.size()) : 0;
  }

  // Returns reference to a specific frame.
  const SpriteFrame *getFrame(const std::string &animName, int frameIdx) const {
    auto it = m_tracks.find(animName);
    if (it == m_tracks.end() || it->second.frames.empty()) return nullptr;

    int idx = std::clamp(frameIdx, 0, static_cast<int>(it->second.frames.size()) - 1);
    return &it->second.frames[idx];
  }

  // Clears all frames from an animation track.
  void clearFrames(const std::string &animName) {
    auto it = m_tracks.find(animName);
    if (it != m_tracks.end()) {
      it->second.frames.clear();
    }
  }

private:
  std::unordered_map<std::string, SpriteAnimationTrack> m_tracks;
};
