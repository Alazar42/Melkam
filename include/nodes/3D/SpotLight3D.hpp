#pragma once

#include "components/Components3D.hpp"
#include "nodes/3D/Node3D.hpp"
#include <cmath>

// 3D Conical Spot Light Node (inspired by Godot SpotLight3D)
class SpotLight3D : public Node3D {
public:
  Color lightColor = Color::WHITE;
  float lightEnergy = 1.0f;
  float spotRange = 15.0f;
  float spotAngle = 45.0f; // Cone angle in degrees
  float spotAngleAttenuation = 1.0f;

  SpotLight3D() : Node3D("SpotLight3D") {
    initLightECS();
  }

  void onProcess(float delta) override {
    (void)delta;
    if (m_entity.isValid()) {
      auto &comp = m_entity.getOrAddComponent<PointLight3DComponent>();
      comp.color = lightColor;
      comp.energy = lightEnergy;
      comp.range = spotRange;
      comp.attenuation = spotAngleAttenuation;
    }
  }

  Vector3 getDirection() const {
    return getGlobalTransform().basis.xform(Vector3(0.0f, 0.0f, -1.0f)).normalized();
  }

private:
  void initLightECS() {
    auto &comp = m_entity.getOrAddComponent<PointLight3DComponent>();
    comp.color = lightColor;
    comp.energy = lightEnergy;
    comp.range = spotRange;
  }
};
