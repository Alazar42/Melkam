#pragma once

#include "core/ConfigFile.hpp"
#include <memory>
#include <string>

// Godot-style Project Settings Configuration (project.godot / project.melkam)
class ProjectSettings {
public:
  static ProjectSettings *get_singleton() {
    if (!s_instance) {
      s_instance = new ProjectSettings();
      s_instance->loadDefaultSettings();
    }
    return s_instance;
  }

  static ConfigFile::Value get_setting(const std::string &name, const ConfigFile::Value &defaultVal = std::string("")) {
    return get_singleton()->getSetting(name, defaultVal);
  }

  static void set_setting(const std::string &name, const ConfigFile::Value &value) {
    get_singleton()->setSetting(name, value);
  }

  static void set_setting(const std::string &name, const char *value) {
    get_singleton()->setSetting(name, std::string(value));
  }

  static bool load(const std::string &path = "project.melkam") {
    return get_singleton()->loadSettings(path);
  }

  static bool save(const std::string &path = "project.melkam") {
    return get_singleton()->saveSettings(path);
  }

  ConfigFile::Value getSetting(const std::string &name, const ConfigFile::Value &defaultVal = std::string("")) const {
    auto slash = name.find('/');
    if (slash != std::string::npos) {
      std::string section = name.substr(0, slash);
      std::string key = name.substr(slash + 1);
      return m_config.get_value(section, key, defaultVal);
    }
    return m_config.get_value("general", name, defaultVal);
  }


  void setSetting(const std::string &name, const ConfigFile::Value &val) {
    auto slash = name.find('/');
    if (slash != std::string::npos) {
      std::string section = name.substr(0, slash);
      std::string key = name.substr(slash + 1);
      m_config.set_value(section, key, val);
    } else {
      m_config.set_value("general", name, val);
    }
  }

  void setSetting(const std::string &name, const char *val) {
    setSetting(name, std::string(val));
  }


  bool loadSettings(const std::string &path) {
    m_projectPath = path;
    if (m_config.load(path)) {
      return true;
    }
    // Fallback check for project.godot
    if (path == "project.melkam" && m_config.load("project.godot")) {
      m_projectPath = "project.godot";
      return true;
    }
    return false;
  }

  bool saveSettings(const std::string &path) {
    return m_config.save(path.empty() ? m_projectPath : path);
  }

  std::string getProjectName() const {
    return m_config.getString("application", "config/name", "MelkamEngine Game");
  }

  std::string getMainScene() const {
    return m_config.getString("application", "run/main_scene", "res://scenes/main.tscn");
  }

  uint32_t getViewportWidth() const {
    return static_cast<uint32_t>(m_config.getInt("display", "window/size/viewport_width", 1280));
  }

  uint32_t getViewportHeight() const {
    return static_cast<uint32_t>(m_config.getInt("display", "window/size/viewport_height", 720));
  }

private:
  void loadDefaultSettings() {
    setSetting("application/config/name", "MelkamEngine Game");
    setSetting("application/run/main_scene", "res://scenes/main.tscn");
    setSetting("display/window/size/viewport_width", static_cast<int64_t>(1280));
    setSetting("display/window/size/viewport_height", static_cast<int64_t>(720));
    setSetting("display/window/vsync/vsync_mode", true);
    setSetting("physics/2d/default_gravity", 980.0);
  }

  ConfigFile m_config;
  std::string m_projectPath = "project.melkam";
  inline static ProjectSettings *s_instance = nullptr;
};
