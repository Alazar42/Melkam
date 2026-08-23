#pragma once

#include "core/Memory.hpp"
#include "core/Signal.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <vector>

enum class TransitionType {
  Linear,
  Sine,
  Quad,
  Cubic,
  Quart,
  Quint,
  Expo,
  Circ,
  Back,
  Bounce,
  Elastic
};

enum class EaseType {
  In,
  Out,
  InOut,
  OutIn
};

namespace TweenEasing {
inline float easeRaw(float t, TransitionType trans) {
  switch (trans) {
  case TransitionType::Linear:
    return t;
  case TransitionType::Sine:
    return 1.0f - std::cos(t * (3.14159265f * 0.5f));
  case TransitionType::Quad:
    return t * t;
  case TransitionType::Cubic:
    return t * t * t;
  case TransitionType::Quart:
    return t * t * t * t;
  case TransitionType::Quint:
    return t * t * t * t * t;
  case TransitionType::Expo:
    return (t == 0.0f) ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f));
  case TransitionType::Circ:
    return 1.0f - std::sqrt(std::max(0.0f, 1.0f - t * t));
  case TransitionType::Back: {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
  }
  case TransitionType::Elastic: {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    const float c4 = (2.0f * 3.14159265f) / 3.0f;
    return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);
  }
  case TransitionType::Bounce: {
    // Standard bounce-out evaluated on (1-t) for bounce-in
    float inv = 1.0f - t;
    float b = 0.0f;
    if (inv < 1.0f / 2.75f) {
      b = 7.5625f * inv * inv;
    } else if (inv < 2.0f / 2.75f) {
      inv -= 1.5f / 2.75f;
      b = 7.5625f * inv * inv + 0.75f;
    } else if (inv < 2.5f / 2.75f) {
      inv -= 2.25f / 2.75f;
      b = 7.5625f * inv * inv + 0.9375f;
    } else {
      inv -= 2.625f / 2.75f;
      b = 7.5625f * inv * inv + 0.984375f;
    }
    return 1.0f - b;
  }
  }
  return t;
}

inline float evaluate(float t, TransitionType trans, EaseType ease) {
  t = std::clamp(t, 0.0f, 1.0f);
  switch (ease) {
  case EaseType::In:
    return easeRaw(t, trans);
  case EaseType::Out:
    return 1.0f - easeRaw(1.0f - t, trans);
  case EaseType::InOut: {
    if (t < 0.5f) {
      return easeRaw(t * 2.0f, trans) * 0.5f;
    } else {
      return 1.0f - easeRaw((1.0f - t) * 2.0f, trans) * 0.5f;
    }
  }
  case EaseType::OutIn: {
    if (t < 0.5f) {
      return (1.0f - easeRaw(1.0f - (t * 2.0f), trans)) * 0.5f;
    } else {
      return 0.5f + easeRaw((t - 0.5f) * 2.0f, trans) * 0.5f;
    }
  }
  }
  return t;
}
} // namespace TweenEasing

// Base interface for all tweenable actions
class Tweener {
public:
  virtual ~Tweener() = default;
  virtual bool process(float delta) = 0;
  virtual void start() {}
};

// Property Interpolator Tweener
template <typename T>
class PropertyTweener : public Tweener {
public:
  PropertyTweener(std::function<T()> getter, std::function<void(T)> setter,
                  T targetVal, float dur, TransitionType trans, EaseType ease)
      : m_getter(std::move(getter)), m_setter(std::move(setter)),
        m_targetVal(targetVal), m_duration(std::max(0.0001f, dur)),
        m_trans(trans), m_ease(ease) {}

  void start() override {
    if (m_getter) {
      m_startVal = m_getter();
    }
    m_elapsed = 0.0f;
    m_started = true;
  }

  bool process(float delta) override {
    if (!m_started) start();
    m_elapsed += delta;
    float t = std::clamp(m_elapsed / m_duration, 0.0f, 1.0f);
    float weight = TweenEasing::evaluate(t, m_trans, m_ease);

    if (m_setter) {
      m_setter(interpolate(m_startVal, m_targetVal, weight));
    }

    return m_elapsed >= m_duration;
  }

private:
  static T interpolate(const T &a, const T &b, float f) {
    return a + (b - a) * f;
  }

  std::function<T()> m_getter;
  std::function<void(T)> m_setter;
  T m_startVal{};
  T m_targetVal{};
  float m_duration = 1.0f;
  float m_elapsed = 0.0f;
  TransitionType m_trans = TransitionType::Linear;
  EaseType m_ease = EaseType::InOut;
  bool m_started = false;
};

// Specialization for Color interpolation
template <>
inline Color PropertyTweener<Color>::interpolate(const Color &a, const Color &b, float f) {
  return a.lerp(b, f);
}

// Specialization for Vector2 interpolation
template <>
inline Vector2 PropertyTweener<Vector2>::interpolate(const Vector2 &a, const Vector2 &b, float f) {
  return a.lerp(b, f);
}

// Callback Tweener
class CallbackTweener : public Tweener {
public:
  explicit CallbackTweener(std::function<void()> callback)
      : m_callback(std::move(callback)) {}

  bool process(float delta) override {
    (void)delta;
    if (m_callback) m_callback();
    return true;
  }

private:
  std::function<void()> m_callback;
};

// Interval / Delay Tweener
class IntervalTweener : public Tweener {
public:
  explicit IntervalTweener(float duration)
      : m_duration(std::max(0.0f, duration)) {}

  bool process(float delta) override {
    m_elapsed += delta;
    return m_elapsed >= m_duration;
  }

private:
  float m_duration = 0.0f;
  float m_elapsed = 0.0f;
};

// Fluent Godot-Style Tween Animation Controller (inspired by Godot 4 SceneTreeTween)
class Tween {
public:
  Signal<> finished;
  Signal<> loop_finished;
  Signal<> step_finished;

  Tween() = default;

  Tween *setTrans(TransitionType trans) {
    m_defaultTrans = trans;
    return this;
  }

  Tween *setEase(EaseType ease) {
    m_defaultEase = ease;
    return this;
  }

  Tween *setParallel(bool parallel = true) {
    m_parallel = parallel;
    return this;
  }

  Tween *chain() {
    m_parallel = false;
    return this;
  }

  Tween *setLoops(int loopCount = 0) {
    m_loops = loopCount;
    return this;
  }

  template <typename T>
  Tween *tweenProperty(std::function<T()> getter, std::function<void(T)> setter,
                       T targetValue, float duration) {
    auto tweener = std::make_shared<PropertyTweener<T>>(
        std::move(getter), std::move(setter), targetValue, duration,
        m_defaultTrans, m_defaultEase);
    addTweener(tweener);
    return this;
  }

  Tween *tweenCallback(std::function<void()> callback) {
    addTweener(std::make_shared<CallbackTweener>(std::move(callback)));
    return this;
  }

  Tween *tweenInterval(float duration) {
    addTweener(std::make_shared<IntervalTweener>(duration));
    return this;
  }

  void pause() { m_running = false; }
  void play() { m_running = true; }
  void kill() { m_killed = true; }
  bool isRunning() const { return m_running && !m_killed; }
  bool isKilled() const { return m_killed; }

  bool process(float delta) {
    if (!m_running || m_killed || m_steps.empty()) return true;

    if (m_currentStep >= static_cast<int>(m_steps.size())) {
      m_loopsDone++;
      loop_finished.emit();

      if (m_loops > 0 && m_loopsDone >= m_loops) {
        m_killed = true;
        finished.emit();
        return true;
      }

      // Reset for next loop
      m_currentStep = 0;
      for (auto &step : m_steps) {
        for (auto &tweener : step) {
          tweener->start();
        }
      }
    }

    auto &currentParallelGroup = m_steps[m_currentStep];
    bool allDone = true;

    for (auto &tweener : currentParallelGroup) {
      if (!tweener->process(delta)) {
        allDone = false;
      }
    }

    if (allDone) {
      step_finished.emit();
      m_currentStep++;

      if (m_currentStep >= static_cast<int>(m_steps.size())) {
        m_loopsDone++;
        loop_finished.emit();

        if (m_loops > 0 && m_loopsDone >= m_loops) {
          m_killed = true;
          finished.emit();
          return true;
        }

        // Loop reset
        m_currentStep = 0;
        for (auto &step : m_steps) {
          for (auto &tweener : step) {
            tweener->start();
          }
        }
      }
    }

    return false;
  }

private:
  void addTweener(std::shared_ptr<Tweener> tweener) {
    if (m_parallel && !m_steps.empty()) {
      m_steps.back().push_back(tweener);
    } else {
      m_steps.push_back({tweener});
    }
  }

  std::vector<std::vector<std::shared_ptr<Tweener>>> m_steps;
  int m_currentStep = 0;
  TransitionType m_defaultTrans = TransitionType::Quad;
  EaseType m_defaultEase = EaseType::Out;
  bool m_parallel = false;
  bool m_running = true;
  bool m_killed = false;
  int m_loops = 1;
  int m_loopsDone = 0;
};
