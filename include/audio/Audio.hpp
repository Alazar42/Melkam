#pragma once

#include <SDL3/SDL.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Sound effect loaded in memory.
class Sound {
public:
  Sound() = default;

  explicit Sound(const std::string &filePath) {
    loadFromFile(filePath);
  }

  ~Sound() {
    unload();
  }

  // Non-copyable
  Sound(const Sound &) = delete;
  Sound &operator=(const Sound &) = delete;

  // Move-constructible
  Sound(Sound &&other) noexcept
      : m_buffer(other.m_buffer), m_length(other.m_length),
        m_spec(other.m_spec), m_path(std::move(other.m_path)) {
    other.m_buffer = nullptr;
    other.m_length = 0;
  }

  // Move-assignable
  Sound &operator=(Sound &&other) noexcept {
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

  // Loads a WAV sound effect into memory.
  bool loadFromFile(const std::string &filePath) {
    unload();
    m_path = filePath;

    if (!SDL_LoadWAV(filePath.c_str(), &m_spec, &m_buffer, &m_length)) {
      std::cerr << "[Audio Error] Failed to load sound '" << filePath
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

private:
  uint8_t *m_buffer = nullptr;
  uint32_t m_length = 0;
  SDL_AudioSpec m_spec{};
  std::string m_path;
};

// High-performance SDL3 Audio subsystem for sound effects and music playback.
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

  // Plays a loaded Sound instance.
  static void play(const Sound &sound, float volume = 1.0f) {
    if (!s_initialized || !sound.isValid() || !s_deviceId) return;

    SDL_AudioStream *stream = SDL_CreateAudioStream(&sound.getSpec(), nullptr);
    if (!stream) return;

    if (!SDL_BindAudioStream(s_deviceId, stream)) {
      SDL_DestroyAudioStream(stream);
      return;
    }

    float finalVol = std::clamp(volume * s_masterVolume, 0.0f, 1.0f);
    SDL_SetAudioStreamGain(stream, finalVol);
    SDL_PutAudioStreamData(stream, sound.getBuffer(), sound.getLength());
    SDL_FlushAudioStream(stream);
  }

  // Plays a sound from file directly.
  static void play(const std::string &filePath, float volume = 1.0f) {
    Sound sound(filePath);
    play(sound, volume);
  }

  // Plays a background music track (supports looping).
  static void playMusic(const std::string &filePath, float volume = 1.0f,
                        bool loop = true) {
    if (!s_initialized || !s_deviceId) return;

    stopMusic();

    s_musicSound = std::make_unique<Sound>(filePath);
    if (!s_musicSound->isValid()) return;

    s_musicStream = SDL_CreateAudioStream(&s_musicSound->getSpec(), nullptr);
    if (!s_musicStream) return;

    if (!SDL_BindAudioStream(s_deviceId, s_musicStream)) {
      SDL_DestroyAudioStream(s_musicStream);
      s_musicStream = nullptr;
      return;
    }

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

    // If stream has emptied or has less data than 100ms, queue next loop
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

private:
  inline static SDL_AudioDeviceID s_deviceId = 0;
  inline static bool s_initialized = false;
  inline static float s_masterVolume = 1.0f;

  inline static std::unique_ptr<Sound> s_musicSound = nullptr;
  inline static SDL_AudioStream *s_musicStream = nullptr;
  inline static float s_musicVolume = 1.0f;
  inline static bool s_musicLooping = false;
};
