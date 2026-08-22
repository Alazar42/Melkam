#pragma once

#include "helper/vectors/Vector2.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <cstdint>

// Strongly-typed keyboard key codes (mapped to physical SDL_Scancode values).
enum class Key : uint16_t {
  Unknown = SDL_SCANCODE_UNKNOWN,

  // Letters
  A = SDL_SCANCODE_A,
  B = SDL_SCANCODE_B,
  C = SDL_SCANCODE_C,
  D = SDL_SCANCODE_D,
  E = SDL_SCANCODE_E,
  F = SDL_SCANCODE_F,
  G = SDL_SCANCODE_G,
  H = SDL_SCANCODE_H,
  I = SDL_SCANCODE_I,
  J = SDL_SCANCODE_J,
  K = SDL_SCANCODE_K,
  L = SDL_SCANCODE_L,
  M = SDL_SCANCODE_M,
  N = SDL_SCANCODE_N,
  O = SDL_SCANCODE_O,
  P = SDL_SCANCODE_P,
  Q = SDL_SCANCODE_Q,
  R = SDL_SCANCODE_R,
  S = SDL_SCANCODE_S,
  T = SDL_SCANCODE_T,
  U = SDL_SCANCODE_U,
  V = SDL_SCANCODE_V,
  W = SDL_SCANCODE_W,
  X = SDL_SCANCODE_X,
  Y = SDL_SCANCODE_Y,
  Z = SDL_SCANCODE_Z,

  // Numbers (Top Row)
  Num0 = SDL_SCANCODE_0,
  Num1 = SDL_SCANCODE_1,
  Num2 = SDL_SCANCODE_2,
  Num3 = SDL_SCANCODE_3,
  Num4 = SDL_SCANCODE_4,
  Num5 = SDL_SCANCODE_5,
  Num6 = SDL_SCANCODE_6,
  Num7 = SDL_SCANCODE_7,
  Num8 = SDL_SCANCODE_8,
  Num9 = SDL_SCANCODE_9,

  // Function Keys
  F1 = SDL_SCANCODE_F1,
  F2 = SDL_SCANCODE_F2,
  F3 = SDL_SCANCODE_F3,
  F4 = SDL_SCANCODE_F4,
  F5 = SDL_SCANCODE_F5,
  F6 = SDL_SCANCODE_F6,
  F7 = SDL_SCANCODE_F7,
  F8 = SDL_SCANCODE_F8,
  F9 = SDL_SCANCODE_F9,
  F10 = SDL_SCANCODE_F10,
  F11 = SDL_SCANCODE_F11,
  F12 = SDL_SCANCODE_F12,

  // Control & Navigation
  Escape = SDL_SCANCODE_ESCAPE,
  Enter = SDL_SCANCODE_RETURN,
  Tab = SDL_SCANCODE_TAB,
  Backspace = SDL_SCANCODE_BACKSPACE,
  Insert = SDL_SCANCODE_INSERT,
  Delete = SDL_SCANCODE_DELETE,
  Right = SDL_SCANCODE_RIGHT,
  Left = SDL_SCANCODE_LEFT,
  Down = SDL_SCANCODE_DOWN,
  Up = SDL_SCANCODE_UP,
  PageUp = SDL_SCANCODE_PAGEUP,
  PageDown = SDL_SCANCODE_PAGEDOWN,
  Home = SDL_SCANCODE_HOME,
  End = SDL_SCANCODE_END,
  CapsLock = SDL_SCANCODE_CAPSLOCK,
  ScrollLock = SDL_SCANCODE_SCROLLLOCK,
  NumLock = SDL_SCANCODE_NUMLOCKCLEAR,
  PrintScreen = SDL_SCANCODE_PRINTSCREEN,
  Pause = SDL_SCANCODE_PAUSE,
  Space = SDL_SCANCODE_SPACE,

  // Modifiers
  LCtrl = SDL_SCANCODE_LCTRL,
  LShift = SDL_SCANCODE_LSHIFT,
  LAlt = SDL_SCANCODE_LALT,
  LGui = SDL_SCANCODE_LGUI,
  RCtrl = SDL_SCANCODE_RCTRL,
  RShift = SDL_SCANCODE_RSHIFT,
  RAlt = SDL_SCANCODE_RALT,
  RGui = SDL_SCANCODE_RGUI,

  // Keypad
  KP_0 = SDL_SCANCODE_KP_0,
  KP_1 = SDL_SCANCODE_KP_1,
  KP_2 = SDL_SCANCODE_KP_2,
  KP_3 = SDL_SCANCODE_KP_3,
  KP_4 = SDL_SCANCODE_KP_4,
  KP_5 = SDL_SCANCODE_KP_5,
  KP_6 = SDL_SCANCODE_KP_6,
  KP_7 = SDL_SCANCODE_KP_7,
  KP_8 = SDL_SCANCODE_KP_8,
  KP_9 = SDL_SCANCODE_KP_9,
  KP_Divide = SDL_SCANCODE_KP_DIVIDE,
  KP_Multiply = SDL_SCANCODE_KP_MULTIPLY,
  KP_Minus = SDL_SCANCODE_KP_MINUS,
  KP_Plus = SDL_SCANCODE_KP_PLUS,
  KP_Enter = SDL_SCANCODE_KP_ENTER,
  KP_Period = SDL_SCANCODE_KP_PERIOD,

  // Punctuation
  Grave = SDL_SCANCODE_GRAVE,
  Minus = SDL_SCANCODE_MINUS,
  Equals = SDL_SCANCODE_EQUALS,
  LeftBracket = SDL_SCANCODE_LEFTBRACKET,
  RightBracket = SDL_SCANCODE_RIGHTBRACKET,
  Backslash = SDL_SCANCODE_BACKSLASH,
  Semicolon = SDL_SCANCODE_SEMICOLON,
  Apostrophe = SDL_SCANCODE_APOSTROPHE,
  Comma = SDL_SCANCODE_COMMA,
  Period = SDL_SCANCODE_PERIOD,
  Slash = SDL_SCANCODE_SLASH
};

// Strongly-typed mouse button identifiers.
enum class MouseButton : uint8_t {
  Left = SDL_BUTTON_LEFT,
  Middle = SDL_BUTTON_MIDDLE,
  Right = SDL_BUTTON_RIGHT,
  X1 = SDL_BUTTON_X1,
  X2 = SDL_BUTTON_X2
};

// Comprehensive Input manager handling real-time Keyboard and Mouse states on top of SDL3.
class Input {
public:
  // Processes an SDL event into the input state system.
  static void onEvent(const SDL_Event &event) {
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Scancode scancode = event.key.scancode;
      if (scancode < SDL_SCANCODE_COUNT) {
        s_currentKeys[scancode] = true;
      }
      break;
    }

    case SDL_EVENT_KEY_UP: {
      SDL_Scancode scancode = event.key.scancode;
      if (scancode < SDL_SCANCODE_COUNT) {
        s_currentKeys[scancode] = false;
      }
      break;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      uint8_t btn = event.button.button;
      if (btn < s_currentMouseButtons.size()) {
        s_currentMouseButtons[btn] = true;
      }
      break;
    }

    case SDL_EVENT_MOUSE_BUTTON_UP: {
      uint8_t btn = event.button.button;
      if (btn < s_currentMouseButtons.size()) {
        s_currentMouseButtons[btn] = false;
      }
      break;
    }

    case SDL_EVENT_MOUSE_MOTION: {
      s_mousePosition.x = event.motion.x;
      s_mousePosition.y = event.motion.y;
      s_mouseDelta.x += event.motion.xrel;
      s_mouseDelta.y += event.motion.yrel;
      break;
    }

    case SDL_EVENT_MOUSE_WHEEL: {
      s_mouseScroll.x += event.wheel.x;
      s_mouseScroll.y += event.wheel.y;
      break;
    }

    default:
      break;
    }
  }

  // Prepares input state for a new frame (updates transition buffers & resets deltas).
  // Call this at the end of each frame or before window.pollEvents().
  static void nextFrame() {
    s_previousKeys = s_currentKeys;
    s_previousMouseButtons = s_currentMouseButtons;
    s_mouseDelta = Vector2(0.0f, 0.0f);
    s_mouseScroll = Vector2(0.0f, 0.0f);
  }

  // Returns true while the specified key is held down.
  static bool isKeyPressed(Key key) {
    auto scancode = static_cast<size_t>(key);
    return scancode < SDL_SCANCODE_COUNT && s_currentKeys[scancode];
  }

  // Returns true on the single frame the specified key was initially pressed.
  static bool isKeyJustPressed(Key key) {
    auto scancode = static_cast<size_t>(key);
    return scancode < SDL_SCANCODE_COUNT && s_currentKeys[scancode] &&
           !s_previousKeys[scancode];
  }

  // Returns true on the single frame the specified key was released.
  static bool isKeyJustReleased(Key key) {
    auto scancode = static_cast<size_t>(key);
    return scancode < SDL_SCANCODE_COUNT && !s_currentKeys[scancode] &&
           s_previousKeys[scancode];
  }

  // Returns true while the specified mouse button is held down.
  static bool isMouseButtonPressed(MouseButton button) {
    auto btn = static_cast<size_t>(button);
    return btn < s_currentMouseButtons.size() && s_currentMouseButtons[btn];
  }

  // Returns true on the single frame the specified mouse button was pressed.
  static bool isMouseButtonJustPressed(MouseButton button) {
    auto btn = static_cast<size_t>(button);
    return btn < s_currentMouseButtons.size() && s_currentMouseButtons[btn] &&
           !s_previousMouseButtons[btn];
  }

  // Returns true on the single frame the specified mouse button was released.
  static bool isMouseButtonJustReleased(MouseButton button) {
    auto btn = static_cast<size_t>(button);
    return btn < s_currentMouseButtons.size() && !s_currentMouseButtons[btn] &&
           s_previousMouseButtons[btn];
  }

  // Returns the current mouse position in window coordinates (pixels).
  static Vector2 getMousePosition() { return s_mousePosition; }

  // Returns the current mouse X coordinate in window pixels.
  static float getMouseX() { return s_mousePosition.x; }

  // Returns the current mouse Y coordinate in window pixels.
  static float getMouseY() { return s_mousePosition.y; }

  // Returns the relative mouse motion (delta) since the previous frame.
  static Vector2 getMouseDelta() { return s_mouseDelta; }

  // Returns the mouse wheel scroll offset for this frame (x = horizontal, y = vertical).
  static Vector2 getMouseScroll() { return s_mouseScroll; }

  // Returns the vertical mouse wheel scroll amount for this frame.
  static float getMouseScrollY() { return s_mouseScroll.y; }

  // Computes a 1D axis value between -1.0f (negative key) and +1.0f (positive key).
  static float getAxis(Key negative, Key positive) {
    float val = 0.0f;
    if (isKeyPressed(negative)) {
      val -= 1.0f;
    }
    if (isKeyPressed(positive)) {
      val += 1.0f;
    }
    return val;
  }

  // Computes a 2D movement vector from 4 directional keys (e.g. A, D, S, W or Left, Right, Down, Up).
  static Vector2 getVector(Key left, Key right, Key down, Key up,
                           bool normalize = true) {
    Vector2 vec(getAxis(left, right), getAxis(down, up));
    if (normalize && !vec.is_zero_approx()) {
      return vec.normalized();
    }
    return vec;
  }

  // Shows or hides the OS mouse cursor.
  static void showCursor(bool show) {
    if (show) {
      SDL_ShowCursor();
    } else {
      SDL_HideCursor();
    }
  }

  // Enables or disables relative mouse mode (locks cursor inside window for FPS/camera controls).
  static void setRelativeMouseMode(SDL_Window *window, bool enabled) {
    if (window) {
      SDL_SetWindowRelativeMouseMode(window, enabled);
    }
  }

private:
  inline static std::array<bool, SDL_SCANCODE_COUNT> s_currentKeys{};
  inline static std::array<bool, SDL_SCANCODE_COUNT> s_previousKeys{};

  inline static std::array<bool, 8> s_currentMouseButtons{};
  inline static std::array<bool, 8> s_previousMouseButtons{};

  inline static Vector2 s_mousePosition{0.0f, 0.0f};
  inline static Vector2 s_mouseDelta{0.0f, 0.0f};
  inline static Vector2 s_mouseScroll{0.0f, 0.0f};
};