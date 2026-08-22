# MelkamEngine Texture2D Documentation

MelkamEngine provides a GPU-backed `Texture2D` representation for loading and rendering 2D image assets.

Include header:
```cpp
#include "renderers/Texture2D.hpp"
```

---

## 1. Loading Textures

```cpp
#include "renderers/Texture2D.hpp"

// Load from file:
Texture2D texture("assets/sprite.bmp");

if (texture.isValid()) {
    std::cout << "Texture loaded: " << texture.getWidth() << "x" << texture.getHeight() << "\n";
}
```

---

## 2. Programmatic / Solid Textures

Create solid color test textures on the fly:

```cpp
auto redTex = Texture2D::createSolid(64, 64, Color::RED);
auto blueTex = Texture2D::createSolid(128, 128, Color::AQUA);
```

---

## 3. Drawing Textures with `Renderer2D`

```cpp
#include "renderers/Renderer2D.hpp"

Renderer2D::begin();

// Draw texture at position with size and optional tint
Renderer2D::drawTexture(texture, {100.0f, 100.0f}, {64.0f, 64.0f}, Color::WHITE);

// Draw with rotation (radians) and flipping
Renderer2D::drawTexture(texture, {300.0f, 200.0f}, {128.0f, 128.0f}, Color::CORAL, 0.785f, true /*flipH*/, false);

Renderer2D::end();
```
