#pragma once

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstdint>

// High-precision Time subsystem managing delta time, frame rates, and time scaling.
class Time {
public:
  // Updates the time subsystem metrics.
  // Called automatically by Window::pollEvents() at the beginning of each frame.
  static void update() {
    uint64_t currentCounter = SDL_GetPerformanceCounter();
    if (s_lastCounter == 0) {
      s_lastCounter = currentCounter;
      s_startCounter = currentCounter;
    }

    uint64_t counterFrequency = SDL_GetPerformanceFrequency();
    double rawDelta = static_cast<double>(currentCounter - s_lastCounter) /
                      static_cast<double>(counterFrequency);
    s_lastCounter = currentCounter;

    // Clamp delta time to avoid large spikes (e.g. when dragging window or breaking in debugger)
    float clampedDelta = std::min(static_cast<float>(rawDelta), s_maxDeltaTime);

    s_unscaledDeltaTime = clampedDelta;
    s_deltaTime = s_isPaused ? 0.0f : (clampedDelta * s_timeScale);

    s_unscaledTime += s_unscaledDeltaTime;
    s_time += s_deltaTime;

    s_fixedAccumulator += s_deltaTime;
    s_frameCount++;

    // FPS calculation (smoothed over 0.25 seconds)
    s_fpsTimer += s_unscaledDeltaTime;
    s_fpsFrameCounter++;
    if (s_fpsTimer >= 0.25f) {
      s_fps = static_cast<float>(s_fpsFrameCounter) / s_fpsTimer;
      s_fpsFrameCounter = 0;
      s_fpsTimer = 0.0f;
    }
  }

  // Returns the time in seconds elapsed between the current and previous frame (affected by timeScale).
  static float getDeltaTime() { return s_deltaTime; }

  // Returns the delta time in milliseconds (affected by timeScale).
  static float getDeltaTimeMs() { return s_deltaTime * 1000.0f; }

  // Returns the raw, unscaled delta time in seconds (unaffected by pause or timeScale).
  static float getUnscaledDeltaTime() { return s_unscaledDeltaTime; }

  // Returns the total scaled time in seconds since the engine started.
  static float getTime() { return s_time; }

  // Returns the total unscaled real-world time in seconds since the engine started.
  static float getUnscaledTime() { return s_unscaledTime; }

  // Returns the current smoothed frames per second (FPS).
  static float getFPS() { return s_fps; }

  // Returns the total number of frames rendered since engine start.
  static uint64_t getFrameCount() { return s_frameCount; }

  // Sets the time multiplier (e.g. 0.5f for slow-motion, 2.0f for fast-forward, 1.0f for normal).
  static void setTimeScale(float scale) { s_timeScale = std::max(scale, 0.0f); }

  // Returns the current time multiplier.
  static float getTimeScale() { return s_timeScale; }

  // Pauses or unpauses game time (deltaTime becomes 0 when paused; unscaled delta time continues).
  static void setPaused(bool paused) { s_isPaused = paused; }

  // Returns true if game time is currently paused.
  static bool isPaused() { return s_isPaused; }

  // Sets the fixed timestep interval for physics updates (default is 1/60s).
  static void setFixedDeltaTime(float fixedDt) {
    if (fixedDt > 0.0f) {
      s_fixedDeltaTime = fixedDt;
    }
  }

  // Returns the fixed timestep interval in seconds.
  static float getFixedDeltaTime() { return s_fixedDeltaTime; }

  // Returns true if enough time has accumulated to execute a fixed physics step.
  // Consumes one fixed step interval when true.
  // Usage:
  //   while (Time::shouldDoFixedUpdate()) {
  //       physics.step(Time::getFixedDeltaTime());
  //   }
  static bool shouldDoFixedUpdate() {
    if (s_fixedAccumulator >= s_fixedDeltaTime) {
      s_fixedAccumulator -= s_fixedDeltaTime;
      return true;
    }
    return false;
  }

private:
  inline static uint64_t s_startCounter = 0;
  inline static uint64_t s_lastCounter = 0;

  inline static float s_deltaTime = 0.0f;
  inline static float s_unscaledDeltaTime = 0.0f;
  inline static float s_maxDeltaTime = 0.25f;

  inline static float s_time = 0.0f;
  inline static float s_unscaledTime = 0.0f;
  inline static float s_timeScale = 1.0f;
  inline static bool s_isPaused = false;

  inline static float s_fixedDeltaTime = 1.0f / 60.0f;
  inline static float s_fixedAccumulator = 0.0f;

  inline static uint64_t s_frameCount = 0;
  inline static float s_fps = 0.0f;
  inline static float s_fpsTimer = 0.0f;
  inline static uint32_t s_fpsFrameCounter = 0;
};

// General-purpose Stopwatch/Timer utility for cooldowns, delays, or profiling.
class Timer {
public:
  Timer() { reset(); }

  // Resets the timer to start from the current moment.
  void reset() {
    m_startCounter = SDL_GetPerformanceCounter();
    m_pausedCounter = 0;
    m_isPaused = false;
  }

  // Pauses the timer.
  void pause() {
    if (!m_isPaused) {
      m_pausedCounter = SDL_GetPerformanceCounter();
      m_isPaused = true;
    }
  }

  // Resumes the timer from paused state.
  void resume() {
    if (m_isPaused) {
      uint64_t pauseDuration = SDL_GetPerformanceCounter() - m_pausedCounter;
      m_startCounter += pauseDuration;
      m_isPaused = false;
    }
  }

  // Returns elapsed time in seconds.
  float elapsed() const {
    uint64_t endCounter = m_isPaused ? m_pausedCounter : SDL_GetPerformanceCounter();
    uint64_t frequency = SDL_GetPerformanceFrequency();
    return static_cast<float>(endCounter - m_startCounter) /
           static_cast<float>(frequency);
  }

  // Returns elapsed time in milliseconds.
  float elapsedMs() const { return elapsed() * 1000.0f; }

  // Returns true if the timer has reached or exceeded the specified duration in seconds.
  bool hasElapsed(float durationInSeconds) const {
    return elapsed() >= durationInSeconds;
  }

  // Returns true if the timer is currently paused.
  bool isPaused() const { return m_isPaused; }

private:
  uint64_t m_startCounter = 0;
  uint64_t m_pausedCounter = 0;
  bool m_isPaused = false;
};
