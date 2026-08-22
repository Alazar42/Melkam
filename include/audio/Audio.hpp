#pragma once

#include "helper/vectors/Vector2.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Audio Stream Resource (inspired by Godot AudioStream) containing raw audio data.
class AudioStream {
public:
  AudioStream() = default;

  explicit AudioStream(const std::string &filePath) {
    loadFromFile(filePath);
  }

  ~AudioStream() {
    unload();
  }

  // Non-copyable
  AudioStream(const AudioStream &) = delete;
  AudioStream &operator=(const AudioStream &) = delete;

  // Move-constructible
  AudioStream(AudioStream &&other) noexcept
      : m_buffer(other.m_buffer), m_length(other.m_length),
        m_spec(other.m_spec), m_path(std::move(other.m_path)) {
    other.m_buffer = nullptr;
    other.m_length = 0;
  }

  // Move-assignable
  AudioStream &operator=(AudioStream &&other) noexcept {
    if (this != &other) {
      unload();
      m_buffer = other.m_buffer;
      m_length = other.m_length;
      m_spec = other.m_spec;
      m_path = std::move(other.m_path);

      other.m_buffer = nullptr;
      other.m_length = 0;
    }
    return *this;
  }

  // Loads a WAV audio file into memory.
  bool loadFromFile(const std::string &filePath) {
    unload();
    m_path = filePath;

    if (!SDL_LoadWAV(filePath.c_str(), &m_spec, &m_buffer, &m_length)) {
      std::cerr << "[AudioStream Error] Failed to load audio '" << filePath
                << "': " << SDL_GetError() << std::endl;
      return false;
    }
    return true;
  }

  // Unloads the audio buffer.
  void unload() {
    if (m_buffer) {
      SDL_free(m_buffer);
      m_buffer = nullptr;
    }
    m_length = 0;
  }

  bool isValid() const { return m_buffer != nullptr && m_length > 0; }
  const uint8_t *getBuffer() const { return m_buffer; }
  uint32_t getLength() const { return m_length; }
  const SDL_AudioSpec &getSpec() const { return m_spec; }
  const std::string &getPath() const { return m_path; }

  // Returns playback duration in seconds.
  float getLengthSeconds() const {
    if (!isValid() || m_spec.freq <= 0 || m_spec.channels <= 0) return 0.0f;
    int bytesPerSample = (m_spec.format == SDL_AUDIO_S16LE || m_spec.format == SDL_AUDIO_S16BE) ? 2 : 4;
    int totalSamples = m_length / (bytesPerSample * m_spec.channels);
    return static_cast<float>(totalSamples) / static_cast<float>(m_spec.freq);
  }

private:
  uint8_t *m_buffer = nullptr;
  uint32_t m_length = 0;
  SDL_AudioSpec m_spec{};
  std::string m_path;
};

// Backwards compatibility alias
using Sound = AudioStream;

// High-performance SDL3 Audio subsystem for sound effects, spatial nodes, and music playback.
class Audio {
public:
  // Initializes the audio device and subsystem.
  static bool init() {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
      std::cerr << "[Audio Error] Failed to initialize SDL audio: "
                << SDL_GetError() << std::endl;
      return false;
    }

    s_deviceId = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!s_deviceId) {
      std::cerr << "[Audio Warning] Failed to open playback audio device: "
                << SDL_GetError() << std::endl;
      return false;
    }

    SDL_ResumeAudioDevice(s_deviceId);
    s_initialized = true;
    return true;
  }

  // Shuts down audio device and streams.
  static void shutdown() {
    stopMusic();
    if (s_deviceId) {
      SDL_CloseAudioDevice(s_deviceId);
      s_deviceId = 0;
    }
    s_initialized = false;
  }

  // Returns active SDL audio device ID.
  static SDL_AudioDeviceID getDeviceId() { return s_deviceId; }
  static bool isInitialized() { return s_initialized; }

  // Creates and binds a new native SDL_AudioStream to the default audio device.
  static SDL_AudioStream *createAudioStream(const SDL_AudioSpec &spec) {
    if (!s_initialized || !s_deviceId) return nullptr;

    SDL_AudioStream *stream = SDL_CreateAudioStream(&spec, nullptr);
    if (!stream) return nullptr;

    if (!SDL_BindAudioStream(s_deviceId, stream)) {
      SDL_DestroyAudioStream(stream);
      return nullptr;
    }
    return stream;
  }

  // Plays a loaded AudioStream directly once (one-shot).
  static void play(const AudioStream &sound, float volume = 1.0f) {
    if (!s_initialized || !sound.isValid() || !s_deviceId) return;

    SDL_AudioStream *stream = createAudioStream(sound.getSpec());
    if (!stream) return;

    float finalVol = std::clamp(volume * s_masterVolume, 0.0f, 1.0f);
    SDL_SetAudioStreamGain(stream, finalVol);
    SDL_PutAudioStreamData(stream, sound.getBuffer(), sound.getLength());
    SDL_FlushAudioStream(stream);
  }

  // Plays a sound from file directly.
  static void play(const std::string &filePath, float volume = 1.0f) {
    AudioStream sound(filePath);
    play(sound, volume);
  }

  // Plays a background music track (supports looping).
  static void playMusic(const std::string &filePath, float volume = 1.0f,
                        bool loop = true) {
    if (!s_initialized || !s_deviceId) return;

    stopMusic();

    s_musicSound = std::make_unique<AudioStream>(filePath);
    if (!s_musicSound->isValid()) return;

    s_musicStream = createAudioStream(s_musicSound->getSpec());
    if (!s_musicStream) return;

    s_musicVolume = volume;
    s_musicLooping = loop;

    float finalVol = std::clamp(volume * s_masterVolume, 0.0f, 1.0f);
    SDL_SetAudioStreamGain(s_musicStream, finalVol);
    SDL_PutAudioStreamData(s_musicStream, s_musicSound->getBuffer(),
                           s_musicSound->getLength());
  }

  // Updates background music stream looping. Call in main loop.
  static void update() {
    if (!s_initialized || !s_musicStream || !s_musicSound || !s_musicLooping) {
      return;
    }

    if (SDL_GetAudioStreamAvailable(s_musicStream) == 0) {
      SDL_PutAudioStreamData(s_musicStream, s_musicSound->getBuffer(),
                             s_musicSound->getLength());
    }
  }

  // Stops current background music.
  static void stopMusic() {
    if (s_musicStream) {
      SDL_DestroyAudioStream(s_musicStream);
      s_musicStream = nullptr;
    }
    s_musicSound = nullptr;
  }

  // Sets master volume multiplier (0.0f to 1.0f).
  static void setMasterVolume(float volume) {
    s_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    if (s_musicStream) {
      SDL_SetAudioStreamGain(s_musicStream, s_musicVolume * s_masterVolume);
    }
  }

  // Returns current master volume.
  static float getMasterVolume() { return s_masterVolume; }

  // Global listener position for 2D spatial audio
  static void setListenerPosition(const Vector2 &pos) { s_listenerPos = pos; }
  static const Vector2 &getListenerPosition() { return s_listenerPos; }

private:
  inline static SDL_AudioDeviceID s_deviceId = 0;
  inline static bool s_initialized = false;
  inline static float s_masterVolume = 1.0f;
  inline static Vector2 s_listenerPos{640.0f, 360.0f};

  inline static std::unique_ptr<AudioStream> s_musicSound = nullptr;
  inline static SDL_AudioStream *s_musicStream = nullptr;
  inline static float s_musicVolume = 1.0f;
  inline static bool s_musicLooping = false;
};
