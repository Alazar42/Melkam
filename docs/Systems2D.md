# MelkamEngine 2D ECS Systems & Components Documentation

MelkamEngine bridges the **EnTT-powered ECS** directly with **Renderer2D**, **Input**, and **Time** using standard 2D components and systems.

Include headers:
```cpp
#include "ECS.hpp"
#include "components/Components2D.hpp"
#include "systems/Systems2D.hpp"
```

---

## 1. Quick Start (ECS + Renderer + Input)

```cpp
#include "Window.hpp"
#include "ECS.hpp"
#include "components/Components2D.hpp"
#include "systems/Systems2D.hpp"

int main() {
    WindowProps props;
    props.title = "MelkamEngine - ECS 2D Demo";
    Window window(props);
    Renderer2D::init(window);

    // 1. Create a Player Entity with ECS
    auto player = Entity::create();
    player.emplace<Tag>("Player");
    player.emplace<Transform2D>(Vector2(640.0f, 360.0f));
    player.emplace<Velocity2D>();
    player.emplace<PlayerController>(400.0f); // 400 px/sec
    player.emplace<Shape2D>(Shape2D::createRectangle({50.0f, 50.0f}, Color::GOLD));

    // 2. Create an Asteroid Entity with auto-rotation and velocity
    auto asteroid = Entity::create();
    asteroid.emplace<Transform2D>(Vector2(300.0f, 200.0f));
    asteroid.emplace<Velocity2D>(Vector2(50.0f, 30.0f)); // Linear velocity
    asteroid.emplace<Rotator>(1.5f);                     // Rotates 1.5 rad/s
    asteroid.emplace<Shape2D>(Shape2D::createTriangle({-25, 25}, {25, 25}, {0, -25}, Color::CORAL));

    // 3. Main Game Loop
    while (window.isOpen()) {
        window.pollEvents();

        float dt = Time::getDeltaTime();

        // Run all simulation systems (Input -> Rotators -> Movement)
        Systems2D::update(dt);

        // Render pass
        window.clear(Color::DARK_GRAY);
        Renderer2D::begin();

        // Automatically renders all ECS entities with Shape2D & Sprite2D!
        Systems2D::render();

        Renderer2D::end();
        window.present();
    }

    return 0;
}
```

---

## 2. Standard 2D Components

| Component | Description | Fields |
| :--- | :--- | :--- |
| `Tag` | Entity name / label | `std::string name` |
| `Transform2D` | 2D position, rotation, scale, and zIndex | `Vector2 position`, `float rotation`, `Vector2 scale`, `int zIndex` |
| `Velocity2D` | Linear and angular velocity | `Vector2 linear`, `float angular` |
| `PlayerController` | Marks entity to be driven by WASD / Arrows | `float speed` |
| `Rotator` | Automatically spins entity over time | `float speed` (rad/s) |
| `Shape2D` | Geometric shape render component | Rectangle, Circle, Triangle, Polygon, `Color color`, `bool filled` |
| `Sprite2D` | Texture render component | `Texture2D`, `Color tint`, `flipH`, `flipV`, `origin` |

---

## 3. Standard 2D Systems (`Systems2D`)

- `Systems2D::updatePlayerInput()`: Reads WASD / Arrows from `Input` and updates `Velocity2D` on `PlayerController` entities.
- `Systems2D::updateMovement(float dt)`: Integrates `Velocity2D` into `Transform2D`.
- `Systems2D::updateRotators(float dt)`: Applies angular spin from `Rotator` into `Transform2D.rotation`.
- `Systems2D::render()`: Queries all `Transform2D + Shape2D` and `Transform2D + Sprite2D` entities and draws them to `Renderer2D`.
- `Systems2D::update(float dt)`: Runs all update systems in optimal order.
