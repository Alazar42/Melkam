#pragma once

#include "core/EventTracer.hpp"
#include "time.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using ConnectionId = uint64_t;

// Godot-style Type-Safe Signal / Slot System with Frame-Number Stamping & Event Tracing.
// Every emission is stamped with the exact engine frame count and timestamp from day one,
// enabling post-mortem root-cause debugging for entity events and lifecycle changes.
template <typename... Args>
class Signal {
public:
  using SlotType = std::function<void(Args...)>;

  Signal() = default;
  explicit Signal(std::string name, std::string source = "")
      : m_name(std::move(name)), m_source(std::move(source)) {}
  ~Signal() = default;

  // Non-copyable (signals belong to their enclosing object)
  Signal(const Signal &) = delete;
  Signal &operator=(const Signal &) = delete;

  // Moveable
  Signal(Signal &&other) noexcept
      : m_slots(std::move(other.m_slots)),
        m_name(std::move(other.m_name)),
        m_source(std::move(other.m_source)),
        m_lastEmissionFrame(other.m_lastEmissionFrame),
        m_lastEmissionTime(other.m_lastEmissionTime),
        m_emissionCount(other.m_emissionCount) {}

  Signal &operator=(Signal &&other) noexcept {
    if (this != &other) {
      m_slots = std::move(other.m_slots);
      m_name = std::move(other.m_name);
      m_source = std::move(other.m_source);
      m_lastEmissionFrame = other.m_lastEmissionFrame;
      m_lastEmissionTime = other.m_lastEmissionTime;
      m_emissionCount = other.m_emissionCount;
    }
    return *this;
  }

  // Sets debug name and source entity
  void setName(std::string name) { m_name = std::move(name); }
  const std::string &getName() const { return m_name; }

  void setSource(std::string source) { m_source = std::move(source); }
  const std::string &getSource() const { return m_source; }

  // Returns the frame number when this signal was last emitted
  uint64_t getLastEmissionFrame() const { return m_lastEmissionFrame; }

  // Returns the timestamp (seconds) when this signal was last emitted
  double getLastEmissionTime() const { return m_lastEmissionTime; }

  // Returns total number of times this signal has been emitted
  uint64_t getEmissionCount() const { return m_emissionCount; }

  // Connects a lambda or free function slot
  ConnectionId connect(SlotType slot) {
    if (!slot) return 0;
    ConnectionId id = ++s_nextId;
    m_slots.push_back({id, std::move(slot)});
    return id;
  }

  // Connects a member function slot on an object instance
  template <typename T>
  ConnectionId connect(T *instance, void (T::*method)(Args...)) {
    if (!instance || !method) return 0;
    return connect([instance, method](Args... args) {
      (instance->*method)(std::forward<Args>(args)...);
    });
  }

  // Connects a const member function slot on an object instance
  template <typename T>
  ConnectionId connect(const T *instance, void (T::*method)(Args...) const) {
    if (!instance || !method) return 0;
    return connect([instance, method](Args... args) {
      (instance->*method)(std::forward<Args>(args)...);
    });
  }

  // Disconnects a slot by its connection ID
  bool disconnect(ConnectionId id) {
    auto it = std::find_if(m_slots.begin(), m_slots.end(),
                           [id](const SlotEntry &entry) { return entry.id == id; });
    if (it != m_slots.end()) {
      m_slots.erase(it);
      return true;
    }
    return false;
  }

  // Emits the signal, invoking all connected slots and stamping the frame number
  void emit(Args... args) const {
    m_lastEmissionFrame = Time::getFrameCount();
    m_lastEmissionTime = static_cast<double>(Time::getTime());
    m_emissionCount++;

    // Record into the central EventTracer journal if a name is configured or listeners exist
    if (!m_name.empty() || !m_slots.empty()) {
      EventTracer::record(m_name.empty() ? "Signal" : m_name,
                          m_source,
                          "",
                          m_slots.size(),
                          m_lastEmissionFrame,
                          m_lastEmissionTime);
    }

    // Copy slot list before iterating so handlers can safely disconnect during emission
    auto slotsCopy = m_slots;
    for (const auto &entry : slotsCopy) {
      if (entry.slot) {
        entry.slot(args...);
      }
    }
  }

  // Callable operator alias for emit
  void operator()(Args... args) const {
    emit(std::forward<Args>(args)...);
  }

  // Returns true if a specific connection ID is currently registered
  bool isConnected(ConnectionId id) const {
    return std::any_of(m_slots.begin(), m_slots.end(),
                       [id](const SlotEntry &entry) { return entry.id == id; });
  }

  // Returns the number of connected slots
  size_t getConnectionCount() const {
    return m_slots.size();
  }

  // Returns true if at least one slot is connected
  bool hasConnections() const {
    return !m_slots.empty();
  }

  // Clears all connected slots
  void clear() {
    m_slots.clear();
  }

private:
  struct SlotEntry {
    ConnectionId id;
    SlotType slot;
  };

  std::vector<SlotEntry> m_slots;
  std::string m_name;
  std::string m_source;
  mutable uint64_t m_lastEmissionFrame = 0;
  mutable double m_lastEmissionTime = 0.0;
  mutable uint64_t m_emissionCount = 0;

  inline static ConnectionId s_nextId = 0;
};
