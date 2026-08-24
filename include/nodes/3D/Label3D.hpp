#pragma once

#include "helper/color/Color.hpp"
#include "nodes/3D/GeometryInstance3D.hpp"
#include <string>

enum class BillboardMode3D {
  Disabled,
  Enabled,
  YBillboard
};

// 3D Text Label in World Space (inspired by Godot Label3D)
class Label3D : public GeometryInstance3D {
public:
  std::string text = "Label3D";
  float fontSize = 32.0f;
  float pixelSize = 0.005f;
  Color modulate = Color::WHITE;
  Color outlineModulate = Color::BLACK;
  float outlineSize = 0.0f;
  BillboardMode3D billboardMode = BillboardMode3D::Enabled;

  Label3D() : GeometryInstance3D("Label3D") {}
  explicit Label3D(std::string text) : GeometryInstance3D("Label3D"), text(std::move(text)) {}

  void setText(const std::string &t) { text = t; }
  const std::string &getText() const { return text; }

  void setFontSize(float size) { fontSize = size; }
  float getFontSize() const { return fontSize; }

  void setModulate(const Color &col) { modulate = col; }
  Color getModulate() const { return modulate; }

  void setBillboardMode(BillboardMode3D mode) { billboardMode = mode; }
  BillboardMode3D getBillboardMode() const { return billboardMode; }
};
