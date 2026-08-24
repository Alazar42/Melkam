#pragma once

#include "nodes/3D/VisualInstance3D.hpp"

// Base 3D Light Node in Godot 4
class Light3D : public VisualInstance3D {
public:
  Color lightColor = Color::WHITE;
  float lightEnergy = 1.0f;
  float lightIndirectEnergy = 1.0f;
  float lightSpecular = 0.5f;
  bool shadowEnabled = false;
  float shadowBias = 0.05f;
  float shadowNormalBias = 1.0f;
  float shadowOpacity = 0.75f;
  float shadowBlur = 1.0f;

  Light3D() : VisualInstance3D("Light3D") {}
  explicit Light3D(std::string name) : VisualInstance3D(std::move(name)) {}

  void setColor(const Color &col) { lightColor = col; }
  Color getColor() const { return lightColor; }

  void setEnergy(float energy) { lightEnergy = energy; }
  float getEnergy() const { return lightEnergy; }

  void setShadow(bool enabled) { shadowEnabled = enabled; }
  bool hasShadow() const { return shadowEnabled; }
  void setShadowEnabled(bool enabled) { shadowEnabled = enabled; }
  bool isShadowEnabled() const { return shadowEnabled; }
};
