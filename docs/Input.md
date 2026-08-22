# MelkamEngine Input Documentation

MelkamEngine provides a comprehensive, static Input management system built on top of **SDL3** and integrated with MSL's `Vector2`.

Include header:
```cpp
#include "input.hpp"
```

---

## 1. Quick Start Example

```cpp
#include "Window.hpp"
#include "input.hpp"
#include <iostream>

int main() {
    WindowProps props;
    props.title = "MelkamEngine - Input Demo";
    props.width = 1280;
    props.height = 720;

    Window window(props);

    while (window.isOpen()) {
        window.pollEvents(); // Automatically updates Input state

        // 1. Keyboard checks
        if (Input::isKeyJustPressed(Key::Escape)) {
            window.close();
        }

        if (Input::isKeyPressed(Key::Space)) {
            // Jump or fire
        }

        // 2. 2D Movement vector from WASD or Arrows
        Vector2 movement = Input::getVector(Key::A, Key::D, Key::S, Key::W);

        // 3. Mouse checks
        if (Input::isMouseButtonJustPressed(MouseButton::Left)) {
            Vector2 mousePos = Input::getMousePosition();
            std::cout << "Left Click at: (" << mousePos.x << ", " << mousePos.y << ")\n";
        }

        // 4. Mouse Scroll
        float scrollY = Input::getMouseScrollY();

        window.clear(Color::DARK_GRAY);
        window.present();
    }

    return 0;
}
```

---

## 2. Keyboard Input API

| Method | Return Type | Description |
| :--- | :--- | :--- |
| `Input::isKeyPressed(Key key)` | `bool` | True continuously while key is held down |
| `Input::isKeyJustPressed(Key key)` | `bool` | True only on the frame the key was pressed down |
| `Input::isKeyJustReleased(Key key)` | `bool` | True only on the frame the key was released |
| `Input::getAxis(Key negative, Key positive)` | `float` | Returns `-1.0f`, `0.0f`, or `+1.0f` |
| `Input::getVector(Key left, Key right, Key down, Key up, bool normalize = true)` | `Vector2` | Returns directional movement vector (normalized by default) |

---

## 3. Mouse Input API

| Method | Return Type | Description |
| :--- | :--- | :--- |
| `Input::isMouseButtonPressed(MouseButton button)` | `bool` | True continuously while button is held |
| `Input::isMouseButtonJustPressed(MouseButton button)` | `bool` | True only on the frame button was clicked |
| `Input::isMouseButtonJustReleased(MouseButton button)` | `bool` | True only on the frame button was released |
| `Input::getMousePosition()` | `Vector2` | Mouse position in window pixels `{x, y}` |
| `Input::getMouseX()` | `float` | Mouse X position in window pixels |
| `Input::getMouseY()` | `float` | Mouse Y position in window pixels |
| `Input::getMouseDelta()` | `Vector2` | Relative mouse movement since previous frame |
| `Input::getMouseScroll()` | `Vector2` | Scroll wheel delta `{x, y}` |
| `Input::getMouseScrollY()` | `float` | Vertical scroll wheel amount |
| `Input::showCursor(bool show)` | `void` | Show or hide the OS mouse cursor |
| `Input::setRelativeMouseMode(SDL_Window *w, bool enabled)` | `void` | Lock cursor into window for first-person / free camera |

---

## 4. Key Codes (`Key`)

- **Letters**: `Key::A` through `Key::Z`
- **Numbers**: `Key::Num0` through `Key::Num9`
- **Function Keys**: `Key::F1` through `Key::F12`
- **Navigation / Control**: `Key::Space`, `Key::Enter`, `Key::Escape`, `Key::Tab`, `Key::Backspace`, `Key::Delete`, `Key::Left`, `Key::Right`, `Key::Up`, `Key::Down`, `Key::Home`, `Key::End`, `Key::PageUp`, `Key::PageDown`
- **Modifiers**: `Key::LShift`, `Key::RShift`, `Key::LCtrl`, `Key::RCtrl`, `Key::LAlt`, `Key::RAlt`, `Key::LGui`, `Key::RGui`
- **Mouse Buttons**: `MouseButton::Left`, `MouseButton::Middle`, `MouseButton::Right`, `MouseButton::X1`, `MouseButton::X2`
