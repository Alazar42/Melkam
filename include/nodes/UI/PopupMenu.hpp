#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "nodes/UI/Icons.hpp"
#include "nodes/UI/StyleBox.hpp"

#include "renderers/Font.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

struct MenuItem {
  std::string text;
  int id = 0;
  Ref<Texture2D> icon = nullptr;
  bool checkable = false;
  bool checked = false;
  bool isRadio = false;
  bool separator = false;
  bool disabled = false;
  std::string shortcutText;
};

// Floating Context & Dropdown Popup Menu UI Node (inspired by Godot PopupMenu)
class PopupMenu : public Control {
public:
  // Signals
  Signal<int> index_pressed;
  Signal<int> id_pressed;

  std::vector<MenuItem> items;
  float itemHeight = 28.0f;
  float minMenuWidth = 160.0f;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f;

  Color backgroundColor = Color(0, 0, 0, 0);
  Color borderColor = Color(0, 0, 0, 0);
  Color itemHoverColor = Color(0, 0, 0, 0);
  Color fontColor = Color(0, 0, 0, 0);
  Color separatorColor = Color(0, 0, 0, 0);
  float cornerRadius = -1.0f;

  PopupMenu() : Control("PopupMenu") {
    visible = false;
  }

  ~PopupMenu() override {
    hideMenu();
  }

  void addItem(std::string label, int id = -1, Ref<Texture2D> icon = nullptr) {
    if (id == -1) id = static_cast<int>(items.size());
    items.push_back({std::move(label), id, std::move(icon), false, false, false, false, false, ""});
  }

  void addCheckItem(std::string label, int id = -1, bool checked = false) {
    if (id == -1) id = static_cast<int>(items.size());
    items.push_back({std::move(label), id, nullptr, true, checked, false, false, false, ""});
  }

  void addRadioCheckItem(std::string label, int id = -1, bool checked = false) {
    if (id == -1) id = static_cast<int>(items.size());
    items.push_back({std::move(label), id, nullptr, true, checked, true, false, false, ""});
  }

  void addSeparator(std::string label = "") {
    items.push_back({std::move(label), -1, nullptr, false, false, false, true, true, ""});
  }

  void addItemWithShortcut(std::string label, std::string shortcut, int id = -1) {
    if (id == -1) id = static_cast<int>(items.size());
    items.push_back({std::move(label), id, nullptr, false, false, false, false, false, std::move(shortcut)});
  }

  void setItemChecked(int index, bool checked) {
    if (index >= 0 && index < static_cast<int>(items.size())) {
      items[index].checked = checked;
    }
  }

  bool isItemChecked(int index) const {
    if (index >= 0 && index < static_cast<int>(items.size())) {
      return items[index].checked;
    }
    return false;
  }

  void clear() {
    hideMenu();
    items.clear();
  }

  int getItemCount() const { return static_cast<int>(items.size()); }

  void popup(const Rect2 &anchorRect) {
    if (items.empty()) return;

    m_isOpen = true;
    m_hoveredItem = -1;

    float maxW = minMenuWidth;
    Ref<Font> activeFont = font ? font : getThemeFont("font", "PopupMenu");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "PopupMenu", 15));

    float totalH = 8.0f; // top/bottom padding
    for (const auto &item : items) {
      if (item.separator) {
        totalH += 8.0f;
      } else {
        totalH += itemHeight;
        float itemW = 36.0f + f.getStringSize(item.text, activeSize).x;
        if (!item.shortcutText.empty()) {
          itemW += 24.0f + f.getStringSize(item.shortcutText, activeSize - 2.0f).x;
        }
        if (itemW > maxW) maxW = itemW;
      }
    }
    maxW += 20.0f;

    Vector2 vp = Window::getViewportSize();
    float posX = anchorRect.position.x;
    float posY = anchorRect.position.y + anchorRect.size.y + 2.0f;

    if (posX + maxW > vp.x) posX = std::max(0.0f, vp.x - maxW);
    if (posY + totalH > vp.y) posY = std::max(0.0f, anchorRect.position.y - totalH - 2.0f);

    m_menuRect = Rect2(posX, posY, maxW, totalH);

    Control::setModalOverlay(
        this,
        [this]() { drawOverlay(); },
        [this](const InputEvent &event) -> bool { return handleOverlayInput(event); });
  }

  void popupAtPosition(const Vector2 &screenPos) {
    popup(Rect2(screenPos.x, screenPos.y, 0.0f, 0.0f));
  }

  void hideMenu() {
    if (m_isOpen) {
      m_isOpen = false;
      Control::removeModalOverlay(this);
    }
  }

private:
  void drawOverlay() {
    Color bg = (backgroundColor.a > 0.0f) ? backgroundColor : getThemeColor("dropdown_bg_color", "OptionButton", Color::from_rgba8(28, 30, 42, 250));
    Color border = (borderColor.a > 0.0f) ? borderColor : getThemeColor("border_color", "OptionButton", Color::from_rgba8(75, 80, 110));
    Color itemHov = (itemHoverColor.a > 0.0f) ? itemHoverColor : getThemeColor("item_hover_color", "OptionButton", Color::from_rgba8(52, 120, 246));
    Color txtCol = (fontColor.a > 0.0f) ? fontColor : getThemeColor("font_color", "OptionButton", Color::WHITE);
    Color sepCol = (separatorColor.a > 0.0f) ? separatorColor : Color::from_rgba8(60, 65, 85);
    float cr = (cornerRadius >= 0.0f) ? cornerRadius : 5.0f;

    Ref<Font> activeFont = font ? font : getThemeFont("font", "PopupMenu");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "PopupMenu", 15));

    // Shadow & Window Frame
    Renderer2D::drawRoundedRectScreen(m_menuRect.position + Vector2(2.0f, 3.0f), m_menuRect.size, cr,
                                      Color::from_rgba8(0, 0, 0, 100));
    Renderer2D::drawRoundedRectScreen(m_menuRect.position, m_menuRect.size, cr,
                                      bg, border, 1.5f);

    float currY = m_menuRect.position.y + 4.0f;
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
      const auto &item = items[i];

      if (item.separator) {
        float lineY = currY + 3.5f;
        Renderer2D::drawRectScreen(Vector2(m_menuRect.position.x + 8.0f, lineY),
                                  Vector2(m_menuRect.size.x - 16.0f, 1.0f), sepCol);
        currY += 8.0f;
        continue;
      }

      Rect2 itemRect(m_menuRect.position.x + 4.0f, currY, m_menuRect.size.x - 8.0f, itemHeight);

      if (i == m_hoveredItem && !item.disabled) {
        Renderer2D::drawRoundedRectScreen(itemRect.position, itemRect.size, 3.0f, itemHov);
      }

      float textX = itemRect.position.x + 28.0f;
      float textY = itemRect.position.y + (itemHeight - activeSize) * 0.5f;

      // Draw Checkmark or Radio
      if (item.checkable) {
        float indSize = 14.0f;
        float indX = itemRect.position.x + 8.0f;
        float indY = itemRect.position.y + (itemHeight - indSize) * 0.5f;

        if (item.isRadio) {
          Renderer2D::drawCircleScreen(Vector2(indX + indSize * 0.5f, indY + indSize * 0.5f), indSize * 0.5f,
                                       item.checked ? Color::from_rgba8(52, 199, 89) : Color::from_rgba8(70, 75, 95), true);
          if (item.checked) {
            Renderer2D::drawCircleScreen(Vector2(indX + indSize * 0.5f, indY + indSize * 0.5f), indSize * 0.25f,
                                         Color::WHITE, true);
          }
        } else {
          Renderer2D::drawRoundedRectScreen(Vector2(indX, indY), Vector2(indSize, indSize), 2.0f,
                                            item.checked ? Color::from_rgba8(52, 120, 246) : Color::from_rgba8(45, 48, 65),
                                            Color::from_rgba8(90, 95, 120), 1.0f);
          if (item.checked) {
            Vector2 checkCenter = Vector2(indX + indSize * 0.5f, indY + indSize * 0.5f);
            Icons::draw(IconType::Check, checkCenter, indSize * 0.8f, Color::WHITE, 1.8f);
          }
        }
      } else if (item.icon && item.icon->isValid()) {

        float icSize = 16.0f;
        float icX = itemRect.position.x + 6.0f;
        float icY = itemRect.position.y + (itemHeight - icSize) * 0.5f;
        Renderer2D::drawTextureScreen(item.icon.get(), Vector2(icX, icY), Vector2(icSize, icSize));
      }

      Color itemTxtCol = item.disabled ? Color::from_rgba8(110, 115, 130) : txtCol;
      Renderer2D::drawText(item.text, Vector2(textX, textY), itemTxtCol, activeSize, activeFont);

      // Shortcut
      if (!item.shortcutText.empty()) {
        Vector2 scSize = f.getStringSize(item.shortcutText, activeSize - 2.0f);
        float scX = m_menuRect.position.x + m_menuRect.size.x - scSize.x - 12.0f;
        Renderer2D::drawText(item.shortcutText, Vector2(scX, textY + 1.0f),
                             Color::from_rgba8(140, 145, 165), activeSize - 2.0f, activeFont);
      }

      currY += itemHeight;
    }
  }

  int getItemAt(const Vector2 &mousePos) const {
    if (!m_menuRect.hasPoint(mousePos)) return -1;
    float relY = mousePos.y - (m_menuRect.position.y + 4.0f);
    float accumY = 0.0f;
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
      float h = items[i].separator ? 8.0f : itemHeight;
      if (relY >= accumY && relY < accumY + h) {
        if (!items[i].separator && !items[i].disabled) {
          return i;
        }
        return -1;
      }
      accumY += h;
    }
    return -1;
  }

  bool handleOverlayInput(const InputEvent &event) {
    if (!m_isOpen) return false;

    Vector2 mousePos = Input::getMousePosition();

    if (event.type == InputEventType::MouseMotion) {
      m_hoveredItem = getItemAt(mousePos);
      return true;
    }

    if (event.type == InputEventType::MouseButton && event.isPressed()) {
      if (m_menuRect.hasPoint(mousePos)) {
        int idx = getItemAt(mousePos);
        if (idx >= 0 && idx < static_cast<int>(items.size())) {
          auto &it = items[idx];
          if (!it.disabled && !it.separator) {
            if (it.checkable) {
              it.checked = !it.checked;
            }
            int id = it.id;
            hideMenu();
            index_pressed.emit(idx);
            id_pressed.emit(id);
            return true;
          }
        }
        return true;
      } else {
        hideMenu();
        return true;
      }
    }

    if (event.type == InputEventType::Key && event.isPressed() && event.key == Key::Escape) {
      hideMenu();
      return true;
    }

    return false;
  }

  bool m_isOpen = false;
  Rect2 m_menuRect;
  int m_hoveredItem = -1;
};

