# MSL Vectors Module

The `vectors` module provides high-performance 2D and 3D geometric vector types (`Vector2` and `Vector3`) with arithmetic operators, geometric projections, rotations, interpolation, and bounds-checking.

---

## Key Features

- **`Vector2`**: 2D coordinate system with `x`, `y`. Angles, 2D cross product, orthogonal vectors, 2D rotations.
- **`Vector3`**: 3D coordinate system with `x`, `y`, `z`. 3D cross product, spherical linear interpolation (`slerp`), Rodrigues' rotation around arbitrary 3D axes.
- **Comprehensive Vector Math**: `dot()`, `cross()`, `normalized()`, `distance_to()`, `direction_to()`, `reflect()`, `bounce()`, `project()`, `slide()`.
- **Interpolation & Snapping**: `lerp()`, `slerp()`, `move_toward()`, `clamp()`, `limit_length()`, `snapped()`, `posmod()`.
- **Standard Constants**: `ZERO`, `ONE`, `UP`, `DOWN`, `LEFT`, `RIGHT`, `FORWARD`, `BACK`, `INF_VEC`.
- **Safety**: Safe indexing (`[0..1]` for `Vector2`, `[0..2]` for `Vector3`) throwing `Vector2IndexOutOfRange` and `Vector3IndexOutOfRange` (defined in `exceptions.hpp`).

---

## Quick Tutorial & Examples

### 1. Vector2 Basics & Arithmetic

```cpp
#include "vectors/Vector2.hpp"
#include <iostream>

int main() {
    Vector2 pos(10.0f, 20.0f);
    Vector2 velocity(2.0f, 5.0f);

    // Vector addition and scaling
    Vector2 new_pos = pos + velocity * 0.5f;
    std::cout << "New Position: " << new_pos << "\n"; // (11, 22.5)

    // Length and normalization
    Vector2 dir(3.0f, 4.0f);
    std::cout << "Length: " << dir.length() << "\n"; // 5
    std::cout << "Normalized: " << dir.normalized() << "\n"; // (0.6, 0.8)

    // Dot and Cross product
    Vector2 a(1.0f, 0.0f);
    Vector2 b(0.0f, 1.0f);
    std::cout << "Dot: " << a.dot(b) << "\n"; // 0
    std::cout << "Cross: " << a.cross(b) << "\n"; // 1
    return 0;
}
```

### 2. 2D Rotations, Angles, and Distance

```cpp
#include "vectors/Vector2.hpp"
#include <iostream>

int main() {
    Vector2 p1(0.0f, 0.0f);
    Vector2 p2(10.0f, 10.0f);

    // Distance and direction
    std::cout << "Distance: " << p1.distance_to(p2) << "\n";
    std::cout << "Direction: " << p1.direction_to(p2) << "\n";

    // 2D Rotation (by radians)
    Vector2 forward(1.0f, 0.0f);
    Vector2 rotated = forward.rotated(1.5707963f); // 90 degrees in radians
    std::cout << "Rotated 90 deg: " << rotated << "\n"; // Approx (0, 1)

    // Linear interpolation
    Vector2 interpolated = p1.lerp(p2, 0.5f);
    std::cout << "Halfway: " << interpolated << "\n"; // (5, 5)
    return 0;
}
```

### 3. Vector3 Basics & 3D Math

```cpp
#include "vectors/Vector3.hpp"
#include <iostream>

int main() {
    Vector3 right = Vector3::RIGHT; // (1, 0, 0)
    Vector3 up = Vector3::UP;       // (0, 1, 0)

    // 3D Cross Product
    Vector3 normal = right.cross(up);
    std::cout << "Right x Up = " << normal << "\n"; // (0, 0, 1) [FORWARD / BACK axis]

    // Rodrigues' Rotation around arbitrary axis
    Vector3 point(1.0f, 0.0f, 0.0f);
    Vector3 axis(0.0f, 0.0f, 1.0f); // Rotate around Z
    Vector3 rotated = point.rotated(axis, 1.5707963f);
    std::cout << "Rotated around Z: " << rotated << "\n";

    // Spherical Linear Interpolation (slerp)
    Vector3 slerped = right.slerp(up, 0.5f);
    std::cout << "Slerp 50%: " << slerped << "\n";
    return 0;
}
```

### 4. Bouncing, Reflection, and Projection

```cpp
#include "vectors/Vector2.hpp"
#include "vectors/Vector3.hpp"
#include <iostream>

int main() {
    // 2D Reflection off a surface normal
    Vector2 motion(1.0f, -1.0f);
    Vector2 normal(0.0f, 1.0f);
    Vector2 bounce = motion.bounce(normal);
    std::cout << "Bounced: " << bounce << "\n"; // (1, 1)

    // Projection onto another vector
    Vector2 v(5.0f, 5.0f);
    Vector2 target(10.0f, 0.0f);
    Vector2 projected = v.project(target);
    std::cout << "Projected: " << projected << "\n"; // (5, 0)
    return 0;
}
```

---

## API Summary

| Feature / Method | `Vector2` | `Vector3` | Description |
| :--- | :--- | :--- | :--- |
| Coordinates | `x`, `y` | `x`, `y`, `z` | Public floating-point coordinates. |
| `length()` / `length_squared()` | Yes | Yes | Computes magnitude of vector. |
| `normalized()` | Yes | Yes | Unit length vector in same direction. |
| `dot(other)` | Yes | Yes | Scalar product. |
| `cross(other)` | `float` | `Vector3` | 2D perpendicular scalar / 3D cross vector. |
| `angle_to(other)` | Yes | Yes | Angular difference in radians. |
| `distance_to(other)` | Yes | Yes | Euclidean distance. |
| `direction_to(other)` | Yes | Yes | Unit vector pointing from `*this` to `other`. |
| `reflect(n)` / `bounce(n)` | Yes | Yes | Reflection across normal vector. |
| `project(b)` / `slide(n)` | Yes | Yes | Vector projection and plane sliding. |
| `lerp(to, weight)` | Yes | Yes | Linear interpolation. |
| `slerp(to, weight)` | Yes | Yes | Spherical linear interpolation. |
| `move_toward(to, delta)` | Yes | Yes | Moves toward target by fixed delta distance. |
| `rotated(angle / axis, angle)`| Yes | Yes | Rotates vector in 2D or around 3D axis. |
| `clamp(min, max)` | Yes | Yes | Component-wise clamping. |
| `limit_length(max_len)` | Yes | Yes | Clamps maximum magnitude. |
| `operator[](index)` | `0..1` | `0..2` | Bounds-checked channel indexing. |
