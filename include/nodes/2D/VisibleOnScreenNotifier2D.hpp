#pragma once

#include "core/Signal.hpp"
#include "helper/Rect2.hpp"
#include "nodes/2D/Camera2D.hpp"
#include "nodes/2D/Node2D.hpp"
#include "window.hpp"

// Viewport Visibility Detector Node (inspired by Godot VisibleOnScreenNotifier2D)
class VisibleOnScreenNotifier2D : public Node2D {
public:
  Signal<> screen_entered;
  Signal<> screen_exited;

  Rect2 rect{-16.0f, -16.0f, 32.0f, 32.0f};

  VisibleOnScreenNotifier2D() : Node2D("VisibleOnScreenNotifier2D") {}

  explicit VisibleOnScreenNotifier2D(const Rect2 &boundingBox)
      : Node2D("VisibleOnScreenNotifier2D"), rect(boundingBox) {}

  bool isOnScreen() const {
    return m_isOnScreen;
  }

  void onProcess(float delta) override {
    (void)delta;
    checkVisibility();
  }

  void checkVisibility() {
    Transform2D globalTrans = getGlobalTransform();
    Vector2 worldPos = globalTrans.transformPoint(rect.position);
    Vector2 worldSize = rect.size * globalTrans.scale;

    Vector2 screenPos = Renderer2D::toScreen(worldPos);
    Vector2 screenSize = worldSize * (Camera2D::getCurrent() ? Camera2D::getCurrent()->zoom : 1.0f);
    Rect2 screenRect(screenPos.x, screenPos.y, screenSize.x, screenSize.y);

    Vector2 vpSize = Window::getViewportSize();
    Rect2 vpRect(0.0f, 0.0f, vpSize.x, vpSize.y);

    bool currentlyOnScreen = vpRect.intersects(screenRect);

    if (currentlyOnScreen && !m_isOnScreen) {
      m_isOnScreen = true;
      screen_entered.emit();
    } else if (!currentlyOnScreen && m_isOnScreen) {
      m_isOnScreen = false;
      screen_exited.emit();
    }
  }

private:
  bool m_isOnScreen = false;
};

using VisibilityNotifier2D = VisibleOnScreenNotifier2D;
