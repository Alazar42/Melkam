#pragma once

#include "audio/Audio.hpp"
#include "core/Node.hpp"
#include "core/Signal.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

// Non-positional Audio Stream Player Node (inspired by Godot AudioStreamPlayer) for music and global SFX.
class AudioStreamPlayer : public Node {
public:
  // Signals
  Signal<> finished;

  // Audio Resource
  std::shared_ptr<AudioStream> stream = nullptr;

  // Playback Properties
  float volumeDb = 0.0f;     // Volume in decibels (0 dB = 100%, -6 dB ~ 50%, -80 dB = mute)
  float pitchScale = 1.0f;   // Pitch / speed multiplier (1.0 = normal)
  bool autoplay = false;     // Automatically play when entering scene tree
  bool streamPaused = false;
  std::string bus = "Master";

  AudioStreamPlayer() : Node("AudioStreamPlayer") {}

  explicit AudioStreamPlayer(std::shared_ptr<AudioStream> audioStream)
      : Node("AudioStreamPlayer"), stream(std::move(audioStream)) {}

  explicit AudioStreamPlayer(const std::string &audioPath)
      : Node("AudioStreamPlayer") {
    stream = std::make_shared<AudioStream>(audioPath);
  }

  ~AudioStreamPlayer() override {
    stop();
  }

  void onReady() override {
    if (autoplay && stream && stream->isValid()) {
      play();
    }
  }

  // Converts decibels (dB) to linear gain multiplier [0.0 .. 1.0+]
  static float dbToLinear(float db) {
    if (db <= -80.0f) return 0.0f;
    return std::pow(10.0f, db / 20.0f);
  }

  // Converts linear gain multiplier to decibels (dB)
  static float linearToDb(float linear) {
    if (linear <= 0.0001f) return -80.0f;
    return 20.0f * std::log10(linear);
  }

  void setVolumeLinear(float linear) {
    volumeDb = linearToDb(linear);
    updateVolume();
  }

  float getVolumeLinear() const {
    return dbToLinear(volumeDb);
  }

  // Starts or restarts playback.
  void play(float fromPosition = 0.0f) {
    stop();
    if (!stream || !stream->isValid()) return;

    m_audioStream = Audio::createAudioStream(stream->getSpec());
    if (!m_audioStream) return;

    updateVolume();
    updatePitch();

    // Compute start sample byte offset
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

  // Stops playback and destroys active audio stream.
  void stop() {
    if (m_audioStream) {
      SDL_DestroyAudioStream(m_audioStream);
      m_audioStream = nullptr;
    }
    m_isPlaying = false;
    m_playbackPosition = 0.0f;
  }

  // Returns true if audio is actively playing.
  bool isPlaying() const { return m_isPlaying; }

  // Returns current playback position in seconds.
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

    // Check if the stream has finished playing
    if (SDL_GetAudioStreamAvailable(m_audioStream) == 0 &&
        SDL_GetAudioStreamQueued(m_audioStream) == 0) {
      stop();
      finished.emit();
    }
  }

protected:
  virtual void updateVolume() {
    if (m_audioStream) {
      float gain = std::clamp(getVolumeLinear() * Audio::getMasterVolume(), 0.0f, 4.0f);
      SDL_SetAudioStreamGain(m_audioStream, gain);
    }
  }

  virtual void updatePitch() {
    if (m_audioStream) {
      SDL_SetAudioStreamFrequencyRatio(m_audioStream, std::clamp(pitchScale, 0.1f, 4.0f));
    }
  }

  SDL_AudioStream *m_audioStream = nullptr;
  bool m_isPlaying = false;
  float m_playbackPosition = 0.0f;
};
