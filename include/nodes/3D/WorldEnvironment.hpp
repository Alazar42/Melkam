#pragma once

#include "core/Node.hpp"
#include "helper/color/Color.hpp"
#include <memory>

enum class EnvironmentBGMode {
  ClearColor,
  Color,
  Sky
};

enum class TonemapMode3D {
  Linear,
  Reinhard,
  ACES
};

// Global Environment Settings Resource (Sky, Ambient, Tonemap, Fog)
class Environment {
public:
  EnvironmentBGMode backgroundMode = EnvironmentBGMode::ClearColor;
  Color backgroundColor = Color::from_rgba8(18, 22, 34);

  Color ambientLightColor = Color::from_rgba8(50, 55, 75);
  float ambientLightEnergy = 1.0f;

  TonemapMode3D tonemapMode = TonemapMode3D::ACES;
  float tonemapExposure = 1.0f;
  float tonemapWhite = 1.0f;

  // Godot 4 Standard Glow / Bloom Post-Processing
  bool glowEnabled = true;
  float glowIntensity = 0.85f;
  float glowThreshold = 0.70f;
  float glowBloom = 0.6f;

  bool fogEnabled = false;
  Color fogColor = Color::from_rgba8(120, 140, 180);
  float fogDensity = 0.01f;
  float fogDepthBegin = 10.0f;
  float fogDepthEnd = 100.0f;

  static std::shared_ptr<Environment> create() {
    return std::make_shared<Environment>();
  }
};

// World Environment Node (inspired by Godot WorldEnvironment)
class WorldEnvironment : public Node {
public:
  inline static const Environment *s_current = nullptr;
  Ref<Environment> environment = nullptr;

  WorldEnvironment() : Node("WorldEnvironment") {
    environment = Environment::create();
    s_current = environment.get();
  }

  explicit WorldEnvironment(Ref<Environment> env) : Node("WorldEnvironment"), environment(std::move(env)) {
    s_current = environment.get();
  }

  void onReady() override {
    if (environment) {
      s_current = environment.get();
    }
  }

  void setEnvironment(const Ref<Environment> &env) {
    environment = env;
    s_current = environment.get();
  }

  Ref<Environment> getEnvironment() const { return environment; }

  static const Environment *getCurrent() { return s_current; }
};
