# MelkamEngine Time Documentation

MelkamEngine provides a high-precision Time subsystem and Timer utility for delta time, frame rates, time scaling, and physics accumulators.

Include header:
```cpp
#include "time.hpp"
```

---

## 1. Quick Start Example

```cpp
#include "Window.hpp"
#include "input.hpp"
#include "time.hpp"
#include <iostream>

int main() {
    WindowProps props;
    props.title = "MelkamEngine - Time Demo";
    Window window(props);

    Vector2 playerPos(100.0f, 100.0f);
    float playerSpeed = 300.0f; // pixels per second

    while (window.isOpen()) {
        window.pollEvents(); // Automatically updates Time and Input

        float dt = Time::getDeltaTime(); // Frame time in seconds (e.g. ~0.016s for 60 FPS)

        // Frame-rate independent movement
        Vector2 inputDir = Input::getVector(Key::A, Key::D, Key::S, Key::W);
        playerPos += inputDir * playerSpeed * dt;

        // Display real-time FPS in window title every few frames
        if (Time::getFrameCount() % 60 == 0) {
            window.setTitle("FPS: " + std::to_string(static_cast<int>(Time::getFPS())));
        }

        window.clear(Color::DARK_GRAY);
        window.present();
    }

    return 0;
}
```

---

## 2. Time API Reference

| Method | Return Type | Description |
| :--- | :--- | :--- |
| `Time::getDeltaTime()` | `float` | Elapsed seconds since previous frame (scaled) |
| `Time::getDeltaTimeMs()` | `float` | Elapsed milliseconds since previous frame |
| `Time::getUnscaledDeltaTime()` | `float` | Raw elapsed seconds ignoring pause and timeScale |
| `Time::getTime()` | `float` | Total accumulated game time in seconds |
| `Time::getUnscaledTime()` | `float` | Real wall-clock seconds since engine startup |
| `Time::getFPS()` | `float` | Current smoothed frames per second |
| `Time::getFrameCount()` | `uint64_t` | Total frames rendered |
| `Time::setTimeScale(float scale)` | `void` | Set time speed (`0.5f` = half speed, `2.0f` = double speed) |
| `Time::getTimeScale()` | `float` | Get current time scale multiplier |
| `Time::setPaused(bool paused)` | `void` | Pauses game time (`deltaTime` becomes 0) |
| `Time::isPaused()` | `bool` | Check if game time is paused |
| `Time::setFixedDeltaTime(float dt)` | `void` | Set fixed physics step (default `1.0f / 60.0f`) |
| `Time::getFixedDeltaTime()` | `float` | Get fixed physics step duration |
| `Time::shouldDoFixedUpdate()` | `bool` | True when accumulator has enough time for a physics step |

---

## 3. Fixed Timestep Loop (Physics)

For physics and deterministic logic:

```cpp
while (Time::shouldDoFixedUpdate()) {
    float fixedDt = Time::getFixedDeltaTime();
    physicsWorld.step(fixedDt);
}
```

---

## 4. Stopwatch / Timer Utility (`Timer`)

The `Timer` class makes it trivial to profile code or handle gameplay cooldowns/delays:

```cpp
Timer cooldownTimer;

// In game loop:
if (Input::isMouseButtonJustPressed(MouseButton::Left)) {
    if (cooldownTimer.hasElapsed(0.5f)) { // 500ms cooldown
        // Shoot!
        cooldownTimer.reset();
    }
}

// Or inspect elapsed time:
float seconds = cooldownTimer.elapsed();
float millis = cooldownTimer.elapsedMs();
```
