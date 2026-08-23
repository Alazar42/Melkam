#pragma once

#include "core/Memory.hpp"
#include "helper/color/Color.hpp"
#include "nodes/UI/StyleBox.hpp"
#include "renderers/Font.hpp"
#include "renderers/Texture2D.hpp"
#include <memory>
#include <string>
#include <unordered_map>

// Godot-style Theme Resource for central UI styling configuration & StyleBox skinning.
class Theme {
public:
  Ref<Font> defaultFont = nullptr;
  int defaultFontSize = 16;

  Theme() = default;

  // =========================================================================
  // Color Items
  // =========================================================================
  void setColor(const std::string &name, const std::string &nodeType, const Color &color) {
    m_colors[makeKey(name, nodeType)] = color;
  }

  bool hasColor(const std::string &name, const std::string &nodeType = "") const {
    return m_colors.find(makeKey(name, nodeType)) != m_colors.end() ||
           (!nodeType.empty() && m_colors.find(makeKey(name, "")) != m_colors.end());
  }

  Color getColor(const std::string &name, const std::string &nodeType = "",
                 const Color &defaultColor = Color::WHITE) const {
    auto it = m_colors.find(makeKey(name, nodeType));
    if (it != m_colors.end()) return it->second;

    if (!nodeType.empty()) {
      it = m_colors.find(makeKey(name, ""));
      if (it != m_colors.end()) return it->second;
    }
    return defaultColor;
  }

  // =========================================================================
  // Font Items
  // =========================================================================
  void setFont(const std::string &name, const std::string &nodeType, Ref<Font> font) {
    m_fonts[makeKey(name, nodeType)] = std::move(font);
  }

  bool hasFont(const std::string &name, const std::string &nodeType = "") const {
    return m_fonts.find(makeKey(name, nodeType)) != m_fonts.end() ||
           (!nodeType.empty() && m_fonts.find(makeKey(name, "")) != m_fonts.end()) ||
           (defaultFont != nullptr);
  }

  Ref<Font> getFont(const std::string &name = "font",
                    const std::string &nodeType = "") const {
    auto it = m_fonts.find(makeKey(name, nodeType));
    if (it != m_fonts.end()) return it->second;

    if (!nodeType.empty()) {
      it = m_fonts.find(makeKey(name, ""));
      if (it != m_fonts.end()) return it->second;
    }
    return defaultFont ? defaultFont : Font::getDefaultFont();
  }

  // =========================================================================
  // Font Size Items
  // =========================================================================
  void setFontSize(const std::string &name, const std::string &nodeType, int size) {
    m_fontSizes[makeKey(name, nodeType)] = size;
  }

  bool hasFontSize(const std::string &name, const std::string &nodeType = "") const {
    return m_fontSizes.find(makeKey(name, nodeType)) != m_fontSizes.end() ||
           (!nodeType.empty() && m_fontSizes.find(makeKey(name, "")) != m_fontSizes.end());
  }

  int getFontSize(const std::string &name = "font_size", const std::string &nodeType = "",
                  int fallbackSize = 16) const {
    auto it = m_fontSizes.find(makeKey(name, nodeType));
    if (it != m_fontSizes.end()) return it->second;

    if (!nodeType.empty()) {
      it = m_fontSizes.find(makeKey(name, ""));
      if (it != m_fontSizes.end()) return it->second;
    }
    return (defaultFontSize > 0) ? defaultFontSize : fallbackSize;
  }

  // =========================================================================
  // Constant Items (Padding, Margin, Separation, Corner Radius)
  // =========================================================================
  void setConstant(const std::string &name, const std::string &nodeType, int value) {
    m_constants[makeKey(name, nodeType)] = value;
  }

  bool hasConstant(const std::string &name, const std::string &nodeType = "") const {
    return m_constants.find(makeKey(name, nodeType)) != m_constants.end() ||
           (!nodeType.empty() && m_constants.find(makeKey(name, "")) != m_constants.end());
  }

  int getConstant(const std::string &name, const std::string &nodeType = "",
                  int fallback = 0) const {
    auto it = m_constants.find(makeKey(name, nodeType));
    if (it != m_constants.end()) return it->second;

    if (!nodeType.empty()) {
      it = m_constants.find(makeKey(name, ""));
      if (it != m_constants.end()) return it->second;
    }
    return fallback;
  }

  // =========================================================================
  // Texture Items (Panel backgrounds, Button skins, Icons)
  // =========================================================================
  void setTexture(const std::string &name, const std::string &nodeType,
                  Ref<Texture2D> texture) {
    m_textures[makeKey(name, nodeType)] = std::move(texture);
  }

  bool hasTexture(const std::string &name, const std::string &nodeType = "") const {
    return m_textures.find(makeKey(name, nodeType)) != m_textures.end() ||
           (!nodeType.empty() && m_textures.find(makeKey(name, "")) != m_textures.end());
  }

  Ref<Texture2D> getTexture(const std::string &name = "texture",
                            const std::string &nodeType = "") const {
    auto it = m_textures.find(makeKey(name, nodeType));
    if (it != m_textures.end()) return it->second;

    if (!nodeType.empty()) {
      it = m_textures.find(makeKey(name, ""));
      if (it != m_textures.end()) return it->second;
    }
    return nullptr;
  }

  // =========================================================================
  // StyleBox Items (Flat backgrounds, Borders, 9-Slices)
  // =========================================================================
  void setStyleBox(const std::string &name, const std::string &nodeType, Ref<StyleBox> styleBox) {
    m_styleBoxes[makeKey(name, nodeType)] = std::move(styleBox);
  }

  bool hasStyleBox(const std::string &name, const std::string &nodeType = "") const {
    return m_styleBoxes.find(makeKey(name, nodeType)) != m_styleBoxes.end() ||
           (!nodeType.empty() && m_styleBoxes.find(makeKey(name, "")) != m_styleBoxes.end());
  }

  Ref<StyleBox> getStyleBox(const std::string &name, const std::string &nodeType = "") const {
    auto it = m_styleBoxes.find(makeKey(name, nodeType));
    if (it != m_styleBoxes.end()) return it->second;

    if (!nodeType.empty()) {
      it = m_styleBoxes.find(makeKey(name, ""));
      if (it != m_styleBoxes.end()) return it->second;
    }
    return nullptr;
  }

  // =========================================================================
  // Global Engine Theme Singletons
  // =========================================================================
  static Ref<Theme> getDefaultTheme() {
    if (!s_defaultTheme) {
      s_defaultTheme = createDefaultDarkTheme();
    }
    return s_defaultTheme;
  }

  static void setDefaultTheme(Ref<Theme> theme) {
    s_defaultTheme = std::move(theme);
  }

  // Default Godot-inspired Modern Dark Theme
  static Ref<Theme> createDefaultDarkTheme() {
    auto theme = makeRef<Theme>();
    theme->defaultFont = Font::getDefaultFont();
    theme->defaultFontSize = 16;

    // Panel
    theme->setColor("bg_color", "Panel", Color::from_rgba8(25, 28, 38, 230));
    theme->setColor("border_color", "Panel", Color::from_rgba8(65, 75, 105));
    theme->setConstant("corner_radius", "Panel", 6);
    theme->setConstant("border_width", "Panel", 1);
    theme->setStyleBox("panel", "Panel", makeRef<StyleBoxFlat>(Color::from_rgba8(25, 28, 38, 230), 6.0f, 1.0f, Color::from_rgba8(65, 75, 105)));

    // Button
    theme->setColor("normal_color", "Button", Color::from_rgba8(45, 52, 75));
    theme->setColor("hover_color", "Button", Color::from_rgba8(65, 80, 120));
    theme->setColor("pressed_color", "Button", Color::from_rgba8(30, 38, 55));
    theme->setColor("disabled_color", "Button", Color::from_rgba8(30, 30, 35));
    theme->setColor("font_color", "Button", Color::WHITE);
    theme->setColor("border_color", "Button", Color::from_rgba8(90, 100, 130));
    theme->setConstant("corner_radius", "Button", 4);
    theme->setConstant("border_width", "Button", 1);

    theme->setStyleBox("normal", "Button", makeRef<StyleBoxFlat>(Color::from_rgba8(45, 52, 75), 4.0f, 1.0f, Color::from_rgba8(90, 100, 130)));
    theme->setStyleBox("hover", "Button", makeRef<StyleBoxFlat>(Color::from_rgba8(65, 80, 120), 4.0f, 1.0f, Color::from_rgba8(130, 150, 200)));
    theme->setStyleBox("pressed", "Button", makeRef<StyleBoxFlat>(Color::from_rgba8(30, 38, 55), 4.0f, 1.0f, Color::from_rgba8(50, 60, 90)));
    theme->setStyleBox("disabled", "Button", makeRef<StyleBoxFlat>(Color::from_rgba8(30, 30, 35), 4.0f, 1.0f, Color::from_rgba8(50, 50, 60)));

    // Label
    theme->setColor("font_color", "Label", Color::WHITE);
    theme->setColor("shadow_color", "Label", Color(0.0f, 0.0f, 0.0f, 0.0f));

    // ProgressBar
    theme->setColor("fill_color", "ProgressBar", Color::from_rgba8(52, 199, 89));
    theme->setColor("bg_color", "ProgressBar", Color::from_rgba8(35, 36, 45));
    theme->setColor("border_color", "ProgressBar", Color::from_rgba8(80, 85, 105));
    theme->setColor("font_color", "ProgressBar", Color::WHITE);
    theme->setConstant("corner_radius", "ProgressBar", 3);
    theme->setConstant("border_width", "ProgressBar", 1);

    // Slider / HSlider / VSlider
    theme->setColor("track_color", "Slider", Color::from_rgba8(45, 48, 60));
    theme->setColor("track_fill_color", "Slider", Color::from_rgba8(66, 135, 245));
    theme->setColor("grabber_color", "Slider", Color::from_rgba8(230, 235, 245));
    theme->setColor("grabber_hover_color", "Slider", Color::WHITE);
    theme->setColor("grabber_border_color", "Slider", Color::from_rgba8(30, 32, 40));

    // OptionButton
    theme->setColor("normal_color", "OptionButton", Color::from_rgba8(45, 48, 62));
    theme->setColor("hover_color", "OptionButton", Color::from_rgba8(60, 65, 85));
    theme->setColor("dropdown_bg_color", "OptionButton", Color::from_rgba8(28, 30, 40, 255));
    theme->setColor("item_hover_color", "OptionButton", Color::from_rgba8(52, 120, 246));
    theme->setColor("font_color", "OptionButton", Color::WHITE);
    theme->setColor("border_color", "OptionButton", Color::from_rgba8(85, 90, 115));
    theme->setConstant("corner_radius", "OptionButton", 4);
    theme->setConstant("border_width", "OptionButton", 1);

    // LineEdit
    theme->setColor("bg_color", "LineEdit", Color::from_rgba8(30, 32, 42));
    theme->setColor("border_color", "LineEdit", Color::from_rgba8(75, 80, 105));
    theme->setColor("focus_border_color", "LineEdit", Color::from_rgba8(52, 120, 246));
    theme->setColor("font_color", "LineEdit", Color::WHITE);
    theme->setColor("placeholder_color", "LineEdit", Color::from_rgba8(120, 125, 145));
    theme->setColor("cursor_color", "LineEdit", Color::WHITE);
    theme->setConstant("corner_radius", "LineEdit", 4);
    theme->setConstant("border_width", "LineEdit", 1);

    // CheckBox & CheckButton
    theme->setColor("box_color", "CheckBox", Color::from_rgba8(35, 38, 48));
    theme->setColor("box_checked_color", "CheckBox", Color::from_rgba8(52, 120, 246));
    theme->setColor("checkmark_color", "CheckBox", Color::WHITE);
    theme->setColor("track_off_color", "CheckButton", Color::from_rgba8(45, 48, 60));
    theme->setColor("track_on_color", "CheckButton", Color::from_rgba8(52, 199, 89));
    theme->setColor("thumb_color", "CheckButton", Color::WHITE);

    // TabContainer
    theme->setColor("tab_bg_color", "TabContainer", Color::from_rgba8(35, 40, 55));
    theme->setColor("tab_active_bg_color", "TabContainer", Color::from_rgba8(55, 65, 95));
    theme->setColor("tab_hover_bg_color", "TabContainer", Color::from_rgba8(45, 52, 75));
    theme->setColor("font_color", "TabContainer", Color::WHITE);

    return theme;
  }

private:
  static std::string makeKey(const std::string &name, const std::string &nodeType) {
    if (nodeType.empty()) return name;
    return nodeType + "/" + name;
  }

  std::unordered_map<std::string, Color> m_colors;
  std::unordered_map<std::string, int> m_constants;
  std::unordered_map<std::string, Ref<Font>> m_fonts;
  std::unordered_map<std::string, int> m_fontSizes;
  std::unordered_map<std::string, Ref<Texture2D>> m_textures;
  std::unordered_map<std::string, Ref<StyleBox>> m_styleBoxes;

  inline static Ref<Theme> s_defaultTheme = nullptr;
};
