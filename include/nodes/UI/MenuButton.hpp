#pragma once

#include "nodes/UI/Button.hpp"
#include "nodes/UI/PopupMenu.hpp"

// Menu Button UI Node (inspired by Godot MenuButton) that automatically shows an attached PopupMenu.
class MenuButton : public Button {
public:
  MenuButton() : Button() {
    name = "MenuButton";
    m_popup = makeRef<PopupMenu>();
    setupMenuConnection();
  }

  explicit MenuButton(std::string buttonText) : Button(std::move(buttonText)) {
    name = "MenuButton";
    m_popup = makeRef<PopupMenu>();
    setupMenuConnection();
  }

  Ref<PopupMenu> getPopup() const {
    return m_popup;
  }

private:
  void setupMenuConnection() {
    pressed.connect([this]() {
      if (m_popup) {
        m_popup->popup(getGlobalRect());
      }
    });
  }

  Ref<PopupMenu> m_popup = nullptr;
};
