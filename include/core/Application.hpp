#pragma once

#include "ECS.hpp"
#include "audio/Audio.hpp"
#include "core/BootSplash.hpp"
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
  BootSplashConfig bootSplash{};
  // Direct constructor accepting title, dimensions, maximized, stretch mode, and stretch aspect
  explicit Application(const std::string &title = "MelkamEngine",
                       uint32_t width = 1280, uint32_t height = 720,
                       bool maximized = false,
                       StretchMode stretchMode = StretchMode::CanvasItems,
                       StretchAspect stretchAspect = StretchAspect::Keep,
                       bool fullscreen = false,
                       bool minimized = false,
                       const Color &clearColor = Color::from_rgba8(20, 20, 25),
                       bool vsync = true, bool resizable = true) {
    m_props.title = title;
    m_props.width = width;
    m_props.height = height;
    m_props.designWidth = width;
    m_props.designHeight = height;
    m_props.stretchMode = stretchMode;
    m_props.stretchAspect = stretchAspect;
    m_props.maximized = maximized;
    m_props.fullscreen = fullscreen;
    m_props.minimized = minimized;
    m_props.clearColor = clearColor;
    m_props.vsync = vsync;
    m_props.resizable = resizable;
    s_instance = this;
    m_tree = std::make_shared<SceneTree>();
  }

  // Explicit WindowProps constructor
  explicit Application(const WindowProps &props) : m_props(props) {
    s_instance = this;
    m_tree = std::make_shared<SceneTree>();
  }

  virtual ~Application() {
    if (s_instance == this) s_instance = nullptr;
  }

  static Application *getInstance() { return s_instance; }


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

  // Attaches an existing Node to the scene root.
  std::shared_ptr<Node> addChild(std::shared_ptr<Node> node) {
    addNode(node);
    return node;
  }

  // Instantiates a Node and attaches it to the scene root in a single call.
  template <typename T, typename... Args>
  std::shared_ptr<T> addChild(Args &&...args) {
    return m_tree->getRoot()->addChild<T>(std::forward<Args>(args)...);
  }

  // Alias for addChild.
  template <typename T, typename... Args>
  std::shared_ptr<T> spawn(Args &&...args) {
    return addChild<T>(std::forward<Args>(args)...);
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
    PhysicsServer2D::init();

    // Godot-style Boot Splash Presentation
    BootSplash::run(*m_window, bootSplash);

    // User initialization
    onInit();

    while (m_window->isOpen()) {
      m_window->pollEvents();
      Audio::update();

      if (!m_window->isOpen()) break;

      float dt = Time::getDeltaTime();

      // 1. Fixed Timestep Physics Cycle (Node Physics Process -> Box2D Simulation Step)
      while (Time::shouldDoFixedUpdate()) {
        float fixedDt = Time::getFixedDeltaTime();
        m_tree->savePhysicsTransformState();
        m_tree->physicsProcess(fixedDt);
        PhysicsServer2D::step(fixedDt);
      }

      // 2. Variable Frame Update Cycle
      m_tree->process(dt);
      Systems2D::updateMovement(dt);
      onUpdate(dt);

      // 3. Render Pass with Subpixel Physics Transform Interpolation
      float alpha = Time::getPhysicsInterpolationAlpha();
      m_window->clear();
      Renderer2D::begin();

      m_tree->draw(alpha);
      Systems2D::render();
      onRender();

      Renderer2D::end();
      m_window->present();
    }



    onShutdown();
    PhysicsServer2D::shutdown();
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

  // Configures Godot-style stretch mode and aspect ratio
  void setStretch(StretchMode mode, StretchAspect aspect = StretchAspect::Keep) {
    m_props.stretchMode = mode;
    m_props.stretchAspect = aspect;
    if (m_window) {
      m_window->setStretch(mode, aspect);
    }
  }

  void setStretchMode(StretchMode mode) {
    m_props.stretchMode = mode;
    if (m_window) {
      m_window->setStretchMode(mode);
    }
  }

  void setStretchAspect(StretchAspect aspect) {
    m_props.stretchAspect = aspect;
    if (m_window) {
      m_window->setStretchAspect(aspect);
    }
  }

  void setDesignResolution(uint32_t width, uint32_t height) {
    m_props.designWidth = width;
    m_props.designHeight = height;
    if (m_window) {
      m_window->setDesignResolution(width, height);
    }
  }

  // Configures Godot-style Boot Splash
  void setBootSplash(const BootSplashConfig &config) {
    bootSplash = config;
  }

  void setBootSplashEnabled(bool enabled) {
    bootSplash.enabled = enabled;
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
  inline static Application *s_instance = nullptr;
  WindowProps m_props;
  std::unique_ptr<Window> m_window = nullptr;
  std::shared_ptr<SceneTree> m_tree = nullptr;
};

