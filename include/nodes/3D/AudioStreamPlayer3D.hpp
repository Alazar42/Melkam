#pragma once

#include "audio/Audio.hpp"
#include "nodes/3D/Camera3D.hpp"
#include "nodes/3D/Node3D.hpp"
#include <algorithm>
#include <cmath>

// Godot 4-style 3D Positional Audio Node
class AudioStreamPlayer3D : public Node3D {
public:
  float maxDistance = 35.0f;
  float unitSize = 3.0f;
  float volumeDb = 0.0f;
  float pitchScale = 1.0f;
  bool autoplay = false;

  AudioStreamPlayer3D() : Node3D("AudioStreamPlayer3D") {}
  explicit AudioStreamPlayer3D(std::string name) : Node3D(std::move(name)) {}

  // Plays a procedural or loaded sound with full 3D spatial attenuation & stereo panning
  void playSound(const std::vector<int16_t> &samples, int sampleRate = 44100) {
    if (samples.empty()) return;

    Vector3 myPos = getGlobalPosition();
    Vector3 listenerPos(0.0f, 0.0f, 0.0f);
    Vector3 listenerRight(1.0f, 0.0f, 0.0f);

    const Camera3D *cam = Camera3D::getCurrent();
    if (cam) {
      Transform3D camTrans = cam->getGlobalTransform();
      listenerPos = camTrans.origin;
      listenerRight = camTrans.basis.xform(Vector3(1.0f, 0.0f, 0.0f)).normalized();
    }

    Vector3 toSource = myPos - listenerPos;
    float dist = toSource.length();

    if (dist > maxDistance) return; // Beyond audible distance

    // Distance attenuation (Inverse Distance Model)
    float atten = 1.0f / (1.0f + (dist / std::max(0.1f, unitSize)));
    atten = std::clamp(atten, 0.0f, 1.0f);

    // Stereo Panning (-1.0 left to +1.0 right)
    float pan = 0.0f;
    if (dist > 0.001f) {
      Vector3 dir = toSource / dist;
      pan = std::clamp(dir.dot(listenerRight), -1.0f, 1.0f);
    }

    float leftVol = atten * std::clamp(0.5f * (1.0f - pan) + 0.5f, 0.0f, 1.0f);
    float rightVol = atten * std::clamp(0.5f * (1.0f + pan) + 0.5f, 0.0f, 1.0f);

    Audio::playSpatialBuffer(samples.data(), static_cast<int>(samples.size()), sampleRate, leftVol, rightVol);
  }

  void playChime() {
    static std::vector<int16_t> s_chime = Audio::createChimeSound();
    playSound(s_chime);
  }

  void playJump() {
    static std::vector<int16_t> s_jump = Audio::createJumpPadSound();
    playSound(s_jump);
  }
};
