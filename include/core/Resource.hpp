#pragma once

#include "core/Memory.hpp"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

// Godot-style Base Resource Class
class Resource : public RefCounted {
public:
  std::string resourcePath;
  std::string resourceName;

  Resource() = default;
  explicit Resource(std::string path) : resourcePath(std::move(path)) {}

  virtual ~Resource() = default;

  void setPath(std::string path) { resourcePath = std::move(path); }
  const std::string &getPath() const { return resourcePath; }

  void setName(std::string name) { resourceName = std::move(name); }
  const std::string &getName() const { return resourceName; }

  virtual bool load(const std::string &path) {
    (void)path;
    return true;
  }

  virtual bool save(const std::string &path) const {
    (void)path;
    return true;
  }
};

// Global Resource Loader & Cache Pipeline (inspired by Godot ResourceLoader)
class ResourceLoader {
public:
  template <typename T = Resource>
  static Ref<T> load(const std::string &path, bool noCache = false) {
    std::string resolved = resolvePath(path);
    if (!noCache) {
      auto it = s_cache.find(resolved);
      if (it != s_cache.end()) {
        auto locked = it->second.lock();
        if (locked) {
          return std::dynamic_pointer_cast<T>(locked);
        }
      }
    }

    auto res = makeRef<T>();
    res->setPath(resolved);
    if (res->load(resolved)) {
      if (!noCache) {
        s_cache[resolved] = res;
      }
      return res;
    }
    return nullptr;
  }

  static bool exists(const std::string &path) {
    return std::filesystem::exists(resolvePath(path));
  }

  static void clearCache() {
    s_cache.clear();
  }

  static std::string resolvePath(const std::string &path) {
    std::string clean = path;
    if (clean.rfind("res://", 0) == 0) {
      clean = clean.substr(6);
    }
    if (std::filesystem::exists(clean)) return clean;
    if (std::filesystem::exists("../" + clean)) return "../" + clean;
    if (std::filesystem::exists("../../" + clean)) return "../../" + clean;
    return clean;
  }

private:
  inline static std::unordered_map<std::string, std::weak_ptr<Resource>> s_cache;
};

// Global Resource Saver (inspired by Godot ResourceSaver)
class ResourceSaver {
public:
  static bool save(const Ref<Resource> &resource, const std::string &path) {
    if (!resource) return false;
    return resource->save(path);
  }
};
