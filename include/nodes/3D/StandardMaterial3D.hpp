#pragma once

#include "helper/color/Color.hpp"
#include "renderers/Texture2D.hpp"
#include <memory>

// Godot-Standard 3D Material Base & PBR Standard Material
enum class CullMode3D {
  Back,
  Front,
  Disabled
};

enum class ShadingMode3D {
  Shaded,
  Unshaded
};

enum class TransparencyMode3D {
  Opaque,
  Alpha,
  AlphaScissor
};

class Material3D {
public:
  virtual ~Material3D() = default;
  virtual Color getAlbedoColor() const = 0;
  virtual CullMode3D getCullMode() const = 0;
};

// Godot 4-style StandardMaterial3D (Albedo, Metallic, Roughness, Specular, Emission, Textures)
class StandardMaterial3D : public Material3D {
public:
  Color albedoColor = Color::WHITE;
  Ref<Texture2D> albedoTexture = nullptr;
  Vector2 uvScale = Vector2(1.0f, 1.0f);
  Vector2 uvOffset = Vector2(0.0f, 0.0f);

  float roughness = 0.5f;
  float metallic = 0.0f;
  float specular = 0.5f;

  Color emissionColor = Color::BLACK;
  float emissionEnergy = 0.0f;
  Ref<Texture2D> emissionTexture = nullptr;

  CullMode3D cullMode = CullMode3D::Back;
  ShadingMode3D shadingMode = ShadingMode3D::Shaded;
  TransparencyMode3D transparency = TransparencyMode3D::Opaque;
  float alphaScissorThreshold = 0.5f;

  StandardMaterial3D() = default;
  explicit StandardMaterial3D(const Color &color) : albedoColor(color) {}

  void setTexture(const Ref<Texture2D> &tex) { albedoTexture = tex; }
  Ref<Texture2D> getTexture() const { return albedoTexture; }
  void setUvScale(const Vector2 &scale) { uvScale = scale; }
  Vector2 getUvScale() const { return uvScale; }

  Color getAlbedoColor() const override { return albedoColor; }
  CullMode3D getCullMode() const override { return cullMode; }

  static std::shared_ptr<StandardMaterial3D> create(const Color &color = Color::WHITE) {
    return std::make_shared<StandardMaterial3D>(color);
  }
};
