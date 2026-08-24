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

  // Plays a procedural stereo audio buffer with left/right volume panning
  static void playSpatialBuffer(const int16_t *samples, int sampleCount, int sampleRate, float leftVol, float rightVol) {
    if (!s_initialized || !s_deviceId || !samples || sampleCount <= 0) return;

    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_S16LE;
    spec.channels = 2;
    spec.freq = sampleRate;

    SDL_AudioStream *stream = createAudioStream(spec);
    if (!stream) return;

    std::vector<int16_t> stereo(sampleCount * 2);
    float master = s_masterVolume;
    float lGain = std::clamp(leftVol * master, 0.0f, 1.0f);
    float rGain = std::clamp(rightVol * master, 0.0f, 1.0f);

    for (int i = 0; i < sampleCount; ++i) {
      stereo[i * 2 + 0] = static_cast<int16_t>(std::clamp(static_cast<float>(samples[i]) * lGain, -32768.0f, 32767.0f));
      stereo[i * 2 + 1] = static_cast<int16_t>(std::clamp(static_cast<float>(samples[i]) * rGain, -32768.0f, 32767.0f));
    }

    SDL_PutAudioStreamData(stream, stereo.data(), static_cast<int>(stereo.size() * sizeof(int16_t)));
    SDL_FlushAudioStream(stream);
  }

  // Generates high-pitch golden coin pickup chime sound
  static std::vector<int16_t> createChimeSound(int sampleRate = 44100) {
    int totalSamples = static_cast<int>(0.35f * sampleRate);
    std::vector<int16_t> samples(totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
      float t = static_cast<float>(i) / static_cast<float>(sampleRate);
      float decay = std::exp(-8.0f * t);
      float freq1 = (t < 0.08f) ? 987.77f : 1318.51f; // B5 -> E6
      float freq2 = 2637.0f; // E7 overtone
      float wave = 0.7f * std::sin(6.2831853f * freq1 * t) + 0.3f * std::sin(6.2831853f * freq2 * t);
      samples[i] = static_cast<int16_t>(std::clamp(wave * decay * 28000.0f, -32768.0f, 32767.0f));
    }
    return samples;
  }

  // Generates upward pitch sweep jump pad / spring launch sound
  static std::vector<int16_t> createJumpPadSound(int sampleRate = 44100) {
    int totalSamples = static_cast<int>(0.4f * sampleRate);
    std::vector<int16_t> samples(totalSamples);
    for (int i = 0; i < totalSamples; ++i) {
      float t = static_cast<float>(i) / static_cast<float>(sampleRate);
      float decay = std::exp(-3.5f * t);
      float freq = 180.0f + 650.0f * (t / 0.4f);
      float wave = std::sin(6.2831853f * freq * t);
      samples[i] = static_cast<int16_t>(std::clamp(wave * decay * 26000.0f, -32768.0f, 32767.0f));
    }
    return samples;
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
