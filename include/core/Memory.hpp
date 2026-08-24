#pragma once

#include <memory>
#include <utility>

// =============================================================================
// MelkamEngine Memory Management & Smart Pointer Aliases
// Inspired by Godot (Ref<T>) & Modern C++ Game Engine Conventions (Scope<T>, Shared<T>)
// =============================================================================

// Shared reference-counted pointer (std::shared_ptr)
template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

// Unique ownership pointer (std::unique_ptr)
template <typename T>
using Scope = std::unique_ptr<T>;

template <typename T>
using Unique = std::unique_ptr<T>;

// Non-owning weak reference pointer (std::weak_ptr)
template <typename T>
using Weak = std::weak_ptr<T>;

template <typename T>
using WeakRef = std::weak_ptr<T>;

// Smart Pointer Creation Helpers
template <typename T, typename... Args>
constexpr Ref<T> makeRef(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr Ref<T> createRef(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr Shared<T> makeShared(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr Shared<T> createShared(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr Scope<T> makeScope(Args &&...args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr Scope<T> createScope(Args &&...args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr Unique<T> makeUnique(Args &&...args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr Unique<T> createUnique(Args &&...args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

// Pointer Casting Helpers
template <typename T, typename U>
constexpr Ref<T> refCast(const Ref<U> &r) {
  return std::static_pointer_cast<T>(r);
}

template <typename T, typename U>
constexpr Ref<T> dynamicRefCast(const Ref<U> &r) {
  return std::dynamic_pointer_cast<T>(r);
}

template <typename T, typename U>
constexpr Ref<T> constRefCast(const Ref<U> &r) {
  return std::const_pointer_cast<T>(r);
}

template <typename T, typename U>
constexpr Shared<T> sharedCast(const Shared<U> &s) {
  return std::static_pointer_cast<T>(s);
}

template <typename T, typename U>
constexpr Shared<T> dynamicSharedCast(const Shared<U> &s) {
  return std::dynamic_pointer_cast<T>(s);
}

// Godot-style RefCounted Base Object
class RefCounted : public std::enable_shared_from_this<RefCounted> {
public:
  virtual ~RefCounted() = default;
};

