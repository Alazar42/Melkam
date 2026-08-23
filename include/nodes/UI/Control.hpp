#pragma once

#include "core/Memory.hpp"
#include "core/Node.hpp"
#include "core/Signal.hpp"
#include "helper/Rect2.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "input.hpp"
#include "nodes/UI/StyleBox.hpp"
#include "nodes/UI/Theme.hpp"
#include "renderers/Renderer2D.hpp"
#include "window.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

// Container sizing behavior flags (inspired by Godot Control.SizeFlags)
enum class SizeFlags {
  ShrinkBegin = 0,
  Fill = 1,
  Expand = 2,
  ExpandFill = 3,
  ShrinkCenter = 4,
  ShrinkEnd = 8
};

inline SizeFlags operator|(SizeFlags a, SizeFlags b) {
  return static_cast<SizeFlags>(static_cast<int>(a) | static_cast<int>(b));
}
inline SizeFlags operator&(SizeFlags a, SizeFlags b) {
  return static_cast<SizeFlags>(static_cast<int>(a) & static_cast<int>(b));
}

// Focus Mode for keyboard and gamepad navigation
enum class FocusMode {
  None = 0,
  Click = 1,
  All = 2
};

// Base UI Node (inspired by Godot Control) rendering in screen space on the Canvas layer.
class Control : public Node {
public:
  // Signals
  Signal<> resized;
  Signal<> mouse_entered;
  Signal<> mouse_exited;
  Signal<> focus_entered;
  Signal<> focus_exited;
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

  // Container Size Flags
  SizeFlags sizeFlagsHorizontal = SizeFlags::Fill;
  SizeFlags sizeFlagsVertical = SizeFlags::Fill;
  float sizeFlagsStretchRatio = 1.0f;

  // Input, Focus & Theming
  MouseFilter mouseFilter = MouseFilter::Stop;
  FocusMode focusMode = FocusMode::None;
  bool clipContents = false;
  std::string tooltipText;
  Ref<Theme> theme = nullptr;

  Control() : Node("Control") {}
  explicit Control(std::string nodeName) : Node(std::move(nodeName)) {}
  ~Control() override {
    if (s_focusedControl == this) s_focusedControl = nullptr;
    removeModalOverlay(this);
  }

  void onDestroy() override {
    if (s_focusedControl == this) s_focusedControl = nullptr;
    removeModalOverlay(this);
  }

  // Focus Management
  void grabFocus() {
    if (focusMode == FocusMode::None) return;
    if (s_focusedControl != this) {
      if (s_focusedControl) s_focusedControl->focus_exited.emit();
      s_focusedControl = this;
      focus_entered.emit();
    }
  }

  void releaseFocus() {
    if (s_focusedControl == this) {
      s_focusedControl = nullptr;
      focus_exited.emit();
    }
  }

  bool hasFocus() const {
    return s_focusedControl == this;
  }

  static Control *getFocusedControl() {
    return s_focusedControl;
  }

  // Tooltip
  void setTooltipText(std::string text) { tooltipText = std::move(text); }
  const std::string &getTooltipText() const { return tooltipText; }

  // Theme Overrides
  void addThemeColorOverride(const std::string &name, const Color &color) {
    m_overrideColors[name] = color;
  }

  void addThemeConstantOverride(const std::string &name, int value) {
    m_overrideConstants[name] = value;
  }

  void addThemeFontOverride(const std::string &name, Ref<Font> font) {
    m_overrideFonts[name] = std::move(font);
  }

  void addThemeFontSizeOverride(const std::string &name, int size) {
    m_overrideFontSizes[name] = size;
  }

  void addThemeStyleboxOverride(const std::string &name, Ref<StyleBox> styleBox) {
    m_overrideStyleBoxes[name] = std::move(styleBox);
  }

  // Resolves active theme (searches local theme, parent tree, or global default theme)
  Ref<Theme> getTheme() const {
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
    auto it = m_overrideColors.find(name);
    if (it != m_overrideColors.end()) return it->second;

    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getColor(name, nodeType.empty() ? name : nodeType, fallback)
                       : fallback;
  }

  Ref<Font> getThemeFont(const std::string &name = "font",
                         const std::string &nodeType = "") const {
    auto it = m_overrideFonts.find(name);
    if (it != m_overrideFonts.end()) return it->second;

    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getFont(name, nodeType) : Font::getDefaultFont();
  }

  int getThemeFontSize(const std::string &name = "font_size", const std::string &nodeType = "",
                       int fallback = 16) const {
    auto it = m_overrideFontSizes.find(name);
    if (it != m_overrideFontSizes.end()) return it->second;

    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getFontSize(name, nodeType, fallback) : fallback;
  }

  int getThemeConstant(const std::string &name, const std::string &nodeType = "",
                       int fallback = 0) const {
    auto it = m_overrideConstants.find(name);
    if (it != m_overrideConstants.end()) return it->second;

    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getConstant(name, nodeType, fallback) : fallback;
  }

  Ref<Texture2D> getThemeTexture(const std::string &name = "texture",
                                 const std::string &nodeType = "") const {
    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getTexture(name, nodeType) : nullptr;
  }

  Ref<StyleBox> getThemeStylebox(const std::string &name, const std::string &nodeType = "") const {
    auto it = m_overrideStyleBoxes.find(name);
    if (it != m_overrideStyleBoxes.end()) return it->second;

    auto activeTheme = getTheme();
    return activeTheme ? activeTheme->getStyleBox(name, nodeType) : nullptr;
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

  // Sizing & Positioning
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
    if (!visible) return;

    // 1. Keyboard event: route directly to focused control
    if (event.type == InputEventType::Key) {
      if (hasFocus()) {
        gui_input.emit(event);
        onGuiInput(event);
      }
      return;
    }

    if (mouseFilter == MouseFilter::Ignore) return;

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

    if (event.type == InputEventType::MouseButton && event.isPressed()) {
      if (!inside && hasFocus()) {
        releaseFocus();
      }
    }

    if (inside) {
      gui_input.emit(event);
      onGuiInput(event);
      if (mouseFilter == MouseFilter::Stop && event.type == InputEventType::MouseButton) {
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

  static void clearAllOverlays() {
    s_overlays.clear();
  }

  static bool hasActiveOverlay() {
    return !s_overlays.empty();
  }

  static bool processOverlayInput(const InputEvent &event) {
    auto overlaysCopy = s_overlays;
    for (auto it = overlaysCopy.rbegin(); it != overlaysCopy.rend(); ++it) {
      bool stillPresent = std::any_of(s_overlays.begin(), s_overlays.end(),
                                      [&](const OverlayItem &o) { return o.owner == it->owner; });
      if (stillPresent && it->inputCallback && it->inputCallback(event)) {
        return true;
      }
    }
    return false;
  }

  static void renderOverlays() {
    auto overlaysCopy = s_overlays;
    for (auto &item : overlaysCopy) {
      bool stillPresent = std::any_of(s_overlays.begin(), s_overlays.end(),
                                      [&](const OverlayItem &o) { return o.owner == item.owner; });
      if (stillPresent && item.drawCallback) {
        item.drawCallback();
      }
    }
  }


protected:
  bool m_isHovered = false;
  std::unordered_map<std::string, Color> m_overrideColors;
  std::unordered_map<std::string, int> m_overrideConstants;
  std::unordered_map<std::string, Ref<Font>> m_overrideFonts;
  std::unordered_map<std::string, int> m_overrideFontSizes;
  std::unordered_map<std::string, Ref<StyleBox>> m_overrideStyleBoxes;

  inline static Control *s_focusedControl = nullptr;
  inline static std::vector<OverlayItem> s_overlays;
};
