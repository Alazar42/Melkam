#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "nodes/UI/StyleBox.hpp"
#include "renderers/Font.hpp"
#include "renderers/Renderer2D.hpp"
#include <algorithm>
#include <memory>
#include <string>

// Floating, Draggable GUI Modal/Window Node (inspired by Godot Window & PopupPanel)
class UIWindow : public Control {
public:
  // Signals
  Signal<> close_requested;

  std::string title = "Window";
  float titleBarHeight = 32.0f;
  bool draggable = true;
  bool closable = true;
  bool exclusive = false; // Modal backdrop

  Ref<Font> font = nullptr;
  float fontSize = 0.0f;

  Color titleBarColor = Color::from_rgba8(35, 40, 56);
  Color bodyColor = Color::from_rgba8(24, 28, 38, 245);
  Color borderColor = Color::from_rgba8(75, 85, 115);
  Color fontColor = Color::WHITE;
  Color closeButtonHoverColor = Color::from_rgba8(230, 60, 60);

  UIWindow() : Control("UIWindow") {
    customMinimumSize = Vector2(300.0f, 200.0f);
    mouseFilter = MouseFilter::Stop;
    visible = false;
  }

  explicit UIWindow(std::string windowTitle)
      : Control("UIWindow"), title(std::move(windowTitle)) {
    customMinimumSize = Vector2(300.0f, 200.0f);
    mouseFilter = MouseFilter::Stop;
    visible = false;
  }

  ~UIWindow() override {
    hideWindow();
  }

  void popupCentered(const Vector2 &size = Vector2(400.0f, 260.0f)) {
    Vector2 vp = Window::getViewportSize();
    float x = (vp.x - size.x) * 0.5f;
    float y = (vp.y - size.y) * 0.5f;
    setPosition(Vector2(x, y));
    setSize(size);
    visible = true;
    m_isOpen = true;
    grabFocus();

    Control::setModalOverlay(
        this,
        [this]() { drawOverlay(); },
        [this](const InputEvent &event) -> bool { return handleOverlayInput(event); });
  }

  void hideWindow() {
    if (visible || m_isOpen) {
      visible = false;
      m_isOpen = false;
      m_isDragging = false;
      Control::removeModalOverlay(this);
      releaseFocus();
      close_requested.emit();
    }
  }

  void onDraw() override {
    if (!m_isOpen && visible) {
      drawControl();
    }
  }

  virtual void drawOverlay() {
    if (!visible && !m_isOpen) return;
    drawControl();
  }

  virtual bool handleOverlayInput(const InputEvent &event) {
    if (!visible && !m_isOpen) return false;

    Rect2 rect = getGlobalRect();
    Rect2 titleRect(rect.position.x, rect.position.y, rect.size.x, titleBarHeight);
    Rect2 closeRect(rect.position.x + rect.size.x - 28.0f, rect.position.y + 4.0f, 24.0f, 24.0f);

    Vector2 mousePos = Input::getMousePosition();

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left) {
      if (event.isPressed()) {
        if (closable && closeRect.hasPoint(mousePos)) {
          hideWindow();
          return true;
        }

        if (draggable && titleRect.hasPoint(mousePos)) {
          m_isDragging = true;
          m_dragMouseStart = mousePos;
          m_dragWindowStart = getPosition();
          return true;
        }

        if (rect.hasPoint(mousePos)) {
          onGuiInput(event);
          return true;
        }

        if (exclusive) {
          return true;
        }
      } else {
        if (m_isDragging) {
          m_isDragging = false;
          return true;
        }
        if (rect.hasPoint(mousePos) || exclusive) {
          onGuiInput(event);
          return true;
        }
      }
    } else if (event.type == InputEventType::MouseMotion) {
      if (m_isDragging) {
        Vector2 delta = mousePos - m_dragMouseStart;
        setPosition(m_dragWindowStart + delta);
        return true;
      }
      if (rect.hasPoint(mousePos)) {
        onGuiInput(event);
        return true;
      }
      if (exclusive) {
        return true;
      }
    } else if (event.type == InputEventType::Key && event.isPressed()) {
      if (event.key == Key::Escape) {
        hideWindow();
        return true;
      }
      onGuiInput(event);
      return exclusive;
    }

    return exclusive;
  }

  void onGuiInput(const InputEvent &event) override {
    (void)event;
  }

  void drawControl() override {
    if (!visible && !m_isOpen) return;

    Rect2 rect = getGlobalRect();
    Vector2 mousePos = Input::getMousePosition();

    // 1. Modal backdrop if exclusive
    if (exclusive) {
      Vector2 vp = Window::getViewportSize();
      Renderer2D::drawRectScreen(Vector2(0.0f, 0.0f), vp, Color::from_rgba8(0, 0, 0, 140), true);
    }

    // 2. Window Shadow & Body Frame
    Renderer2D::drawRoundedRectScreen(rect.position + Vector2(3.0f, 4.0f), rect.size, 6.0f,
                                      Color::from_rgba8(0, 0, 0, 120));
    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, 6.0f,
                                      bodyColor, borderColor, 1.5f);

    // 3. Title Bar
    Rect2 titleRect(rect.position.x, rect.position.y, rect.size.x, titleBarHeight);
    Renderer2D::drawRoundedRectScreen(titleRect.position, titleRect.size, 6.0f,
                                      titleBarColor);
    Renderer2D::drawRectScreen(Vector2(rect.position.x, rect.position.y + titleBarHeight - 1.0f),
                              Vector2(rect.size.x, 1.0f), borderColor, true);

    // Title Text
    Ref<Font> activeFont = font ? font : getThemeFont("font", "UIWindow");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "UIWindow", 16));

    float textX = rect.position.x + 12.0f;
    float textY = rect.position.y + (titleBarHeight - activeSize) * 0.5f;
    Renderer2D::drawText(title, Vector2(textX, textY), fontColor, activeSize, activeFont);

    // Close Button
    if (closable) {
      Rect2 closeRect(rect.position.x + rect.size.x - 26.0f, rect.position.y + (titleBarHeight - 20.0f) * 0.5f, 20.0f, 20.0f);
      bool isCloseHover = closeRect.hasPoint(mousePos);
      Color cbCol = isCloseHover ? closeButtonHoverColor : Color::from_rgba8(60, 65, 85);
      Renderer2D::drawRoundedRectScreen(closeRect.position, closeRect.size, 3.0f, cbCol);
      Renderer2D::drawText("x", Vector2(closeRect.position.x + 6.0f, closeRect.position.y + 1.0f), Color::WHITE, 13.0f, activeFont);
    }
  }

protected:
  bool m_isOpen = false;
  bool m_isDragging = false;
  Vector2 m_dragMouseStart{0.0f, 0.0f};
  Vector2 m_dragWindowStart{0.0f, 0.0f};
};

using PopupPanel = UIWindow;
using WindowControl = UIWindow;
