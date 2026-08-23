#pragma once

#include "core/Node.hpp"
#include "core/Signal.hpp"
#include "helper/Rect2.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "input.hpp"
#include "renderers/Renderer2D.hpp"
#include "nodes/UI/Theme.hpp"
#include "window.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <string>

// Godot-style Layout Presets for Anchors and Offsets
enum class LayoutPreset {
  TopLeft,
  TopRight,
  BottomLeft,
  BottomRight,
  CenterLeft,
  CenterTop,
  CenterRight,
  CenterBottom,
  Center,
  LeftWide,
  TopWide,
  RightWide,
  BottomWide,
  VCenterWide,
  HCenterWide,
  FullRect
};

// Controls how UI controls handle and consume mouse input events
enum class MouseFilter {
  Stop,   // Consumes mouse event, stops propagation
  Pass,   // Processes mouse event and allows underlying nodes to receive it
  Ignore  // Ignores mouse events completely
};

// Base UI Node (inspired by Godot Control) rendering in screen space on the Canvas layer.
class Control : public Node {
public:
  // Signals
  Signal<> resized;
  Signal<> mouse_entered;
  Signal<> mouse_exited;
  Signal<const InputEvent &> gui_input;

  // Anchors (0.0 to 1.0 relative to parent Control or Viewport)
  float anchorLeft = 0.0f;
  float anchorTop = 0.0f;
  float anchorRight = 0.0f;
  float anchorBottom = 0.0f;

  // Offsets / Margins (in pixels)
  float offsetLeft = 0.0f;
  float offsetTop = 0.0f;
  float offsetRight = 0.0f;
  float offsetBottom = 0.0f;

  // Sizing & Transform
  Vector2 customMinimumSize{0.0f, 0.0f};
  Vector2 pivotOffset{0.0f, 0.0f};
  Vector2 scale{1.0f, 1.0f};
  float rotation = 0.0f;
  Color modulate = Color::WHITE;

  // Input, Focus & Theming
  MouseFilter mouseFilter = MouseFilter::Stop;
  bool clipContents = false;
  std::shared_ptr<Theme> theme = nullptr;

  Control() : Node("Control") {}
  explicit Control(std::string nodeName) : Node(std::move(nodeName)) {}

  // Resolves active theme (searches local theme, parent tree, or global default theme)
  std::shared_ptr<Theme> getTheme() const {
    if (theme) return theme;
    Node *curr = getParent();
    while (curr) {
      auto *ctrl = dynamic_cast<Control *>(curr);
      if (ctrl && ctrl->theme) return ctrl->theme;
      curr = curr->getParent();
    }
    return Theme::getDefaultTheme();
  }

  Color getThemeColor(const std::string &name, const std::string &nodeType = "",
                      const Color &fallback = Color::WHITE) const {
    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getColor(name, nodeType.empty() ? name : nodeType, fallback)
                       : fallback;
  }

  std::shared_ptr<Font> getThemeFont(const std::string &name = "font",
                                     const std::string &nodeType = "") const {
    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getFont(name, nodeType) : Font::getDefaultFont();
  }

  int getThemeFontSize(const std::string &name = "font_size", const std::string &nodeType = "",
                       int fallback = 16) const {
    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getFontSize(name, nodeType, fallback) : fallback;
  }

  int getThemeConstant(const std::string &name, const std::string &nodeType = "",
                       int fallback = 0) const {
    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getConstant(name, nodeType, fallback) : fallback;
  }

  std::shared_ptr<Texture2D> getThemeTexture(const std::string &name = "texture",
                                             const std::string &nodeType = "") const {
    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getTexture(name, nodeType) : nullptr;
  }

  // Sets anchors to a Godot Layout Preset
  void setAnchorsPreset(LayoutPreset preset, bool keepOffsets = false) {
    float prevW = getGlobalRect().size.x;
    float prevH = getGlobalRect().size.y;

    switch (preset) {
    case LayoutPreset::TopLeft:
      anchorLeft = 0.0f; anchorTop = 0.0f; anchorRight = 0.0f; anchorBottom = 0.0f;
      break;
    case LayoutPreset::TopRight:
      anchorLeft = 1.0f; anchorTop = 0.0f; anchorRight = 1.0f; anchorBottom = 0.0f;
      break;
    case LayoutPreset::BottomLeft:
      anchorLeft = 0.0f; anchorTop = 1.0f; anchorRight = 0.0f; anchorBottom = 1.0f;
      break;
    case LayoutPreset::BottomRight:
      anchorLeft = 1.0f; anchorTop = 1.0f; anchorRight = 1.0f; anchorBottom = 1.0f;
      break;
    case LayoutPreset::CenterLeft:
      anchorLeft = 0.0f; anchorTop = 0.5f; anchorRight = 0.0f; anchorBottom = 0.5f;
      break;
    case LayoutPreset::CenterTop:
      anchorLeft = 0.5f; anchorTop = 0.0f; anchorRight = 0.5f; anchorBottom = 0.0f;
      break;
    case LayoutPreset::CenterRight:
      anchorLeft = 1.0f; anchorTop = 0.5f; anchorRight = 1.0f; anchorBottom = 0.5f;
      break;
    case LayoutPreset::CenterBottom:
      anchorLeft = 0.5f; anchorTop = 1.0f; anchorRight = 0.5f; anchorBottom = 1.0f;
      break;
    case LayoutPreset::Center:
      anchorLeft = 0.5f; anchorTop = 0.5f; anchorRight = 0.5f; anchorBottom = 0.5f;
      break;
    case LayoutPreset::LeftWide:
      anchorLeft = 0.0f; anchorTop = 0.0f; anchorRight = 0.0f; anchorBottom = 1.0f;
      break;
    case LayoutPreset::TopWide:
      anchorLeft = 0.0f; anchorTop = 0.0f; anchorRight = 1.0f; anchorBottom = 0.0f;
      break;
    case LayoutPreset::RightWide:
      anchorLeft = 1.0f; anchorTop = 0.0f; anchorRight = 1.0f; anchorBottom = 1.0f;
      break;
    case LayoutPreset::BottomWide:
      anchorLeft = 0.0f; anchorTop = 1.0f; anchorRight = 1.0f; anchorBottom = 1.0f;
      break;
    case LayoutPreset::VCenterWide:
      anchorLeft = 0.0f; anchorTop = 0.5f; anchorRight = 1.0f; anchorBottom = 0.5f;
      break;
    case LayoutPreset::HCenterWide:
      anchorLeft = 0.5f; anchorTop = 0.0f; anchorRight = 0.5f; anchorBottom = 1.0f;
      break;
    case LayoutPreset::FullRect:
      anchorLeft = 0.0f; anchorTop = 0.0f; anchorRight = 1.0f; anchorBottom = 1.0f;
      break;
    }

    if (!keepOffsets) {
      if (anchorLeft == anchorRight && anchorTop == anchorBottom) {
        // Point anchor (e.g. Center, TopRight)
        offsetLeft = -prevW * anchorLeft;
        offsetTop = -prevH * anchorTop;
        offsetRight = offsetLeft + prevW;
        offsetBottom = offsetTop + prevH;
      } else {
        offsetLeft = 0.0f;
        offsetTop = 0.0f;
        offsetRight = 0.0f;
        offsetBottom = 0.0f;
      }
    }
  }

  // Returns position of top-left corner relative to parent
  void setPosition(const Vector2 &pos) {
    float w = getGlobalRect().size.x;
    float h = getGlobalRect().size.y;
    offsetLeft = pos.x;
    offsetTop = pos.y;
    offsetRight = pos.x + w;
    offsetBottom = pos.y + h;
  }

  // Returns size of control
  void setSize(const Vector2 &s) {
    offsetRight = offsetLeft + std::max(s.x, customMinimumSize.x);
    offsetBottom = offsetTop + std::max(s.y, customMinimumSize.y);
  }

  Vector2 getPosition() const {
    return {offsetLeft, offsetTop};
  }

  Vector2 getSize() const {
    return getGlobalRect().size;
  }

  // Gets parent's rectangle in screen space
  Rect2 getParentRect() const {
    Node *p = getParent();
    if (p) {
      auto *parentCtrl = dynamic_cast<Control *>(p);
      if (parentCtrl) {
        return parentCtrl->getGlobalRect();
      }
    }
    // Default to full window logical viewport
    Vector2 vpSize = Window::getViewportSize();
    return Rect2(0.0f, 0.0f, vpSize.x, vpSize.y);
  }

  // Computes the absolute screen-space bounding box for this control
  Rect2 getGlobalRect() const {
    Rect2 pRect = getParentRect();

    float left = pRect.position.x + (pRect.size.x * anchorLeft) + offsetLeft;
    float top = pRect.position.y + (pRect.size.y * anchorTop) + offsetTop;
    float right = pRect.position.x + (pRect.size.x * anchorRight) + offsetRight;
    float bottom = pRect.position.y + (pRect.size.y * anchorBottom) + offsetBottom;

    float width = std::max(right - left, customMinimumSize.x);
    float height = std::max(bottom - top, customMinimumSize.y);

    return Rect2(left, top, width, height);
  }

  // Returns true if mouse cursor is currently hovering over this control
  bool isHovered() const {
    return m_isHovered;
  }

  // Handles raw input events routed to GUI
  void onInput(const InputEvent &event) override {
    if (!visible || mouseFilter == MouseFilter::Ignore) return;

    Vector2 mousePos = Input::getMousePosition();
    Rect2 screenRect = getGlobalRect();
    bool inside = screenRect.hasPoint(mousePos);

    if (inside && !m_isHovered) {
      m_isHovered = true;
      mouse_entered.emit();
    } else if (!inside && m_isHovered) {
      m_isHovered = false;
      mouse_exited.emit();
    }

    if (inside) {
      gui_input.emit(event);
      onGuiInput(event);
      if (mouseFilter == MouseFilter::Stop) {
        const_cast<InputEvent &>(event).setHandled();
      }
    }
  }

  // Custom GUI input callback overridden by subclasses (Button, etc.)
  virtual void onGuiInput(const InputEvent &event) {
    (void)event;
  }

  // UI elements render during onDraw() in screen space
  void onDraw() override {
    if (!visible) return;
    drawControl();
  }

  // Subclasses override drawControl to perform screen-space drawing
  virtual void drawControl() {}

  // =========================================================================
  // Top-Level Overlay & Modal Popup Management (Godot PopupMenu / Floating Layer)
  // =========================================================================
  struct OverlayItem {
    void *owner = nullptr;
    std::function<void()> drawCallback;
    std::function<bool(const InputEvent &)> inputCallback;
  };

  static void setModalOverlay(void *owner, std::function<void()> drawCallback,
                              std::function<bool(const InputEvent &)> inputCallback) {
    removeModalOverlay(owner);
    s_overlays.push_back({owner, std::move(drawCallback), std::move(inputCallback)});
  }

  static void removeModalOverlay(void *owner) {
    s_overlays.erase(
        std::remove_if(s_overlays.begin(), s_overlays.end(),
                       [owner](const OverlayItem &item) { return item.owner == owner; }),
        s_overlays.end());
  }

  static bool hasActiveOverlay() {
    return !s_overlays.empty();
  }

  static bool processOverlayInput(const InputEvent &event) {
    for (auto it = s_overlays.rbegin(); it != s_overlays.rend(); ++it) {
      if (it->inputCallback && it->inputCallback(event)) {
        return true;
      }
    }
    return false;
  }

  static void renderOverlays() {
    for (auto &item : s_overlays) {
      if (item.drawCallback) item.drawCallback();
    }
  }

protected:
  bool m_isHovered = false;
  inline static std::vector<OverlayItem> s_overlays;
};
