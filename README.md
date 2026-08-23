<p align="center">
  <img src="logo.png" alt="Melkam Engine Logo" width="180">
</p>

<h1 align="center">Melkam Engine</h1>

<p align="center">
  <strong>A modern, lightweight C++ game engine framework inspired by the Godot architecture.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++20">
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="MIT License">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg" alt="Cross Platform">
</p>

---

## Overview

**Melkam Engine** is a cross-platform 2D/3D game engine and library written in modern C++20. Designed with a clean, ergonomic node tree hierarchy inspired by Godot 4, it provides developers with modular building blocks for scene management, rendering, physics, and UI theming.

## Key Features

- **Godot-Inspired Node Hierarchy**: Flexible scene tree with `Node`, `Node2D`, `CanvasItem`, and `Control` bases.
- **Type-Safe Signals & Slots**: Event-driven communication with lambda and member function slots.
- **2D Physics Engine**: Rigid bodies, character bodies (`moveAndSlide`), static colliders, and area triggers powered by Box2D.
- **Canvas UI & Theme Subsystem**: Comprehensive UI controls (`Button`, `Slider`, `OptionButton`, `ProgressBar`, `Panel`, `Label`, `TextureRect`) with Godot-style subtree theme inheritance.
- **9-Slice & Textured UI**: Dynamic border patch slicing for scalable UI cards, dialog frames, and button skins.
- **Subpixel Font Engine**: Crisp, multi-resolution TrueType font rendering with oversampled glyph packing.
- **Hardware-Accelerated 2D Renderer**: Fast batch and immediate rendering powered by SDL3.

## License

This project is open-source and licensed under the [MIT License](LICENSE) &copy; 2026 **Mickyas Tesfaye**.
