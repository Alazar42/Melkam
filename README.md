# Melkam Engine

Melkam is a C++17 game engine library built with raylib and raygui. It provides a Godot-inspired UI system (Control, containers, anchors), scene management, and 2D/3D gameplay helpers.

This is a library and a growing game engine.

Proudly being developed by an Ethiopian developer, with the goal of becoming the first widely recognized game engine from Ethiopia.

## Features

- Godot-like UI system (Control nodes, anchors/offsets, VBox/HBox, ScrollContainer)
- Raygui-backed widgets (buttons, toggles, sliders, lists, tabs, message boxes, input boxes)
- UI layout sizing with size flags (fill/expand)
- UI theming with a custom Melkam theme and font override
- Text input focus handling and placeholders
- Scene builder/rebuild workflow with scene switching
- Area2D/Area3D trigger signals and collision callbacks
- 2D movement + collision via fixed-step physics (AABB, static bodies, layers/masks)
- 3D character movement with MoveAndSlide + floor/wall/ceiling detection
- Window configuration flags (resizable, fullscreen, vsync, HiDPI)

## Requirements

- CMake 3.20+
- A C++17 compiler (MSYS2 MinGW-w64, clang, or MSVC)
- raylib (headers and libraries available to your compiler)

## Setup (Windows + MSYS2)

1. Install MSYS2 and the MinGW-w64 toolchain.
2. Install CMake.
3. Install raylib for MinGW-w64 (or build it from source).

## ECS Overview

Melkam uses a lightweight ECS (Entity-Component-System) model:

- `Scene` owns entities and components.
- `Entity` is a handle with helper methods (add/get components).
- `System` updates entities each frame.

Small example:

```cpp
auto entity = scene.createEntity("Player");
entity.addComponent<TransformComponent>();
entity.addComponent<CharacterBody3DComponent>();
entity.addComponent<Velocity3DComponent>();

for (auto &e : scene.view<CharacterBody3DComponent, Velocity3DComponent>())
{
	MoveAndSlide3D(e, dt);
}
```

## 2D and 3D (What Works Today)

### 2D

- `Register2DSystems(scene)` wires in input + fixed-step physics.
- `CharacterController2DComponent` + `Input2DComponent` drive movement.
- Collisions are AABB-based with `BoxShape2DComponent`, static bodies, and layer/mask filtering.

Minimal example:

```cpp
Register2DSystems(scene);

auto player = scene.createEntity("Player2D");
player.addComponent<TransformComponent>();
player.addComponent<BoxShape2DComponent>();
player.addComponent<Velocity2DComponent>();
player.addComponent<Input2DComponent>();
player.addComponent<CharacterController2DComponent>();
```

### 3D

- `MoveAndSlide3D()` supports character movement with floor/wall/ceiling detection.
- `ColliderComponent` + shape components enable collisions.
- `Area3DComponent` provides trigger zones with `ConnectAreaBodyEntered/Exited`.

Minimal example:

```cpp
auto player = scene.createEntity("Player3D");
player.addComponent<TransformComponent>();
player.addComponent<CharacterBody3DComponent>();
player.addComponent<Velocity3DComponent>();
player.addComponent<ColliderComponent>();
player.addComponent<BoxShape3DComponent>();

MoveAndSlide3D(player, dt);
```

## Customize UI Theme

Theme configuration lives in `SetUiThemeMelkam()` inside `src/Melkam/ui/Ui.cpp`:

- Adjust the color palette via `GuiSetStyle(...)` calls.
- Change the UI font in `applyMelkamFont()` by pointing to a different file.
- Override styles per node by attaching `UiStyleComponent` to a control or its parent.

## Project Structure

```
include/   Engine headers and components
src/       Engine and sample app code
docs/      Sample code snippets and references
fonts/     UI fonts
```

## License

TBD
