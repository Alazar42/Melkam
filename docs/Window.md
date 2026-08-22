# MelkamEngine Window Documentation

MelkamEngine provides a custom, modern C++ wrapper around **SDL3** for window management, event handling, and hardware-accelerated rendering.

Include header:
```cpp
#include "Window.hpp"
```

---

## 1. Quick Start Example

```cpp
#include "Window.hpp"
#include <iostream>

int main() {
    // 1. Configure window properties
    WindowProps props;
    props.title = "MelkamEngine - Game Window";
    props.width = 1280;
    props.height = 720;
    props.vsync = true;
    props.resizable = true;
    props.clearColor = Color::from_rgba8(25, 25, 30);

    // 2. Create the window
    Window window(props);

    // 3. Main Game / Render Loop
    while (window.isOpen()) {
        // Poll OS events (close, resize, input)
        window.pollEvents();

        // Clear frame with an MSL Color
        window.clear(Color::DARK_GRAY);

        // Swap / Present frame to screen
        window.present();
    }

    return 0;
}
```

---

## 2. Window Properties (`WindowProps`)

| Property | Type | Default Value | Description |
| :--- | :--- | :--- | :--- |
| `title` | `std::string` | `"MelkamEngine"` | Window title text |
| `width` | `uint32_t` | `1280` | Initial width in pixels |
| `height` | `uint32_t` | `720` | Initial height in pixels |
| `vsync` | `bool` | `true` | Vertical sync enabled/disabled |
| `resizable` | `bool` | `true` | User can resize window |
| `fullscreen` | `bool` | `false` | Fullscreen mode |
| `maximized` | `bool` | `false` | Start window maximized |
| `minimized` | `bool` | `false` | Start window minimized |
| `clearColor` | `Color` | `(25, 25, 30)` | Default background clear color |

---

## 3. Window API Reference

### Lifecycle & Loop Control
```cpp
window.isOpen();       // Returns true while running
window.shouldClose();  // Returns true if close requested
window.close();        // Closes window and frees SDL resources
```

### Event Polling & Callbacks
```cpp
// Polls standard events (Quit, Close, Resize, Maximize, Minimize, Restore)
window.pollEvents();

// Optional: Register custom event callback for keyboard/mouse input
window.setEventCallback([](const SDL_Event &event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        std::cout << "Key pressed: " << event.key.key << "\n";
    }
});
```

### Frame Rendering Helpers
```cpp
// Clear with MSL Color
window.clear(Color::DARK_GRAY);
window.clear(Color::html("#19191e"));
window.clear(Color::from_hsv(0.6f, 0.7f, 0.8f));

// Clear with default configured clear color
window.clear();

// Clear with raw RGB(A) integer values
window.clear(30, 30, 35, 255);

// Set renderer draw color using MSL Color
window.setDrawColor(Color::CYAN);

// Present backbuffer
window.present();
```

### Getters & Setters
```cpp
// Dimensions
uint32_t w = window.getWidth();
uint32_t h = window.getHeight();
float aspect = window.getAspectRatio();
window.setSize(1920, 1080);

// Title
window.setTitle("New Title");
const std::string &title = window.getTitle();

// Display Modes
window.setVSync(true);
bool vsync = window.isVSync();

window.setFullscreen(true);
bool fullscreen = window.isFullscreen();

window.maximize();
bool maximized = window.isMaximized();

window.minimize();
bool minimized = window.isMinimized();

window.restore();

window.setResizable(false);
bool resizable = window.isResizable();

// Native SDL3 Pointers
SDL_Window *nativeWindow = window.getNativeWindow();
SDL_Renderer *nativeRenderer = window.getRenderer();
```
