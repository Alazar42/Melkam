# MelkamEngine 2D Renderer & Camera Documentation

MelkamEngine provides a modular, hardware-accelerated **2D Primitive and Batch Renderer** (`Renderer2D`) and a 2D world-to-screen Camera system (`Camera2D`).

Include headers:
```cpp
#include "renderers/Renderer2D.hpp"
#include "nodes/2D/Camera2D.hpp"
```

---

## 1. Quick Start Example

```cpp
#include "window.hpp"
#include "renderers/Renderer2D.hpp"
#include "nodes/2D/Camera2D.hpp"
#include "input.hpp"
#include "time.hpp"

int main() {
    WindowProps props;
    props.title = "MelkamEngine - 2D Rendering Demo";
    props.width = 1280;
    props.height = 720;

    Window window(props);
    Renderer2D::init(window); // Initialize renderer

    Vector2 playerPos(640.0f, 360.0f);
    float playerSpeed = 400.0f;

    while (window.isOpen()) {
        window.pollEvents();

        // 1. Movement logic
        float dt = Time::getDeltaTime();
        Vector2 inputDir = Input::getVector(Key::A, Key::D, Key::S, Key::W);
        playerPos += inputDir * playerSpeed * dt;

        // 2. Rendering pass
        window.clear(Color::DARK_GRAY);

        Renderer2D::begin();

        // Draw player rectangle
        Renderer2D::drawRect(playerPos, {60.0f, 60.0f}, Color::GOLD);

        // Draw crosshair circle following mouse
        Renderer2D::drawCircle(Input::getMousePosition(), 20.0f, Color::AQUA, false);

        // Draw a connecting line
        Renderer2D::drawLine(playerPos + Vector2(30.0f, 30.0f), Input::getMousePosition(), Color::RED, 2.0f);

        Renderer2D::end();

        window.present();
    }

    return 0;
}
```

---

## 2. 2D Drawing API Reference

### Rectangles
```cpp
// Axis-aligned rectangle (filled or wireframe)
Renderer2D::drawRect({x, y}, {width, height}, Color::CYAN, true);

// Rotated rectangle (rotation in radians, centered)
Renderer2D::drawRectRotated({x, y}, {width, height}, 0.785f, Color::ORANGE, true);
```

### Circles
```cpp
// Circle with radius and segment precision (filled or wireframe)
Renderer2D::drawCircle({centerX, centerY}, 40.0f, Color::GREEN, true, 32);
```

### Lines, Points & Triangles
```cpp
// Line with custom pixel thickness
Renderer2D::drawLine({x1, y1}, {x2, y2}, Color::WHITE, 3.0f);

// Single pixel point
Renderer2D::drawPoint({x, y}, Color::YELLOW);

// Multi-point polyline
std::vector<Vector2> points = {{100, 100}, {200, 150}, {150, 300}};
Renderer2D::drawLines(points, Color::PINK, true); // true = closed polygon outline

// Triangle (filled or wireframe)
Renderer2D::drawTriangle({100, 100}, {200, 100}, {150, 200}, Color::CORAL, true);
```

---

## 3. 2D Camera System (`Camera2D`)

`Camera2D` provides smooth panning, zooming, rotation, and coordinate conversion (screen-to-world & world-to-screen).

### Using the Camera with `Renderer2D`:
```cpp
Camera2D camera;
camera.position = playerPos;                // Follow player
camera.offset = {1280.0f / 2, 720.0f / 2};  // Keep player centered on screen
camera.zoom = 1.0f;                         // 1.0 = normal, 2.0 = 2x zoom-in

Renderer2D::begin(camera); // All drawing calls below will be in World Coordinates!

// Draw world elements
Renderer2D::drawRect({0.0f, 0.0f}, {100.0f, 100.0f}, Color::GREEN);
Renderer2D::drawRect(playerPos, {50.0f, 50.0f}, Color::GOLD);

Renderer2D::end();
```

### Coordinate Conversion (Mouse Picking in World):
```cpp
// Convert mouse pixel coordinates to actual in-game world position:
Vector2 worldMouse = camera.screenToWorld(Input::getMousePosition());
```
