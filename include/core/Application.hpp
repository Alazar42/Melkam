#pragma once

#include "ECS.hpp"
#include "audio/Audio.hpp"
#include "core/SceneTree.hpp"
#include "input.hpp"
#include "renderers/Renderer2D.hpp"
#include "systems/Systems2D.hpp"
#include "time.hpp"
#include "window.hpp"
#include <memory>
#include <string>

// Master Application / Game Engine Runtime (inspired by Godot's MainLoop & Engine architecture).
class Application {
public:
  // Direct constructor accepting title, dimensions, maximized, fullscreen, and minimized flags
  explicit Application(const std::string &title = "MelkamEngine",
                       uint32_t width = 1280, uint32_t height = 720,
                       bool maximized = false, bool fullscreen = false,
                       bool minimized = false,
                       const Color &clearColor = Color::from_rgba8(20, 20, 25),
                       bool vsync = true, bool resizable = true) {
    m_props.title = title;
    m_props.width = width;
    m_props.height = height;
    m_props.maximized = maximized;
    m_props.fullscreen = fullscreen;
    m_props.minimized = minimized;
    m_props.clearColor = clearColor;
    m_props.vsync = vsync;
    m_props.resizable = resizable;
    m_tree = std::make_shared<SceneTree>();
  }

  // Explicit WindowProps constructor
  explicit Application(const WindowProps &props) : m_props(props) {
    m_tree = std::make_shared<SceneTree>();
  }

  virtual ~Application() = default;

  virtual void onInit() {}
  virtual void onUpdate(float) {}
  virtual void onRender() {}
  virtual void onShutdown() {}

  // Attaches an existing node directly to the active scene root.
  void addNode(std::shared_ptr<Node> node) {
    if (m_tree && m_tree->getRoot()) {
      m_tree->getRoot()->addChild(std::move(node));
    }
  }

  // Alias for addNode.
  void addChild(std::shared_ptr<Node> node) {
    addNode(std::move(node));
  }

  // Instantiates a Node and attaches it to the scene root in a single call.
  template <typename T, typename... Args>
  std::shared_ptr<T> spawn(Args &&...args) {
    return m_tree->getRoot()->spawnChild<T>(std::forward<Args>(args)...);
  }

  // Starts and runs the main engine loop.
  void run() {
    m_window = std::make_unique<Window>(m_props);
    m_window->setEventCallback([this](const SDL_Event &sdlEvent) {
      if (m_tree) {
        InputEvent event = InputEvent::fromSDL(sdlEvent);
        m_tree->input(event);
        if (!event.isHandled()) {
          m_tree->unhandledInput(event);
        }
      }
    });
    Renderer2D::init(*m_window);
    Audio::init();

    // User initialization
    onInit();

    while (m_window->isOpen()) {
      m_window->pollEvents();
      Audio::update();

      if (!m_window->isOpen()) break;

      float dt = Time::getDeltaTime();

      // 1. Fixed Timestep Physics Cycle
      while (Time::shouldDoFixedUpdate()) {
        float fixedDt = Time::getFixedDeltaTime();
        m_tree->physicsProcess(fixedDt);
      }

      // 2. Variable Frame Update Cycle
      m_tree->process(dt);
      Systems2D::updateMovement(dt);
      onUpdate(dt);

      // 3. Render Pass
      m_window->clear();
      Renderer2D::begin();

      m_tree->draw();
      Systems2D::render();
      onRender();

      Renderer2D::end();
      m_window->present();
    }

    onShutdown();
    Audio::shutdown();
  }

  // Quits the application.
  void quit() {
    if (m_window) {
      m_window->close();
    }
  }

  // Sets window title dynamically.
  void setTitle(const std::string &title) {
    if (m_window) {
      m_window->setTitle(title);
    }
  }

  // Sets window clear background color.
  void setClearColor(const Color &color) {
    if (m_window) {
      m_window->setClearColor(color);
    }
  }

  // Fullscreen controls
  void setFullscreen(bool fullscreen) {
    if (m_window) {
      m_window->setFullscreen(fullscreen);
    }
    m_props.fullscreen = fullscreen;
  }

  // Window state controls
  void maximize() {
    if (m_window) {
      m_window->maximize();
    }
    m_props.maximized = true;
  }

  void minimize() {
    if (m_window) {
      m_window->minimize();
    }
    m_props.minimized = true;
  }

  void restore() {
    if (m_window) {
      m_window->restore();
    }
    m_props.maximized = false;
    m_props.minimized = false;
  }

  bool isFullscreen() const { return m_window ? m_window->isFullscreen() : m_props.fullscreen; }
  bool isMaximized() const { return m_window ? m_window->isMaximized() : m_props.maximized; }
  bool isMinimized() const { return m_window ? m_window->isMinimized() : m_props.minimized; }

  // Returns reference to active Window.
  Window &getWindow() { return *m_window; }

  // Returns shared pointer to active SceneTree.
  std::shared_ptr<SceneTree> getTree() const { return m_tree; }

private:
  WindowProps m_props;
  std::unique_ptr<Window> m_window = nullptr;
  std::shared_ptr<SceneTree> m_tree = nullptr;
};
