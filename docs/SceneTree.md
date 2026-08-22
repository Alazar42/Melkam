# MelkamEngine Node Lifecycle & SceneTree Guide

In MelkamEngine, nodes follow the standard **Godot Node Lifecycle Model** with strongly-typed `InputEvent` dispatching:

---

## 1. Node Lifecycle Methods

```cpp
#include "MelkamEngine.hpp"

class PlayerNode : public Node2D {
public:
    // 1. Called once when the node enters the active SceneTree
    void onReady() override {
        // Initialization, child node lookups, asset setup
    }

    // 2. Called every frame for game logic and variable delta updates
    void onProcess(float delta) override {
        Vector2 dir = Input::getVector(Key::A, Key::D, Key::W, Key::S);
        translate(dir * 400.0f * delta);
    }

    // 3. Called on fixed timestep intervals (e.g. 60 Hz) for deterministic physics
    void onPhysicsProcess(float delta) override {
        // Rigid-body integration, raycasting, physics queries
    }

    // 4. Called when input events arrive before UI/nodes consume them
    void onInput(const InputEvent &event) override {
        if (event.isKeyPressed(Key::Space)) {
            // Jump or action
            // event.setHandled(); // Mark handled to stop further propagation
        }
    }

    // 5. Called for unhandled input events (gameplay actions, hotkeys)
    void onUnhandledInput(const InputEvent &event) override {
        if (event.isMouseButtonPressed(MouseButton::Left)) {
            // Click-to-shoot
        }
    }

    // 6. Called during the 2D rendering pass
    void onDraw() override {
        // Draw primitives, meshes, or custom rendering
    }
};
```

---

## 2. Summary Reference Table

| Lifecycle Method | When Called | Signature | Purpose |
| :--- | :--- | :--- | :--- |
| `onReady()` | Once upon entering active tree | `void onReady()` | Node initialization & child setup |
| `onProcess(delta)` | Every frame | `void onProcess(float delta)` | Gameplay logic, smooth animations |
| `onPhysicsProcess(delta)` | Fixed physics tick | `void onPhysicsProcess(float delta)` | Deterministic simulation & collision |
| `onInput(event)` | First input pass | `void onInput(const InputEvent &event)` | UI elements & event interception |
| `onUnhandledInput(event)` | Second input pass | `void onUnhandledInput(const InputEvent &event)` | Gameplay controls & hotkeys |
| `onDraw()` | 2D render pass | `void onDraw()` | Canvas & sprite drawing |
| `onDestroy()` | When node is removed | `void onDestroy()` | Teardown & cleanup |
