#pragma once

#include "core/Node.hpp"
#include "helper/vectors/Vector2.hpp"
#include <algorithm>
#include <memory>
#include <string>

// Canvas Layer Node (inspired by Godot CanvasLayer) for independent UI/HUD rendering on top of 2D/3D scenes.
class CanvasLayer : public Node {
public:
  int layer = 1;              // Rendering order (higher layers draw on top)
  bool visible = true;        // Controls layer visibility
  Vector2 offset{0.0f, 0.0f}; // Layer screen offset
  float rotation = 0.0f;      // Layer rotation
  Vector2 scale{1.0f, 1.0f};  // Layer scale

  CanvasLayer() : Node("CanvasLayer") {}
  explicit CanvasLayer(int layerIndex, std::string layerName = "CanvasLayer")
      : Node(std::move(layerName)), layer(layerIndex) {}
};
