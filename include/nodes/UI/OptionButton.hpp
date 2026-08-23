#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Button.hpp"
#include "nodes/UI/Icons.hpp"
#include "renderers/Font.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

struct OptionItem {
  std::string text;
  int id = 0;
  bool disabled = false;
};

// Dropdown Select Menu UI Node (inspired by Godot OptionButton).
class OptionButton : public BaseButton {
public:
  // Signals
  Signal<int> item_selected;

  std::vector<OptionItem> items;
  int selectedIndex = -1;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f; // 0 = inherits from active theme

  // Colors & Theme Styling
  Color normalColor = Color(0, 0, 0, 0);
  Color hoverColor = Color(0, 0, 0, 0);
  Color dropdownBgColor = Color(0, 0, 0, 0);
  Color itemHoverColor = Color(0, 0, 0, 0);
  Color fontColor = Color(0, 0, 0, 0);
  Color borderColor = Color(0, 0, 0, 0);
  float borderWidth = -1.0f;
  float cornerRadius = -1.0f;
  float itemHeight = 32.0f;

  OptionButton() : BaseButton("OptionButton") {
    customMinimumSize = {150.0f, 36.0f};
  }

  ~OptionButton() override {
    closeMenu();
  }

  void addItem(std::string label, int id = -1) {
    if (id == -1) id = static_cast<int>(items.size());
    items.push_back({std::move(label), id, false});
    if (selectedIndex == -1) {
      selectedIndex = 0;
    }
  }

  void select(int index) {
    if (index >= 0 && index < static_cast<int>(items.size())) {
      selectedIndex = index;
      item_selected.emit(selectedIndex);
    }
  }

  int getSelected() const { return selectedIndex; }

  std::string getSelectedText() const {
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size())) {
      return items[selectedIndex].text;
    }
    return "";
  }

  int getSelectedId() const {
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size())) {
      return items[selectedIndex].id;
    }
    return -1;
  }

  int getItemCount() const { return static_cast<int>(items.size()); }

  void clear() {
    closeMenu();
    items.clear();
    selectedIndex = -1;
  }

  void openMenu() {
    if (items.empty()) return;
    m_isMenuOpen = true;

    Rect2 rect = getGlobalRect();
    float dropdownY = rect.position.y + rect.size.y + 2.0f;
    float totalH = items.size() * itemHeight;
    Rect2 menuRect(rect.position.x, dropdownY, rect.size.x, totalH);

    Color dropBg = (dropdownBgColor.a > 0.0f)
                       ? dropdownBgColor
                       : getThemeColor("dropdown_bg_color", "OptionButton", Color::from_rgba8(28, 30, 40, 255));
    Color border = (borderColor.a > 0.0f)
                       ? borderColor
                       : getThemeColor("border_color", "OptionButton", Color::from_rgba8(85, 90, 115));
    Color itemHov = (itemHoverColor.a > 0.0f)
                        ? itemHoverColor
                        : getThemeColor("item_hover_color", "OptionButton", Color::from_rgba8(52, 120, 246));
    Color txtCol = (fontColor.a > 0.0f)
                       ? fontColor
                       : getThemeColor("font_color", "OptionButton", Color::WHITE);
    float cr = (cornerRadius >= 0.0f)
                   ? cornerRadius
                   : static_cast<float>(getThemeConstant("corner_radius", "OptionButton", 4));
    float bw = (borderWidth >= 0.0f)
                   ? borderWidth
                   : static_cast<float>(getThemeConstant("border_width", "OptionButton", 1));
    float activeSize = (fontSize > 0.0f)
                           ? fontSize
                           : static_cast<float>(getThemeFontSize("font_size", "OptionButton", 17));
    Ref<Font> activeFont = font ? font : getThemeFont("font", "OptionButton");

    Control::setModalOverlay(
        this,
        [this, menuRect, dropdownY, dropBg, border, itemHov, txtCol, cr, bw, activeSize, activeFont]() {
          Renderer2D::drawRoundedRectScreen(menuRect.position, menuRect.size, cr,
                                            dropBg * modulate, border * modulate,
                                            bw);

          Vector2 mousePos = Input::getMousePosition();

          for (size_t i = 0; i < items.size(); ++i) {
            float itemY = dropdownY + (i * itemHeight);
            Rect2 itemRect(menuRect.position.x, itemY, menuRect.size.x, itemHeight);

            bool isHover = itemRect.hasPoint(mousePos);
            if (isHover) {
              Renderer2D::drawRoundedRectScreen(Vector2(menuRect.position.x + 2.0f, itemY + 2.0f),
                                                Vector2(menuRect.size.x - 4.0f, itemHeight - 4.0f),
                                                3.0f, itemHov * modulate);
            }

            float textY = itemY + (itemHeight - activeSize) * 0.5f;
            Renderer2D::drawText(items[i].text, Vector2(menuRect.position.x + 12.0f, textY),
                                 txtCol * modulate, activeSize, activeFont);
          }
        },
        [this, menuRect, dropdownY](const InputEvent &ev) -> bool {
          if (ev.type == InputEventType::MouseButton && ev.mouseButton == MouseButton::Left && ev.isPressed()) {
            if (menuRect.hasPoint(ev.mousePosition)) {
              int clickedIdx = static_cast<int>((ev.mousePosition.y - dropdownY) / itemHeight);
              if (clickedIdx >= 0 && clickedIdx < static_cast<int>(items.size())) {
                select(clickedIdx);
              }
            }
            closeMenu();
            return true; // Consumes event, stops underlying clicks
          }
          return false;
        });
  }

  void closeMenu() {
    if (m_isMenuOpen) {
      m_isMenuOpen = false;
      Control::removeModalOverlay(this);
    }
  }

  void onGuiInput(const InputEvent &event) override {
    if (disabled) return;

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left) {
      if (event.isPressed()) {
        if (!m_isMenuOpen) {
          openMenu();
        } else {
          closeMenu();
        }
        const_cast<InputEvent &>(event).setHandled();
      }
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    Color norm = (normalColor.a > 0.0f)
                     ? normalColor
                     : getThemeColor("normal_color", "OptionButton", Color::from_rgba8(45, 48, 62));
    Color hov = (hoverColor.a > 0.0f)
                    ? hoverColor
                    : getThemeColor("hover_color", "OptionButton", Color::from_rgba8(60, 65, 85));
    Color border = (borderColor.a > 0.0f)
                       ? borderColor
                       : getThemeColor("border_color", "OptionButton", Color::from_rgba8(85, 90, 115));
    Color txtCol = (fontColor.a > 0.0f)
                       ? fontColor
                       : getThemeColor("font_color", "OptionButton", Color::WHITE);
    float cr = (cornerRadius >= 0.0f)
                   ? cornerRadius
                   : static_cast<float>(getThemeConstant("corner_radius", "OptionButton", 4));
    float bw = (borderWidth >= 0.0f)
                   ? borderWidth
                   : static_cast<float>(getThemeConstant("border_width", "OptionButton", 1));
    float activeSize = (fontSize > 0.0f)
                           ? fontSize
                           : static_cast<float>(getThemeFontSize("font_size", "OptionButton", 17));
    Ref<Font> activeFont = font ? font : getThemeFont("font", "OptionButton");

    // 1. Draw Main Button Box
    Color activeBg = (m_isHovered || m_isMenuOpen) ? hov : norm;
    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, cr,
                                      activeBg * modulate, border * modulate,
                                      bw);

    // 2. Draw Selected Item Text
    std::string currentText = getSelectedText();
    if (!currentText.empty()) {
      float textX = rect.position.x + 12.0f;
      float textY = rect.position.y + (rect.size.y - activeSize) * 0.5f;
      Renderer2D::drawText(currentText, Vector2(textX, textY), txtCol * modulate,
                           activeSize, activeFont);
    }

    // 3. Draw Dropdown Chevron Icon
    Vector2 arrowCenter{rect.position.x + rect.size.x - 18.0f, rect.position.y + rect.size.y * 0.5f};
    Icons::draw(IconType::ChevronDown, arrowCenter, 14.0f, txtCol * modulate, 2.0f);
  }


private:
  bool m_isMenuOpen = false;
};
