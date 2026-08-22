#pragma once

#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Camera2D.hpp"
#include "nodes/2D/MeshInstance2D.hpp"
#include "nodes/2D/Shape2D.hpp"
#include "nodes/2D/Sprite2D.hpp"
#include "nodes/2D/Transform2D.hpp"
#include <string>

// General Entity Tag / Name component.
struct Tag {
  std::string name = "Entity";

  Tag() = default;
  Tag(std::string tagName) : name(std::move(tagName)) {}
};

// General 2D Velocity and physics motion component.
struct Velocity2D {
  Vector2 linear{0.0f, 0.0f};
  float angular = 0.0f; // Radians per second

  Velocity2D() = default;
  Velocity2D(const Vector2 &lin, float ang = 0.0f)
      : linear(lin), angular(ang) {}
};
