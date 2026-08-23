#pragma once

#include "nodes/2D/Camera2D.hpp"
#include "nodes/2D/Node2D.hpp"
#include "window.hpp"
#include <cmath>

// 2D Parallax Layer & Background Scrolling Node (inspired by Godot Parallax2D)
class Parallax2D : public Node2D {
public:
  Vector2 scrollScale{1.0f, 1.0f};  // Motion scale relative to Camera2D (e.g. 0.2 = far background)
  Vector2 scrollOffset{0.0f, 0.0f}; // Base static scroll offset
  Vector2 repeatSize{0.0f, 0.0f};   // Repeating grid size for seamless infinite tiling
  Vector2 autoscroll{0.0f, 0.0f};   // Continuous auto-scroll velocity (px/s)
  bool followViewport = true;

  Parallax2D() : Node2D("Parallax2D") {}

  explicit Parallax2D(const Vector2 &scale, const Vector2 &repeat = {0.0f, 0.0f})
      : Node2D("Parallax2D"), scrollScale(scale), repeatSize(repeat) {}

  void onProcess(float delta) override {
    if (!autoscroll.is_zero_approx()) {
      m_autoScrollAccum += autoscroll * delta;
      if (repeatSize.x > 0.0f) m_autoScrollAccum.x = std::fmod(m_autoScrollAccum.x, repeatSize.x);
      if (repeatSize.y > 0.0f) m_autoScrollAccum.y = std::fmod(m_autoScrollAccum.y, repeatSize.y);
    }
  }

  // Computes the parallax offset based on active camera
  Vector2 getParallaxOffset() const {
    const Camera2D *cam = Camera2D::getCurrent();
    Vector2 camPos = cam ? cam->getGlobalPosition() : Vector2(0.0f, 0.0f);
    Vector2 offset = scrollOffset + m_autoScrollAccum;

    if (followViewport) {
      offset += camPos * (Vector2(1.0f, 1.0f) - scrollScale);
    }

    if (repeatSize.x > 0.0f) {
      offset.x = std::fmod(offset.x, repeatSize.x);
      if (offset.x < 0.0f) offset.x += repeatSize.x;
    }
    if (repeatSize.y > 0.0f) {
      offset.y = std::fmod(offset.y, repeatSize.y);
      if (offset.y < 0.0f) offset.y += repeatSize.y;
    }

    return offset;
  }

private:
  Vector2 m_autoScrollAccum{0.0f, 0.0f};
};

using ParallaxLayer = Parallax2D;
using ParallaxBackground = Parallax2D;
