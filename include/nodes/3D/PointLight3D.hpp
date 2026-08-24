#pragma once

#include "components/Components3D.hpp"
#include "nodes/3D/Node3D.hpp"

// 3D Omnidirectional Point Light Node
class PointLight3D : public Node3D {
public:
  Color lightColor = Color::WHITE;
  float lightEnergy = 1.0f;
  float lightRange = 10.0f;
  float lightAttenuation = 1.0f;

  PointLight3D() : Node3D("PointLight3D") {
    initLightECS();
  }

  void onProcess(float delta) override {
    (void)delta;
    if (m_entity.isValid()) {
      auto &comp = m_entity.getOrAddComponent<PointLight3DComponent>();
      comp.color = lightColor;
      comp.energy = lightEnergy;
      comp.range = lightRange;
      comp.attenuation = lightAttenuation;
    }
  }

private:
  void initLightECS() {
    auto &comp = m_entity.getOrAddComponent<PointLight3DComponent>();
    comp.color = lightColor;
    comp.energy = lightEnergy;
    comp.range = lightRange;
    comp.attenuation = lightAttenuation;
  }
};
