#pragma once

#include "components/Components3D.hpp"
#include "nodes/3D/Light3D.hpp"

// 3D Omni Light Node (Point Light radiating in all directions)
class OmniLight3D : public Light3D {
public:
  float omniRange = 15.0f;
  float omniAttenuation = 1.0f;
  float &lightRange = omniRange;
  float &lightAttenuation = omniAttenuation;

  OmniLight3D() : Light3D("OmniLight3D") {
    initLightECS();
  }

  void onProcess(float delta) override {
    (void)delta;
    if (m_entity.isValid()) {
      auto &comp = m_entity.getOrAddComponent<PointLight3DComponent>();
      comp.color = lightColor;
      comp.energy = lightEnergy;
      comp.range = omniRange;
      comp.attenuation = omniAttenuation;
    }
  }

  void setRange(float range) { omniRange = range; }
  float getRange() const { return omniRange; }

  void setAttenuation(float atten) { omniAttenuation = atten; }
  float getAttenuation() const { return omniAttenuation; }

private:
  void initLightECS() {
    auto &comp = m_entity.getOrAddComponent<PointLight3DComponent>();
    comp.color = lightColor;
    comp.energy = lightEnergy;
    comp.range = omniRange;
    comp.attenuation = omniAttenuation;
  }
};

// Backward-compatible alias matching standard Godot 3 / engine nomenclature
using PointLight3D = OmniLight3D;
