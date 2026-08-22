#pragma once

#include "audio/Audio.hpp"
#include "core/Signal.hpp"
#include "nodes/2D/Node2D.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

// 2D Spatial Positional Audio Player Node (inspired by Godot AudioStreamPlayer2D) with distance attenuation.
class AudioStreamPlayer2D : public Node2D {
public:
  // Signals
  Signal<> finished;

  // Audio Resource
  std::shared_ptr<AudioStream> stream = nullptr;

  // Spatial & Playback Properties
  float volumeDb = 0.0f;          // Base volume in decibels
  float pitchScale = 1.0f;        // Pitch multiplier
  float maxDistance = 1500.0f;    // Maximum hearing distance in world pixels
  float attenuation = 1.0f;       // Distance roll-off exponent (1.0 = linear, 2.0 = inverse square)
  float panningStrength = 1.0f;   // Stereo panning intensity [0.0 .. 1.0]
  bool autoplay = false;
  bool streamPaused = false;
  std::string bus = "Master";

  AudioStreamPlayer2D() : Node2D("AudioStreamPlayer2D") {}

  explicit AudioStreamPlayer2D(std::shared_ptr<AudioStream> audioStream)
      : Node2D("AudioStreamPlayer2D"), stream(std::move(audioStream)) {}

  explicit AudioStreamPlayer2D(const std::string &audioPath)
      : Node2D("AudioStreamPlayer2D") {
    stream = std::make_shared<AudioStream>(audioPath);
  }

  ~AudioStreamPlayer2D() override {
    stop();
  }

  void onReady() override {
    if (autoplay && stream && stream->isValid()) {
      play();
    }
  }

  static float dbToLinear(float db) {
    if (db <= -80.0f) return 0.0f;
    return std::pow(10.0f, db / 20.0f);
  }

  static float linearToDb(float linear) {
    if (linear <= 0.0001f) return -80.0f;
    return 20.0f * std::log10(linear);
  }

  void setVolumeLinear(float linear) {
    volumeDb = linearToDb(linear);
    updateSpatialAudio();
  }

  float getVolumeLinear() const {
    return dbToLinear(volumeDb);
  }

  // Starts or restarts spatial playback.
  void play(float fromPosition = 0.0f) {
    stop();
    if (!stream || !stream->isValid()) return;

    m_audioStream = Audio::createAudioStream(stream->getSpec());
    if (!m_audioStream) return;

    updateSpatialAudio();
    updatePitch();

    uint32_t startByte = 0;
    if (fromPosition > 0.0f && stream->getLengthSeconds() > 0.0f) {
      float frac = std::clamp(fromPosition / stream->getLengthSeconds(), 0.0f, 1.0f);
      startByte = static_cast<uint32_t>(frac * stream->getLength());
    }

    const uint8_t *buffer = stream->getBuffer() + startByte;
    uint32_t remainingLength = stream->getLength() - startByte;

    SDL_PutAudioStreamData(m_audioStream, buffer, remainingLength);
    SDL_FlushAudioStream(m_audioStream);

    m_isPlaying = true;
    m_playbackPosition = fromPosition;
  }

  // Stops playback.
  void stop() {
    if (m_audioStream) {
      SDL_DestroyAudioStream(m_audioStream);
      m_audioStream = nullptr;
    }
    m_isPlaying = false;
    m_playbackPosition = 0.0f;
  }

  bool isPlaying() const { return m_isPlaying; }
  float getPlaybackPosition() const { return m_playbackPosition; }

  void onProcess(float delta) override {
    if (!m_isPlaying || !m_audioStream) return;

    if (streamPaused) {
      SDL_PauseAudioStreamDevice(m_audioStream);
      return;
    } else {
      SDL_ResumeAudioStreamDevice(m_audioStream);
    }

    m_playbackPosition += delta * pitchScale;
    updateSpatialAudio();

    // Check if the stream has finished playing
    if (SDL_GetAudioStreamAvailable(m_audioStream) == 0 &&
        SDL_GetAudioStreamQueued(m_audioStream) == 0) {
      stop();
      finished.emit();
    }
  }

  // Computes distance attenuation and updates audio gain based on distance to listener.
  void updateSpatialAudio() {
    if (!m_audioStream) return;

    Vector2 emitterPos = getGlobalTransform().position;
    Vector2 listenerPos = Audio::getListenerPosition();

    float dist = emitterPos.distance_to(listenerPos);
    float distFactor = 1.0f;
    if (maxDistance > 0.0f) {
      distFactor = std::clamp(1.0f - (dist / maxDistance), 0.0f, 1.0f);
      if (attenuation > 0.0f) {
        distFactor = std::pow(distFactor, attenuation);
      }
    }

    float baseLinear = getVolumeLinear() * Audio::getMasterVolume();
    float finalGain = std::clamp(baseLinear * distFactor, 0.0f, 4.0f);

    SDL_SetAudioStreamGain(m_audioStream, finalGain);
  }

protected:
  void updatePitch() {
    if (m_audioStream) {
      SDL_SetAudioStreamFrequencyRatio(m_audioStream, std::clamp(pitchScale, 0.1f, 4.0f));
    }
  }

  SDL_AudioStream *m_audioStream = nullptr;
  bool m_isPlaying = false;
  float m_playbackPosition = 0.0f;
};
