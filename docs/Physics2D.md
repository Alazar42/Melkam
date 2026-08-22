# MelkamEngine 2D Physics & Box2D Guide

MelkamEngine provides a high-performance **2D Physics Engine powered by Box2D 3.1.1** following Godot's node-based physics architecture.

---

## 1. Core Physics Nodes

| Node | Godot Equivalent | Description |
| :--- | :--- | :--- |
| `CharacterBody2D` | `CharacterBody2D` | Controllable character controller with `moveAndSlide()`, `isOnFloor()`, and gravity |
| `StaticBody2D` | `StaticBody2D` | Immovable geometry (floors, walls, platforms) |
| `Area2D` | `Area2D` | Sensor & trigger volumes with `onBodyEntered` and `onBodyExited` |
| `RigidBody2D` | `RigidBody2D` | Dynamic simulated physics body with mass, bounce, friction, and forces |
| `CollisionShape2D` | `CollisionShape2D` | Collision bounds (`Rectangle`, `Circle`, `Capsule`) attached to any physics body |

---

## 2. CharacterBody2D Example (Player Controller)

```cpp
#include "MelkamEngine.hpp"

class PlayerNode : public CharacterBody2D {
public:
    float speed = 350.0f;
    float jumpVelocity = -550.0f;
    float gravity = 1200.0f;

    void onReady() override {
        // 1. Attach Box Collider (40x50 px)
        spawnChild<CollisionShape2D>(Vector2(40.0f, 50.0f));

        // 2. Attach Visual Mesh
        spawnChild<MeshInstance2D>(Vector2(40.0f, 50.0f), Color::GOLD);
    }

    void onPhysicsProcess(float delta) override {
        // Apply Gravity
        if (!isOnFloor()) {
            velocity.y += gravity * delta;
        } else {
            velocity.y = 0.0f;
        }

        // Handle Jump
        if (Input::isKeyJustPressed(Key::Space) && isOnFloor()) {
            velocity.y = jumpVelocity;
        }

        // Horizontal Movement
        float horizontal = Input::getAxis(Key::A, Key::D);
        velocity.x = horizontal * speed;

        // Move and slide with Box2D collision resolution
        moveAndSlide();
    }
};
```

---

## 3. Static Platforms & Area2D Trigger Pickups

```cpp
// 1. Static Ground Platform
auto ground = app.spawn<StaticBody2D>("Ground");
ground->setPosition({640.0f, 680.0f});
ground->spawnChild<CollisionShape2D>(Vector2(1280.0f, 40.0f));
ground->spawnChild<MeshInstance2D>(Vector2(1280.0f, 40.0f), Color::DARK_GRAY);

// 2. Collectible Coin Trigger (Area2D)
auto coin = app.spawn<Area2D>("Coin");
coin->setPosition({350.0f, 440.0f});
coin->spawnChild<CollisionShape2D>(15.0f); // 15px radius circle
coin->spawnChild<MeshInstance2D>(MeshInstance2D::createCircle(15.0f, Color::GOLD));

coin->onBodyEntered = [](Node2D *body) {
    if (body->name == "Player") {
        // Collect coin
    }
};

// 3. Dynamic Physics Crate (RigidBody2D)
auto crate = app.spawn<RigidBody2D>("Crate");
crate->setPosition({400.0f, 150.0f});
crate->restitution = 0.5f; // Bouncy
crate->spawnChild<CollisionShape2D>(Vector2(45.0f, 45.0f));
crate->spawnChild<MeshInstance2D>(Vector2(45.0f, 45.0f), Color::BROWN);
```
