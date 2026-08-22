#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

using ConnectionId = uint64_t;

// Godot-style Type-Safe Signal / Slot System.
template <typename... Args>
class Signal {
public:
  using SlotType = std::function<void(Args...)>;

  Signal() = default;
  ~Signal() = default;

  // Non-copyable (signals belong to their enclosing object)
  Signal(const Signal &) = delete;
  Signal &operator=(const Signal &) = delete;

  // Moveable
  Signal(Signal &&other) noexcept : m_slots(std::move(other.m_slots)) {}
  Signal &operator=(Signal &&other) noexcept {
    if (this != &other) {
      m_slots = std::move(other.m_slots);
    }
    return *this;
  }

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

  // Emits the signal, invoking all connected slots with the given arguments
  void emit(Args... args) const {
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
  inline static ConnectionId s_nextId = 0;
};
