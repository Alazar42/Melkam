#pragma once

#include "core/Memory.hpp"
#include "helper/color/Color.hpp"
#include "nodes/2D/Node2D.hpp"
#include <algorithm>

enum class Light2DBlendMode {
  Add,
  Sub,
  Mix
};

// Base 2D Light Node (inspired by Godot Light2D)
class Light2D : public Node2D {
public:
  bool enabled = true;
  Color color = Color::WHITE;
  float energy = 1.0f;
  Light2DBlendMode blendMode = Light2DBlendMode::Add;
  int rangeZMin = -100;
  int rangeZMax = 100;
  int rangeLayerMin = 0;
  int rangeLayerMax = 0;
  bool shadowsEnabled = false;
  Color shadowColor = Color::from_rgba8(0, 0, 0, 180);

  Light2D() : Node2D("Light2D") {}
  explicit Light2D(std::string nodeName) : Node2D(std::move(nodeName)) {}
};
