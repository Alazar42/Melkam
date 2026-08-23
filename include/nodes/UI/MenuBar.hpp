#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Control.hpp"
#include "nodes/UI/PopupMenu.hpp"
#include "renderers/Font.hpp"
#include "renderers/Renderer2D.hpp"
#include <memory>
#include <string>
#include <vector>

struct MenuBarItem {
  std::string title;
  Ref<PopupMenu> popup = nullptr;
  bool disabled = false;
};

// Top Window Application Menu Bar (inspired by Godot MenuBar)
class MenuBar : public Control {
public:
  std::vector<MenuBarItem> menus;
  float barHeight = 28.0f;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f;

  Color backgroundColor = Color::from_rgba8(24, 26, 36);
  Color itemHoverColor = Color::from_rgba8(52, 120, 246);
  Color fontColor = Color::WHITE;
  Color borderColor = Color::from_rgba8(50, 55, 75);

  MenuBar() : Control("MenuBar") {
    customMinimumSize = {200.0f, 28.0f};
    mouseFilter = MouseFilter::Stop;
  }

  Ref<PopupMenu> addMenu(std::string title, Ref<PopupMenu> popup = nullptr) {
    if (!popup) {
      popup = makeRef<PopupMenu>();
    }
    menus.push_back({std::move(title), popup, false});
    return popup;
  }


  void onGuiInput(const InputEvent &event) override {
    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left && event.isPressed()) {
      Rect2 rect = getGlobalRect();
      float relX = event.mousePosition.x - rect.position.x;
      float currX = 6.0f;

      Ref<Font> activeFont = font ? font : getThemeFont("font", "MenuBar");
      const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
      float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "MenuBar", 15));

      for (auto &menu : menus) {
        float itemW = f.getStringSize(menu.title, activeSize).x + 16.0f;
        Rect2 itemRect(rect.position.x + currX, rect.position.y, itemW, barHeight);

        if (relX >= currX && relX < currX + itemW && !menu.disabled) {
          if (menu.popup) {
            menu.popup->popup(itemRect);
          }
          const_cast<InputEvent &>(event).setHandled();
          return;
        }
        currX += itemW + 4.0f;
      }
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();
    Vector2 mousePos = Input::getMousePosition();

    // 1. Draw Menu Bar Background & Bottom Border
    Renderer2D::drawRectScreen(rect.position, rect.size, backgroundColor * modulate, true);
    Renderer2D::drawRectScreen(Vector2(rect.position.x, rect.position.y + rect.size.y - 1.0f),
                              Vector2(rect.size.x, 1.0f), borderColor * modulate, true);

    // 2. Draw Menu Titles
    Ref<Font> activeFont = font ? font : getThemeFont("font", "MenuBar");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "MenuBar", 15));

    float currX = 6.0f;
    for (const auto &menu : menus) {
      Vector2 titleSize = f.getStringSize(menu.title, activeSize);
      float itemW = titleSize.x + 16.0f;
      Rect2 itemRect(rect.position.x + currX, rect.position.y + 2.0f, itemW, barHeight - 4.0f);

      bool isHover = itemRect.hasPoint(mousePos) && !menu.disabled;
      if (isHover) {
        Renderer2D::drawRoundedRectScreen(itemRect.position, itemRect.size, 3.0f, itemHoverColor * modulate);
      }

      float textX = itemRect.position.x + (itemW - titleSize.x) * 0.5f;
      float textY = rect.position.y + (barHeight - activeSize) * 0.5f;
      Color txtCol = menu.disabled ? Color::from_rgba8(110, 115, 130) : fontColor;
      Renderer2D::drawText(menu.title, Vector2(textX, textY), txtCol * modulate, activeSize, activeFont);

      currX += itemW + 4.0f;
    }
  }
};
