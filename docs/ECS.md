# MelkamEngine ECS Documentation

MelkamEngine provides a lightweight, header-only Entity Component System (ECS) wrapper built on top of [EnTT](https://github.com/skypjack/entt).

Include header:
```cpp
#include "ECS.hpp"
```

---

## 1. Quick Start (30 Seconds)

```cpp
#include "ECS.hpp"
#include <iostream>

struct Position { float x, y; };
struct Velocity { float dx, dy; };

int main() {
    // 1. Create an entity and attach components
    auto player = Entity::create();
    player.emplace<Position>(0.0f, 0.0f);
    player.emplace<Velocity>(5.0f, 10.0f);

    // 2. System update loop (iterates over all entities with Position & Velocity)
    for (auto [entity, pos, vel] : Entity::view<Position, Velocity>().each()) {
        pos.x += vel.dx;
        pos.y += vel.dy;
    }

    // 3. Access component
    auto &pos = player.get<Position>();
    std::cout << "Player position: (" << pos.x << ", " << pos.y << ")\n";

    // 4. Destroy when done
    player.destroy();
}
```

---

## 2. Entity Lifecycle

```cpp
// Create an entity
auto entity = Entity::create();

// Check if entity is valid & alive
if (entity.isValid()) { /* ... */ }

// Destroy entity and remove all its components
entity.destroy();

// Destroy all entities across the entire registry
Entity::clearAll();

// Get total count of alive entities
size_t aliveCount = Entity::count();
```

---

## 3. Adding & Modifying Components

```cpp
// 1. Emplace (Constructs component in-place)
entity.emplace<Position>(10.0f, 20.0f);

// 2. Emplace or Replace (Creates if missing, replaces if already present)
entity.emplaceOrReplace<Position>(30.0f, 40.0f);

// 3. Get or Emplace (Gets existing, or constructs new one if missing)
auto &pos = entity.getOrEmplace<Position>(0.0f, 0.0f);

// 4. Replace (Overwrites existing component)
entity.replace<Position>(50.0f, 60.0f);

// 5. In-Place Patch (Modifies via lambda & triggers update signals)
entity.patch<Position>([](auto &pos) {
    pos.x += 5.0f;
});
```

---

## 4. Accessing Components

### Direct Reference (`get`)
```cpp
// Single component
auto &pos = entity.get<Position>();
const auto &cpos = entity.get<Position>();

// Multiple components with structured bindings (C++17)
auto [pos, vel] = entity.get<Position, Velocity>();
```

### Safe Pointer Access (`tryGet`)
Returns `nullptr` if the component is missing (no crashes).

```cpp
// Single component
if (auto *pos = entity.tryGet<Position>()) {
    pos->x += 1.0f;
}

// Multiple components
auto [pos, vel] = entity.tryGet<Position, Velocity>();
if (pos && vel) {
    // both exist
}
```

---

## 5. Checking & Removing Components

```cpp
// Check if entity has ALL specified components
if (entity.has<Position, Velocity>()) { /* ... */ }

// Check if entity has AT LEAST ONE of the specified components
if (entity.hasAny<Position, Gravity>()) { /* ... */ }

// Safe Remove (removes if present, returns count removed)
entity.remove<Position>();

// Fast Erase (assumes component exists)
entity.erase<Position>();

// Clear a specific component type from all entities in the game
Entity::clearComponent<Position>();
```

---

## 6. Systems & Views (The Game Loop)

### Iterate All Matching Entities
```cpp
// Structured binding iteration (Entity handle + Components)
for (auto [entity, pos, vel] : Entity::view<Position, Velocity>().each()) {
    pos.x += vel.dx * dt;
    pos.y += vel.dy * dt;
}

// Lambda iteration
Entity::each<Position, Velocity>([](auto entity, auto &pos, auto &vel) {
    pos.x += vel.dx;
});
```

### Filter with Exclude
```cpp
// Process entities with Position but WITHOUT Frozen component
for (auto entity : Entity::view<Position>(entt::exclude<Frozen>)) {
    // ...
}
```

---

## 7. Reactive Signals (Event Observers)

Listen to component construction, updates, and destruction automatically:

```cpp
void onPositionAdded(entt::registry &reg, entt::entity e) {
    std::cout << "Position added to entity " << static_cast<uint32_t>(e) << "\n";
}

// Connect listeners
Entity::onConstruct<Position>().connect<&onPositionAdded>();
Entity::onUpdate<Position>().connect<&onPositionModified>();
Entity::onDestroy<Position>().connect<&onPositionRemoved>();
```

---

## 8. Global Engine Context Variables

Store global singletons (e.g. `DeltaTime`, `WindowHandle`, `InputState`) directly in the ECS:

```cpp
struct DeltaTime { float dt; };

// Set global context
Entity::setContext<DeltaTime>(0.016f);

// Access from any system or function
float dt = Entity::getContext<DeltaTime>().dt;

// Check existence
if (Entity::hasContext<DeltaTime>()) { /* ... */ }
```

---

## 9. Direct Registry Access (Advanced)

For full low-level access to raw EnTT features:

```cpp
entt::registry &reg = Entity::getRegistry();
const entt::registry &creg = Entity::getConstRegistry();
```
