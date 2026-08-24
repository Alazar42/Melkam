#pragma once

#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

// Forward declaration for Time query
class Time;

// Individual event stamped with the exact engine frame number and timing
struct EventRecord {
  uint64_t frame = 0;       // Simulation / Engine frame number
  double timestamp = 0.0;   // Elapsed seconds since startup
  std::string eventName;    // Signal or event name (e.g. "player_died", "damage_taken")
  std::string source;       // Emitting node / entity name
  std::string details;      // Payload or debugging context
  size_t receiverCount = 0; // Number of connected slots invoked
};

// High-performance circular Event Journal & Debug Tracer.
// Stamping every event with the frame number allows immediate post-mortem root-cause analysis
// (e.g. tracing the exact sequence of events in the frames leading up to an entity death).
class EventTracer {
public:
  // Records an event into the circular debug journal stamped with the current frame
  static void record(const std::string &eventName,
                     const std::string &source = "",
                     const std::string &details = "",
                     size_t receiverCount = 0,
                     uint64_t explicitFrame = 0,
                     double explicitTimestamp = 0.0) {
    if (!s_enabled) return;

    std::lock_guard<std::mutex> lock(s_mutex);

    EventRecord rec;
    rec.frame = explicitFrame;
    rec.timestamp = explicitTimestamp;
    rec.eventName = eventName;
    rec.source = source;
    rec.details = details;
    rec.receiverCount = receiverCount;

    s_history.push_back(std::move(rec));
    if (s_history.size() > s_maxHistorySize) {
      s_history.pop_front();
    }
  }

  // Dumps the most recent N events formatted with frame stamps to the given output stream
  static void dumpRecent(size_t maxCount = 25, std::ostream &os = std::cout) {
    std::lock_guard<std::mutex> lock(s_mutex);

    os << "\n=================================================================\n";
    os << " [EventTracer] Recent Event History (Last " << std::min(maxCount, s_history.size()) << " Events)\n";
    os << "=================================================================\n";

    if (s_history.empty()) {
      os << "  (No events recorded)\n";
      os << "=================================================================\n\n";
      return;
    }

    size_t startIdx = (s_history.size() > maxCount) ? (s_history.size() - maxCount) : 0;
    for (size_t i = startIdx; i < s_history.size(); ++i) {
      const auto &e = s_history[i];
      os << " [Frame #" << std::setw(6) << std::left << e.frame << " @ "
         << std::fixed << std::setprecision(3) << e.timestamp << "s] "
         << "Event: \"" << e.eventName << "\"";
      if (!e.source.empty()) {
        os << " | Source: [" << e.source << "]";
      }
      if (e.receiverCount > 0) {
        os << " | (" << e.receiverCount << " slot" << (e.receiverCount == 1 ? "" : "s") << ")";
      }
      if (!e.details.empty()) {
        os << " -> " << e.details;
      }
      os << "\n";
    }
    os << "=================================================================\n\n";
  }

  // Returns all events stamped with a specific frame number
  static std::vector<EventRecord> getEventsForFrame(uint64_t frame) {
    std::lock_guard<std::mutex> lock(s_mutex);
    std::vector<EventRecord> results;
    for (const auto &e : s_history) {
      if (e.frame == frame) results.push_back(e);
    }
    return results;
  }

  // Clears the event history journal
  static void clear() {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_history.clear();
  }

  static void setEnabled(bool enabled) { s_enabled = enabled; }
  static bool isEnabled() { return s_enabled; }

  static void setMaxHistorySize(size_t size) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_maxHistorySize = std::max<size_t>(10, size);
  }

private:
  inline static std::deque<EventRecord> s_history;
  inline static size_t s_maxHistorySize = 1000;
  inline static bool s_enabled = true;
  inline static std::mutex s_mutex;
};
