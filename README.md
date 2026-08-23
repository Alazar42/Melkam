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

> **Note for Developers**: The **2D Engine is ready to use** and actively evolving with new features! It currently provides full 2D scene tree hierarchy, Box2D physics simulation, canvas UI system, 9-slice rendering, theme inheritance, and subpixel font engines. Work on the **3D Engine will start soon**.

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
