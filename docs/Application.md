# MelkamEngine Application & Window Guide

In MelkamEngine, the `Application` class provides full control over window state, dimensions, and node management.

---

## 1. Constructor Signature

```cpp
Application(
    const std::string &title = "MelkamEngine",
    uint32_t width = 1280,
    uint32_t height = 720,
    bool maximized = false,
    bool fullscreen = false,
    bool minimized = false,
    const Color &clearColor = Color::from_rgba8(20, 20, 25),
    bool vsync = true,
    bool resizable = true
);
```

### Examples:

```cpp
// 1. Standard windowed mode
Application app("My Game", 1280, 720);

// 2. Maximized window
Application app("My Game", 1280, 720, true /*maximized*/);

// 3. Fullscreen window
Application app("My Game", 1920, 1080, false, true /*fullscreen*/);

// 4. Minimized on startup
Application app("My Game", 1280, 720, false, false, true /*minimized*/);
```

---

## 2. Runtime Window Controls

You can also dynamically change window states during the game:

```cpp
app.maximize();
app.minimize();
app.restore();
app.setFullscreen(true);

bool isMax = app.isMaximized();
bool isMin = app.isMinimized();
bool isFull = app.isFullscreen();
```

---

## 3. Adding Nodes to the Application

```cpp
// Spawn node in-place:
auto player = app.spawn<PlayerNode>();

// Add existing node pointer:
auto enemy = std::make_shared<EnemyNode>();
app.addNode(enemy);
```
