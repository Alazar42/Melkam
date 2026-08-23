#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include "nodes/UI/StyleBox.hpp"
#include "renderers/Font.hpp"
#include "renderers/Renderer2D.hpp"
#include <algorithm>
#include <string>
#include <vector>

// Tabbed Notebook View Container (inspired by Godot TabContainer)
class TabContainer : public Container {
public:
  // Signals
  Signal<int> tab_changed;
  Signal<int> tab_clicked;
  Signal<int> tab_close_pressed;

  int currentTab = 0;
  float tabHeight = 32.0f;
  bool tabsVisible = true;
  Ref<Font> font = nullptr;
  float fontSize = 0.0f;

  Color tabBgColor = Color(0, 0, 0, 0);
  Color tabActiveBgColor = Color(0, 0, 0, 0);
  Color tabHoverBgColor = Color(0, 0, 0, 0);
  Color fontColor = Color(0, 0, 0, 0);
  Color panelBgColor = Color::from_rgba8(25, 28, 38, 230);
  Color borderColor = Color::from_rgba8(65, 75, 105);

  TabContainer() : Container("TabContainer") {
    mouseFilter = MouseFilter::Stop;
  }

  void setCurrentTab(int tabIndex) {
    auto controls = getTabControls();
    if (tabIndex >= 0 && tabIndex < static_cast<int>(controls.size()) && tabIndex != currentTab) {
      currentTab = tabIndex;
      queueSort();
      tab_changed.emit(currentTab);
    }
  }

  int getCurrentTab() const { return currentTab; }

  void setTabTitle(int index, std::string title) {
    if (index >= static_cast<int>(m_customTitles.size())) {
      m_customTitles.resize(index + 1);
    }
    m_customTitles[index] = std::move(title);
  }

  std::string getTabTitle(int index) const {
    if (index >= 0 && index < static_cast<int>(m_customTitles.size()) && !m_customTitles[index].empty()) {
      return m_customTitles[index];
    }
    auto controls = getTabControls();
    if (index >= 0 && index < static_cast<int>(controls.size())) {
      return controls[index]->name;
    }
    return "Tab " + std::to_string(index);
  }

  int getTabCount() const {
    return static_cast<int>(getTabControls().size());
  }

  void onGuiInput(const InputEvent &event) override {
    if (!tabsVisible) return;

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left && event.isPressed()) {
      Rect2 rect = getGlobalRect();
      if (event.mousePosition.y >= rect.position.y && event.mousePosition.y <= rect.position.y + tabHeight) {
        float relX = event.mousePosition.x - rect.position.x;
        float currX = 0.0f;
        auto controls = getTabControls();

        Ref<Font> activeFont = font ? font : getThemeFont("font", "TabContainer");
        const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
        float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "TabContainer", 15));

        for (int i = 0; i < static_cast<int>(controls.size()); ++i) {
          std::string title = getTabTitle(i);
          float tabW = f.getStringSize(title, activeSize).x + 24.0f;
          if (relX >= currX && relX < currX + tabW) {
            tab_clicked.emit(i);
            setCurrentTab(i);
            const_cast<InputEvent &>(event).setHandled();
            return;
          }
          currX += tabW + 2.0f;
        }
      }
    }
  }

  void fitChildControls() override {
    Rect2 rect = getGlobalRect();
    auto controls = getTabControls();
    if (controls.empty()) return;

    if (currentTab >= static_cast<int>(controls.size())) {
      currentTab = static_cast<int>(controls.size()) - 1;
    }
    if (currentTab < 0) currentTab = 0;

    float topOffset = tabsVisible ? (tabHeight + 4.0f) : 0.0f;

    for (int i = 0; i < static_cast<int>(controls.size()); ++i) {
      auto &ctrl = controls[i];
      if (i == currentTab) {
        ctrl->visible = true;
        ctrl->offsetLeft = 6.0f;
        ctrl->offsetTop = topOffset + 6.0f;
        ctrl->offsetRight = std::max(6.0f, rect.size.x - 6.0f);
        ctrl->offsetBottom = std::max(topOffset + 6.0f, rect.size.y - 6.0f);
      } else {
        ctrl->visible = false;
      }
    }
  }

  void drawControl() override {
    Rect2 rect = getGlobalRect();

    Color tabBg = (tabBgColor.a > 0.0f) ? tabBgColor : getThemeColor("tab_bg_color", "TabContainer", Color::from_rgba8(35, 40, 55));
    Color tabActive = (tabActiveBgColor.a > 0.0f) ? tabActiveBgColor : getThemeColor("tab_active_bg_color", "TabContainer", Color::from_rgba8(55, 65, 95));
    Color tabHover = (tabHoverBgColor.a > 0.0f) ? tabHoverBgColor : getThemeColor("tab_hover_bg_color", "TabContainer", Color::from_rgba8(45, 52, 75));
    Color txtCol = (fontColor.a > 0.0f) ? fontColor : getThemeColor("font_color", "TabContainer", Color::WHITE);

    Ref<Font> activeFont = font ? font : getThemeFont("font", "TabContainer");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (fontSize > 0.0f) ? fontSize : static_cast<float>(getThemeFontSize("font_size", "TabContainer", 15));

    // 1. Draw Page Body Panel
    float topOffset = tabsVisible ? tabHeight : 0.0f;
    Rect2 bodyRect(rect.position.x, rect.position.y + topOffset, rect.size.x, rect.size.y - topOffset);
    Renderer2D::drawRoundedRectScreen(bodyRect.position, bodyRect.size, 6.0f, panelBgColor * modulate, borderColor * modulate, 1.0f);

    // 2. Draw Tab Buttons
    if (tabsVisible) {
      auto controls = getTabControls();
      Vector2 mousePos = Input::getMousePosition();
      float currX = rect.position.x;

      for (int i = 0; i < static_cast<int>(controls.size()); ++i) {
        std::string title = getTabTitle(i);
        Vector2 titleSize = f.getStringSize(title, activeSize);
        float tabW = titleSize.x + 24.0f;
        Rect2 tabRect(currX, rect.position.y, tabW, tabHeight + 2.0f);

        bool isActive = (i == currentTab);
        bool isHover = tabRect.hasPoint(mousePos);

        Color activeTabColor = isActive ? tabActive : (isHover ? tabHover : tabBg);
        Renderer2D::drawRoundedRectScreen(tabRect.position, tabRect.size, 4.0f, activeTabColor * modulate, borderColor * modulate, 1.0f);

        float textX = currX + (tabW - titleSize.x) * 0.5f;
        float textY = rect.position.y + (tabHeight - activeSize) * 0.5f;
        Renderer2D::drawText(title, Vector2(textX, textY), txtCol * modulate, activeSize, activeFont);

        currX += tabW + 2.0f;
      }
    }
  }

private:
  std::vector<Ref<Control>> getTabControls() const {
    std::vector<Ref<Control>> list;
    for (const auto &child : getChildren()) {
      auto ctrl = std::dynamic_pointer_cast<Control>(child);
      if (ctrl) list.push_back(ctrl);
    }
    return list;
  }

  std::vector<std::string> m_customTitles;
};
