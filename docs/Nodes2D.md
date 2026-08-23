# MelkamEngine 2D Nodes Documentation

MelkamEngine provides a Godot-style **2D Scene Node Architecture** (`include/nodes/2D/`) built on top of the ECS and 2D Renderer.

---

## 1. `Node2D` ([include/nodes/2D/Node2D.hpp](file:///c:/Users/Micky/Documents/CppProjects/MelkamEngine/include/nodes/2D/Node2D.hpp))

Base hierarchical class for 2D game entities:

```cpp
#include "MelkamEngine.hpp"

class PlayerNode : public Node2D {
public:
    void onProcess(float delta) override {
        Vector2 dir = Input::getVector(Key::A, Key::D, Key::W, Key::S);
        translate(dir * 300.0f * delta);
    }
};
```

---

## 2. `MeshInstance2D` ([include/nodes/2D/MeshInstance2D.hpp](file:///c:/Users/Micky/Documents/CppProjects/MelkamEngine/include/nodes/2D/MeshInstance2D.hpp))

Godot-style 2D mesh & geometric instance node (Rectangles, Circles, Triangles, Polygons):

```cpp
#include "nodes/2D/MeshInstance2D.hpp"

// Create 2D geometric meshes
auto rect = MeshInstance2D::createRectangle({80.0f, 40.0f}, Color::GOLD);
auto circle = MeshInstance2D::createCircle(30.0f, Color::AQUA, true);
auto tri = MeshInstance2D::createTriangle({-20, 20}, {20, 20}, {0, -20}, Color::CORAL);

// Draw directly or attach to SceneTree:
rect.draw({300.0f, 200.0f}, 0.0f);
```

---

## 3. `Sprite2D` ([include/nodes/2D/Sprite2D.hpp](file:///c:/Users/Micky/Documents/CppProjects/MelkamEngine/include/nodes/2D/Sprite2D.hpp))

Draws a GPU `Texture2D` with color tint, anchor point, and flipping:

```cpp
#include "nodes/2D/Sprite2D.hpp"

auto tex = std::make_shared<Texture2D>("assets/player.bmp");
auto sprite = std::make_shared<Sprite2D>(tex, Color::WHITE);

sprite->origin = {0.5f, 0.5f}; // Centered pivot
sprite->flipH = true;          // Flip horizontally
```

---

## 4. `Camera2D` ([include/nodes/2D/Camera2D.hpp](file:///c:/Users/Micky/Documents/CppProjects/MelkamEngine/include/nodes/2D/Camera2D.hpp))

2D viewport and camera controller with world-to-screen and screen-to-world mapping, `AnchorMode::DragCenter` support, and Godot-style active camera management (`makeCurrent()`).

### Features:
- **Active Camera Registration**: `camera->makeCurrent()` / `camera->make_current()` makes the camera the active camera for the 2D render pass.
- **Anchor Modes**: `AnchorMode::DragCenter` (default, centers the screen on the camera position) or `AnchorMode::FixedTopLeft`.
- **Node Hierarchy Attachment**: Attach `Camera2D` to any `Node2D` (such as `PlayerNode`) via `addChild<Camera2D>()` so it automatically tracks the parent's world position.
- **Zoom & Limits**: `setZoom(float)`, `setLimit(left, top, right, bottom)`.

### Example:
```cpp
class PlayerNode : public CharacterBody2D {
public:
  std::shared_ptr<Camera2D> camera = nullptr;

  void onReady() override {
    // Attach Camera2D and make current so the viewport follows the player
    camera = addChild<Camera2D>();
    camera->makeCurrent();
  }
};
```
