#pragma once

#include "nodes/3D/StandardMaterial3D.hpp"
#include "nodes/3D/VisualInstance3D.hpp"

enum class ShadowCastingSetting3D {
  Off,
  On,
  DoubleSided,
  ShadowsOnly
};

// Base class for geometric rendering instances in Godot 4
class GeometryInstance3D : public VisualInstance3D {
public:
  Ref<StandardMaterial3D> materialOverride = nullptr;
  Ref<StandardMaterial3D> materialOverlay = nullptr;
  ShadowCastingSetting3D castShadow = ShadowCastingSetting3D::On;
  float extraCullMargin = 0.0f;
  float transparency = 0.0f;

  GeometryInstance3D() : VisualInstance3D("GeometryInstance3D") {}
  explicit GeometryInstance3D(std::string name) : VisualInstance3D(std::move(name)) {}

  void setMaterialOverride(const Ref<StandardMaterial3D> &mat) {
    materialOverride = mat;
  }
  Ref<StandardMaterial3D> getMaterialOverride() const { return materialOverride; }

  void setCastShadowsSetting(ShadowCastingSetting3D setting) {
    castShadow = setting;
  }
  ShadowCastingSetting3D getCastShadowsSetting() const { return castShadow; }
};
