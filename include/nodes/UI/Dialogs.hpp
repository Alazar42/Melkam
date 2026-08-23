#pragma once

#include "core/Memory.hpp"
#include "nodes/UI/Button.hpp"
#include "nodes/UI/Label.hpp"
#include "nodes/UI/UIWindow.hpp"
#include <string>
#include <vector>

// Base Modal Alert Dialog (inspired by Godot AcceptDialog)
class AcceptDialog : public UIWindow {
public:
  // Signals
  Signal<> confirmed;

  std::string dialogText = "Please confirm your action.";
  std::string okButtonText = "OK";

  AcceptDialog() : UIWindow("AcceptDialog") {
    this->title = "Alert";
    this->exclusive = true;
    this->customMinimumSize = Vector2(360.0f, 180.0f);
  }

  explicit AcceptDialog(std::string message, std::string dialogTitle = "Alert")
      : UIWindow(std::move(dialogTitle)), dialogText(std::move(message)) {
    this->exclusive = true;
    this->customMinimumSize = Vector2(360.0f, 180.0f);
  }

  void drawControl() override {
    if (!this->visible) return;

    UIWindow::drawControl();

    Rect2 rect = this->getGlobalRect();
    Ref<Font> activeFont = this->font ? this->font : this->getThemeFont("font", "AcceptDialog");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    float activeSize = (this->fontSize > 0.0f) ? this->fontSize : static_cast<float>(this->getThemeFontSize("font_size", "AcceptDialog", 15));

    // 1. Draw Dialog Message Text
    if (!dialogText.empty()) {
      float textY = rect.position.y + titleBarHeight + 20.0f;
      float textX = rect.position.x + 20.0f;
      Renderer2D::drawText(dialogText, Vector2(textX, textY), this->fontColor, activeSize, activeFont);
    }
  }

  void onGuiInput(const InputEvent &event) override {
    UIWindow::onGuiInput(event);
    if (!this->visible) return;

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left && event.isPressed()) {
      Rect2 okRect = getOkButtonRect();
      if (okRect.hasPoint(event.mousePosition)) {
        hideWindow();
        confirmed.emit();
        const_cast<InputEvent &>(event).setHandled();
      }
    }
  }

  void drawPostChildren() {
    if (!this->visible) return;
    Rect2 okRect = getOkButtonRect();
    Vector2 mousePos = Input::getMousePosition();
    bool isHover = okRect.hasPoint(mousePos);

    Renderer2D::drawRoundedRectScreen(okRect.position, okRect.size, 4.0f,
                                      isHover ? Color::from_rgba8(65, 80, 120) : Color::from_rgba8(45, 52, 75),
                                      Color::from_rgba8(90, 100, 130), 1.0f);

    Ref<Font> activeFont = this->font ? this->font : this->getThemeFont("font", "AcceptDialog");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    Vector2 tSize = f.getStringSize(okButtonText, 15.0f);
    float tx = okRect.position.x + (okRect.size.x - tSize.x) * 0.5f;
    float ty = okRect.position.y + (okRect.size.y - tSize.y) * 0.5f;
    Renderer2D::drawText(okButtonText, Vector2(tx, ty), Color::WHITE, 15.0f, activeFont);
  }

protected:
  virtual Rect2 getOkButtonRect() const {
    Rect2 rect = this->getGlobalRect();
    float btnW = 90.0f;
    float btnH = 32.0f;
    float btnX = rect.position.x + (rect.size.x - btnW) * 0.5f;
    float btnY = rect.position.y + rect.size.y - btnH - 14.0f;
    return Rect2(btnX, btnY, btnW, btnH);
  }
};

// Modal Confirmation Dialog with OK and Cancel options (inspired by Godot ConfirmationDialog)
class ConfirmationDialog : public AcceptDialog {
public:
  // Signals
  Signal<> canceled;

  std::string cancelButtonText = "Cancel";

  ConfirmationDialog() : AcceptDialog("Please confirm.", "Please Confirm") {
    this->name = "ConfirmationDialog";
    okButtonText = "Confirm";
    this->customMinimumSize = Vector2(380.0f, 190.0f);
  }

  explicit ConfirmationDialog(std::string message, std::string dialogTitle = "Please Confirm")
      : AcceptDialog(std::move(message), std::move(dialogTitle)) {
    this->name = "ConfirmationDialog";
    okButtonText = "Confirm";
    this->customMinimumSize = Vector2(380.0f, 190.0f);
  }

  void onGuiInput(const InputEvent &event) override {
    AcceptDialog::onGuiInput(event);
    if (!this->visible) return;

    if (event.type == InputEventType::MouseButton && event.mouseButton == MouseButton::Left && event.isPressed()) {
      Rect2 cancelRect = getCancelButtonRect();
      if (cancelRect.hasPoint(event.mousePosition)) {
        hideWindow();
        canceled.emit();
        const_cast<InputEvent &>(event).setHandled();
      }
    }
  }

  void drawControl() override {
    if (!this->visible) return;

    AcceptDialog::drawControl();

    // Draw Cancel Button
    Rect2 cancelRect = getCancelButtonRect();
    Vector2 mousePos = Input::getMousePosition();
    bool isCancelHover = cancelRect.hasPoint(mousePos);

    Renderer2D::drawRoundedRectScreen(cancelRect.position, cancelRect.size, 4.0f,
                                      isCancelHover ? Color::from_rgba8(55, 60, 75) : Color::from_rgba8(35, 38, 48),
                                      Color::from_rgba8(75, 80, 100), 1.0f);

    Ref<Font> activeFont = this->font ? this->font : this->getThemeFont("font", "ConfirmationDialog");
    const Font &f = activeFont ? *activeFont : *Font::getDefaultFont();
    Vector2 tSize = f.getStringSize(cancelButtonText, 15.0f);
    float tx = cancelRect.position.x + (cancelRect.size.x - tSize.x) * 0.5f;
    float ty = cancelRect.position.y + (cancelRect.size.y - tSize.y) * 0.5f;
    Renderer2D::drawText(cancelButtonText, Vector2(tx, ty), Color::from_rgba8(200, 205, 220), 15.0f, activeFont);

    // Draw OK Button
    drawPostChildren();
  }

protected:
  Rect2 getOkButtonRect() const override {
    Rect2 rect = this->getGlobalRect();
    float btnW = 90.0f;
    float btnH = 32.0f;
    float btnX = rect.position.x + rect.size.x - (btnW * 2.0f) - 24.0f;
    float btnY = rect.position.y + rect.size.y - btnH - 14.0f;
    return Rect2(btnX, btnY, btnW, btnH);
  }

  Rect2 getCancelButtonRect() const {
    Rect2 rect = this->getGlobalRect();
    float btnW = 90.0f;
    float btnH = 32.0f;
    float btnX = rect.position.x + rect.size.x - btnW - 14.0f;
    float btnY = rect.position.y + rect.size.y - btnH - 14.0f;
    return Rect2(btnX, btnY, btnW, btnH);
  }
};
