#pragma once

#include <algorithm>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

// Define 'await' as an alias for C++20 'co_await' keyword for cleaner gameplay scripting
#ifndef await
#define await co_await
#endif

// Forward declarations
class SceneTree;
class SceneTreeTimer;
class Node;
class CoroutineScheduler;

#include "core/Signal.hpp"

// =============================================================================
// 1. GODOT-STYLE SCENETREETIMER DECLARATION
// =============================================================================

class SceneTreeTimer : public std::enable_shared_from_this<SceneTreeTimer> {
public:
  Signal<> timeout{"timeout", "SceneTreeTimer"};
  float duration = 0.0f;
  float time_left = 0.0f;

  explicit SceneTreeTimer(float seconds)
      : duration(std::max(0.0f, seconds)), time_left(duration) {}

  void emitTimeout() {
    timeout.emit();
  }

  // Makes SceneTreeTimer directly awaitable: 'await getTree()->createTimer(1.5f);'
  bool await_ready() const noexcept { return time_left <= 0.0f; }
  void await_suspend(std::coroutine_handle<> handle);
  void await_resume() const noexcept {}
};

// =============================================================================
// 2. COROUTINE SCHEDULER
// =============================================================================

class CoroutineScheduler {
public:
  struct TimerAwaiterItem {
    float timeLeft = 0.0f;
    std::coroutine_handle<> handle = nullptr;
    std::shared_ptr<bool> isAlive = nullptr;
    std::shared_ptr<SceneTreeTimer> sceneTreeTimer = nullptr;
  };

  struct ConditionAwaiterItem {
    std::function<bool()> condition;
    std::coroutine_handle<> handle = nullptr;
    std::shared_ptr<bool> isAlive = nullptr;
  };

  static CoroutineScheduler &get() {
    static CoroutineScheduler s_instance;
    return s_instance;
  }

  void addTimer(float duration, std::coroutine_handle<> handle,
                std::shared_ptr<bool> isAlive = nullptr,
                std::shared_ptr<SceneTreeTimer> timer = nullptr) {
    m_timers.push_back({duration, handle, std::move(isAlive), std::move(timer)});
  }

  void addNextFrame(std::coroutine_handle<> handle, std::shared_ptr<bool> isAlive = nullptr) {
    m_nextFrameQueue.push_back({0.0f, handle, std::move(isAlive), nullptr});
  }

  void addNextPhysicsTick(std::coroutine_handle<> handle, std::shared_ptr<bool> isAlive = nullptr) {
    m_nextPhysicsQueue.push_back({0.0f, handle, std::move(isAlive), nullptr});
  }

  void addCondition(std::function<bool()> condition, std::coroutine_handle<> handle,
                    std::shared_ptr<bool> isAlive = nullptr) {
    m_conditions.push_back({std::move(condition), handle, std::move(isAlive)});
  }

  void process(float delta) {
    // 1. Resume next-frame awaiters
    if (!m_nextFrameQueue.empty()) {
      auto current = std::move(m_nextFrameQueue);
      m_nextFrameQueue.clear();
      for (auto &item : current) {
        if (item.handle && (!item.isAlive || *item.isAlive) && !item.handle.done()) {
          item.handle.resume();
        }
      }
    }

    // 2. Process active timers safely without iterator invalidation
    if (!m_timers.empty()) {
      std::vector<std::shared_ptr<SceneTreeTimer>> timersToEmit;
      std::vector<std::coroutine_handle<>> handlesToResume;
      std::vector<TimerAwaiterItem> remainingTimers;
      remainingTimers.reserve(m_timers.size());

      auto activeTimers = std::move(m_timers);
      m_timers.clear();

      for (auto &item : activeTimers) {
        if (item.isAlive && !*item.isAlive) {
          continue;
        }

        item.timeLeft -= delta;
        if (item.timeLeft <= 0.0f) {
          if (item.sceneTreeTimer) {
            timersToEmit.push_back(item.sceneTreeTimer);
          }
          if (item.handle && !item.handle.done()) {
            handlesToResume.push_back(item.handle);
          }
        } else {
          remainingTimers.push_back(std::move(item));
        }
      }

      // Re-merge any newly queued timers added during iteration
      if (!m_timers.empty()) {
        remainingTimers.insert(remainingTimers.end(),
                               std::make_move_iterator(m_timers.begin()),
                               std::make_move_iterator(m_timers.end()));
      }
      m_timers = std::move(remainingTimers);

      for (auto &timerRef : timersToEmit) {
        if (timerRef) {
          timerRef->emitTimeout();
        }
      }

      for (auto &handle : handlesToResume) {
        if (handle && !handle.done()) {
          handle.resume();
        }
      }
    }

    // 3. Process conditional awaiters (WaitUntil / WaitWhile) safely
    if (!m_conditions.empty()) {
      std::vector<std::coroutine_handle<>> condHandlesToResume;
      std::vector<ConditionAwaiterItem> remainingConditions;
      remainingConditions.reserve(m_conditions.size());

      auto activeConditions = std::move(m_conditions);
      m_conditions.clear();

      for (auto &item : activeConditions) {
        if (item.isAlive && !*item.isAlive) {
          continue;
        }

        bool conditionMet = false;
        try {
          if (item.condition) conditionMet = item.condition();
        } catch (...) {
          conditionMet = true;
        }

        if (conditionMet) {
          if (item.handle && !item.handle.done()) {
            condHandlesToResume.push_back(item.handle);
          }
        } else {
          remainingConditions.push_back(std::move(item));
        }
      }

      if (!m_conditions.empty()) {
        remainingConditions.insert(remainingConditions.end(),
                                   std::make_move_iterator(m_conditions.begin()),
                                   std::make_move_iterator(m_conditions.end()));
      }
      m_conditions = std::move(remainingConditions);

      for (auto &handle : condHandlesToResume) {
        if (handle && !handle.done()) {
          handle.resume();
        }
      }
    }
  }

  void physicsProcess(float delta) {
    (void)delta;
    if (!m_nextPhysicsQueue.empty()) {
      auto current = std::move(m_nextPhysicsQueue);
      m_nextPhysicsQueue.clear();
      for (auto &item : current) {
        if (item.handle && (!item.isAlive || *item.isAlive) && !item.handle.done()) {
          item.handle.resume();
        }
      }
    }
  }

  void clear() {
    m_timers.clear();
    m_nextFrameQueue.clear();
    m_nextPhysicsQueue.clear();
    m_conditions.clear();
  }

private:
  std::vector<TimerAwaiterItem> m_timers;
  std::vector<TimerAwaiterItem> m_nextFrameQueue;
  std::vector<TimerAwaiterItem> m_nextPhysicsQueue;
  std::vector<ConditionAwaiterItem> m_conditions;
};

inline void SceneTreeTimer::await_suspend(std::coroutine_handle<> handle) {
  CoroutineScheduler::get().addTimer(time_left, handle, nullptr, shared_from_this());
}

// Enables direct awaiting on shared_ptr / Ref<SceneTreeTimer>: 'await getTree()->createTimer(1.5f);'
inline auto operator co_await(std::shared_ptr<SceneTreeTimer> timer) {
  struct SharedTimerAwaiter {
    std::shared_ptr<SceneTreeTimer> timer;

    bool await_ready() const noexcept {
      return !timer || timer->await_ready();
    }

    void await_suspend(std::coroutine_handle<> handle) const {
      if (timer) {
        timer->await_suspend(handle);
      }
    }

    void await_resume() const noexcept {}
  };

  return SharedTimerAwaiter{std::move(timer)};
}

// =============================================================================
// 3. C++20 TASK<T> AND TASK<VOID>
// =============================================================================

template <typename T = void>
class [[nodiscard]] Task;

// Specialization: Task<void>
template <>
class [[nodiscard]] Task<void> {
public:
  struct promise_type {
    std::coroutine_handle<> continuation = nullptr;
    std::shared_ptr<bool> isAlive = std::make_shared<bool>(true);
    bool isDone = false;

    Task get_return_object() {
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_never initial_suspend() noexcept {
      return {}; // Eager start: executes immediately until the first await
    }

    struct FinalAwaiter {
      bool await_ready() noexcept { return false; }
      std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept {
        auto &promise = h.promise();
        promise.isDone = true;
        if (promise.continuation) {
          return promise.continuation;
        }
        return std::noop_coroutine();
      }
      void await_resume() noexcept {}
    };

    FinalAwaiter final_suspend() noexcept {
      return {};
    }

    void return_void() noexcept {
      isDone = true;
    }

    void unhandled_exception() {}
  };

  using Handle = std::coroutine_handle<promise_type>;

  Task() : m_handle(nullptr) {}
  explicit Task(Handle h) : m_handle(h) {}
  ~Task() {
    if (m_handle && m_handle.done()) {
      m_handle.destroy();
    }
  }

  Task(const Task &) = delete;
  Task &operator=(const Task &) = delete;

  Task(Task &&other) noexcept : m_handle(other.m_handle) {
    other.m_handle = nullptr;
  }

  Task &operator=(Task &&other) noexcept {
    if (this != &other) {
      if (m_handle && m_handle.done()) {
        m_handle.destroy();
      }
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  // Awaiter interface: allows 'await otherTask();' inside coroutines
  bool await_ready() const noexcept {
    return !m_handle || m_handle.done();
  }

  void await_suspend(std::coroutine_handle<> cont) noexcept {
    if (m_handle) {
      m_handle.promise().continuation = cont;
    }
  }

  void await_resume() noexcept {}

  bool isDone() const {
    return !m_handle || m_handle.done();
  }

  void cancel() {
    if (m_handle && m_handle.promise().isAlive) {
      *m_handle.promise().isAlive = false;
    }
  }

  std::shared_ptr<bool> getAliveToken() const {
    return m_handle ? m_handle.promise().isAlive : nullptr;
  }

private:
  Handle m_handle = nullptr;
};

// Generic Task<T> returning a value
template <typename T>
class [[nodiscard]] Task {
public:
  struct promise_type {
    std::coroutine_handle<> continuation = nullptr;
    std::shared_ptr<bool> isAlive = std::make_shared<bool>(true);
    std::optional<T> value;
    bool isDone = false;

    Task get_return_object() {
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_never initial_suspend() noexcept {
      return {};
    }

    struct FinalAwaiter {
      bool await_ready() noexcept { return false; }
      std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept {
        auto &promise = h.promise();
        promise.isDone = true;
        if (promise.continuation) {
          return promise.continuation;
        }
        return std::noop_coroutine();
      }
      void await_resume() noexcept {}
    };

    FinalAwaiter final_suspend() noexcept {
      return {};
    }

    void return_value(T val) {
      value = std::move(val);
      isDone = true;
    }

    void unhandled_exception() {}
  };

  using Handle = std::coroutine_handle<promise_type>;

  Task() : m_handle(nullptr) {}
  explicit Task(Handle h) : m_handle(h) {}
  ~Task() {
    if (m_handle && m_handle.done()) {
      m_handle.destroy();
    }
  }

  Task(const Task &) = delete;
  Task &operator=(const Task &) = delete;

  Task(Task &&other) noexcept : m_handle(other.m_handle) {
    other.m_handle = nullptr;
  }

  Task &operator=(Task &&other) noexcept {
    if (this != &other) {
      if (m_handle && m_handle.done()) {
        m_handle.destroy();
      }
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  bool await_ready() const noexcept {
    return !m_handle || m_handle.done();
  }

  void await_suspend(std::coroutine_handle<> cont) noexcept {
    if (m_handle) {
      m_handle.promise().continuation = cont;
    }
  }

  T await_resume() {
    if (m_handle && m_handle.promise().value) {
      return std::move(*m_handle.promise().value);
    }
    return T{};
  }

  bool isDone() const {
    return !m_handle || m_handle.done();
  }

  void cancel() {
    if (m_handle && m_handle.promise().isAlive) {
      *m_handle.promise().isAlive = false;
    }
  }

private:
  Handle m_handle = nullptr;
};

// =============================================================================
// 4. BUILT-IN GAMEPLAY AWAITERS
// =============================================================================

// Suspends execution for specified duration in seconds: 'await WaitForSeconds(1.5f);'
struct WaitForSeconds {
  float duration = 0.0f;
  explicit WaitForSeconds(float seconds) : duration(std::max(0.0f, seconds)) {}

  bool await_ready() const noexcept { return duration <= 0.0f; }
  void await_suspend(std::coroutine_handle<> handle) const {
    CoroutineScheduler::get().addTimer(duration, handle);
  }
  void await_resume() const noexcept {}
};

// Suspends until the next variable frame cycle: 'await NextFrame{};'
struct NextFrame {
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> handle) const {
    CoroutineScheduler::get().addNextFrame(handle);
  }
  void await_resume() const noexcept {}
};

using WaitForNextFrame = NextFrame;

// Suspends until the next fixed physics tick: 'await NextPhysicsTick{};'
struct NextPhysicsTick {
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> handle) const {
    CoroutineScheduler::get().addNextPhysicsTick(handle);
  }
  void await_resume() const noexcept {}
};

using WaitForPhysicsTick = NextPhysicsTick;

// Suspends until a condition becomes true: 'await WaitUntil([]() { return player->hp <= 0; });'
struct WaitUntil {
  std::function<bool()> predicate;
  explicit WaitUntil(std::function<bool()> pred) : predicate(std::move(pred)) {}

  bool await_ready() const {
    return predicate ? predicate() : true;
  }
  void await_suspend(std::coroutine_handle<> handle) const {
    CoroutineScheduler::get().addCondition(predicate, handle);
  }
  void await_resume() const noexcept {}
};

// Suspends while a condition is true (resumes once false): 'await WaitWhile([]() { return isBusy; });'
struct WaitWhile {
  std::function<bool()> predicate;
  explicit WaitWhile(std::function<bool()> pred) : predicate(std::move(pred)) {}

  bool await_ready() const {
    return predicate ? !predicate() : true;
  }
  void await_suspend(std::coroutine_handle<> handle) const {
    auto pred = predicate;
    CoroutineScheduler::get().addCondition([pred]() { return pred ? !pred() : true; }, handle);
  }
  void await_resume() const noexcept {}
};
