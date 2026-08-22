# MSL Color Module

The `Color` class represents an RGBA color with floating-point precision (0.0 to 1.0 per channel) and provides extensive utilities for color space transformations, blending, interpolation, and formatting.

---

## Key Features

- **Standard Channels**: `r`, `g`, `b`, `a` floating-point fields.
- **Factory Parsers**: Hex strings (`#RRGGBB`, `#RRGGBBAA`, `#RGB`, `#RGBA`), 32-bit/64-bit integer masks, and HSV models.
- **Predefined Palette**: 30+ named color constants (e.g. `Color::RED`, `Color::BLUE`, `Color::GOLD`, `Color::CLEAR`).
- **Color Manipulation**: `inverted()`, `lightened()`, `darkened()`, `lerp()`, `blend()`, `clamp()`, `linear_to_srgb()`, `srgb_to_linear()`.
- **HSV & Luminance**: Computed `get_h()`, `get_s()`, `get_v()`, and `get_luminance()`.
- **Safe Parsing & Indexing**: Throws `InvalidColorFormat` and `ColorIndexOutOfRange` (defined in `exceptions.hpp`).

---

## Quick Tutorial & Examples

### 1. Creation & Parsing

```cpp
#include "color/Color.hpp"
#include <iostream>

int main() {
    // Direct constructor (R, G, B, A)
    Color custom(1.0f, 0.5f, 0.2f, 1.0f);

    // From HTML hex string
    Color orange = Color::html("#ff8800");
    Color semi_blue = Color::html("#0000ff80");

    // From 8-bit RGBA (0 - 255)
    Color byte_color = Color::from_rgba8(255, 128, 0, 255);

    // From HSV (Hue: 0.0-1.0, Sat: 0.0-1.0, Val: 0.0-1.0)
    Color green = Color::from_hsv(0.333f, 1.0f, 1.0f);

    // Named constants
    Color gold = Color::GOLD;

    std::cout << "Custom Color: " << custom << "\n";
    std::cout << "Orange: " << orange << "\n";
    return 0;
}
```

### 2. Manipulation, Blending & Interpolation

```cpp
#include "color/Color.hpp"
#include <iostream>

int main() {
    Color red = Color::RED;

    // Lighten and darken
    Color pink = red.lightened(0.5f);
    Color dark_red = red.darkened(0.3f);

    // Invert
    Color cyan = red.inverted();

    // Linear interpolation (lerp)
    Color mid = Color::BLACK.lerp(Color::WHITE, 0.5f); // 50% gray

    // Alpha blending
    Color bg(1.0f, 1.0f, 1.0f, 1.0f); // White background
    Color fg(1.0f, 0.0f, 0.0f, 0.5f); // 50% transparent red
    Color blended = fg.blend(bg);

    std::cout << "Blended: " << blended << "\n";
    return 0;
}
```

### 3. Conversions & Output

```cpp
#include "color/Color.hpp"
#include <iostream>

int main() {
    Color c(1.0f, 0.5f, 0.0f, 1.0f);

    // Output as HTML hex string
    std::cout << "HTML Hex: " << c.to_html(true) << "\n"; // "ff8000ff"

    // Convert to packed 32-bit integer formats
    uint32_t rgba32 = c.to_rgba32();
    uint32_t argb32 = c.to_argb32();

    // Get color attributes
    std::cout << "Luminance: " << c.get_luminance() << "\n";
    std::cout << "Hue: " << c.get_h() << "\n";
    return 0;
}
```

---

## API Summary

| Method / Constant | Description |
| :--- | :--- |
| `Color::from_rgba8(r, g, b, a)` | Constructs from 8-bit integer channels (0–255). |
| `Color::from_hsv(h, s, v, a)` | Constructs from HSV floats. |
| `Color::html(hex_string)` | Parses `#RGB`, `#RGBA`, `#RRGGBB`, `#RRGGBBAA`. |
| `Color::hex(uint32)` / `hex64(uint64)` | Constructs from integer hex values. |
| `to_html(with_alpha)` | Formats color as hexadecimal string. |
| `to_rgba32()` / `to_argb32()` | Packs color into 32-bit unsigned integers. |
| `get_h()`, `get_s()`, `get_v()` | Returns Hue, Saturation, and Value components. |
| `get_luminance()` | Returns perceived luminance. |
| `inverted()` | Inverts RGB channels `(1 - c)`. |
| `lightened(amount)` / `darkened(amount)` | Brightens or dims the color. |
| `lerp(to, weight)` | Linearly interpolates towards target color. |
| `blend(over)` | Performs Porter-Duff alpha blending. |
| `linear_to_srgb()` / `srgb_to_linear()` | Performs gamma conversion. |
| `operator[](0..3)` | Channel access (0: `r`, 1: `g`, 2: `b`, 3: `a`). |
