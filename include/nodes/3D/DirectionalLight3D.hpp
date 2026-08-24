#pragma once

#include "components/Components3D.hpp"
#include "nodes/3D/Light3D.hpp"

// 3D Directional Light Node (Sun / Infinite Light source)
class DirectionalLight3D : public Light3D {
public:
  DirectionalLight3D() : Light3D("DirectionalLight3D") {
    initLightECS();
  }

  void onProcess(float delta) override {
    (void)delta;
    if (m_entity.isValid()) {
      auto &comp = m_entity.getOrAddComponent<DirectionalLight3DComponent>();
      comp.color = lightColor;
      comp.energy = lightEnergy;
      comp.castShadows = shadowEnabled;
      comp.shadowBias = shadowBias;
      comp.shadowOpacity = shadowOpacity;
      comp.direction = getGlobalTransform().basis.xform(Vector3(0.0f, 0.0f, -1.0f)).normalized();
    }
  }

private:
  void initLightECS() {
    auto &comp = m_entity.getOrAddComponent<DirectionalLight3DComponent>();
    comp.color = lightColor;
    comp.energy = lightEnergy;
    comp.castShadows = shadowEnabled;
    comp.shadowBias = shadowBias;
    comp.shadowOpacity = shadowOpacity;
  }
};
