<p align="center">
  <img src="logo.png" alt="Melkam Engine Logo" width="180">
</p>

<h1 align="center">Melkam Engine</h1>

<p align="center">
  <strong>A modern, lightweight C++ game engine framework inspired by the Godot architecture.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/2D%20Engine-Ready%20%26%20Active-brightgreen.svg" alt="2D Ready & Active">
  <img src="https://img.shields.io/badge/3D%20Engine-Coming%20Soon-orange.svg" alt="3D Coming Soon">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++20">
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="MIT License">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg" alt="Cross Platform">
</p>

---

## Current Status & Roadmap

> **Note for Developers**: The **2D Engine is ready to use** and actively expanding! It currently provides full SceneTree node hierarchies, 2D physics bodies with collision shapes, Canvas UI nodes with Theme subtree styling, 9-slice patch margins, and subpixel TrueType font rendering. Work on the **3D Engine will start soon**.

## Core Engine Architecture

- **SceneTree & Node Hierarchy**: Base `Node`, `Node2D`, `CanvasItem`, and `Control` classes with recursive `onReady`, `onProcess`, `onPhysicsProcess`, `onDraw`, and `onInput` lifecycles.
- **Type-Safe Signals**: Godot-style `Signal<Args...>` event dispatching supporting lambda and member function slots.
- **2D Physics Nodes (Box2D)**: `CharacterBody2D` (`moveAndSlide`), `RigidBody2D`, `StaticBody2D`, `Area2D`, and `CollisionShape2D` (box & circle shapes).
- **Canvas UI & Theme Subsystem**: Full suite of `Control` nodes (`Button`, `HSlider`, `VSlider`, `OptionButton`, `ProgressBar`, `Panel`, `Label`, `TextureRect`, `BoxContainer`, `GridContainer`) driven by hierarchical `Theme` configurations.
- **9-Slice Patch Textures**: Seamless UI skinning with configurable patch margins on `Panel` and `Button` nodes.
- **Subpixel Font Engine**: Multi-resolution TrueType font rendering with oversampled glyph packing and fallback bitmap fonts.
- **2D Hardware Rendering**: Immediate and batched screen/world drawing via `Renderer2D`, `Sprite2D`, `Texture2D`, and `Camera2D` on top of SDL3.

## License

This project is open-source and licensed under the [MIT License](LICENSE) &copy; 2026 **Mickyas Tesfaye**.
