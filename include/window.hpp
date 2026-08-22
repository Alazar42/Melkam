#pragma once

#include "helper/color/Color.hpp"
#include "input.hpp"
#include "time.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

// Configuration properties for window creation.
struct WindowProps {
  std::string title = "MelkamEngine";
  uint32_t width = 1280;
  uint32_t height = 720;
  bool vsync = true;
  bool resizable = true;
  bool fullscreen = false;
  Color clearColor = Color::from_rgba8(25, 25, 30);
  bool maximized = false;
  bool minimized = false;
};

// Custom Window class managing an SDL3 Window, Renderer, and Event loop.
class Window {
public:
  using EventCallbackFn = std::function<void(const SDL_Event &)>;

  // Constructs and initializes a window with the given properties.
  explicit Window(const WindowProps &props = WindowProps{}) {
    init(props);
  }

  // Destructor: cleans up window, renderer, and SDL resources.
  ~Window() {
    close();
  }

  // Non-copyable (manages unique OS handles)
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  // Move-constructible
  Window(Window &&other) noexcept
      : m_window(other.m_window), m_renderer(other.m_renderer),
        m_props(std::move(other.m_props)), m_isOpen(other.m_isOpen),
        m_eventCallback(std::move(other.m_eventCallback)) {
    other.m_window = nullptr;
    other.m_renderer = nullptr;
    other.m_isOpen = false;
  }

  // Move-assignable
  Window &operator=(Window &&other) noexcept {
    if (this != &other) {
      close();
      m_window = other.m_window;
      m_renderer = other.m_renderer;
      m_props = std::move(other.m_props);
      m_isOpen = other.m_isOpen;
      m_eventCallback = std::move(other.m_eventCallback);

      other.m_window = nullptr;
      other.m_renderer = nullptr;
      other.m_isOpen = false;
    }
    return *this;
  }

  // Initializes the SDL3 video subsystem, window, and renderer.
  bool init(const WindowProps &props = WindowProps{}) {
    m_props = props;

    // Initialize SDL3 video subsystem if not already initialized
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
      std::cerr << "[Window Error] Failed to initialize SDL3: "
                << SDL_GetError() << std::endl;
      m_isOpen = false;
      return false;
    }

    // Configure SDL3 window creation flags
    SDL_WindowFlags flags = 0;
    if (m_props.resizable) {
      flags |= SDL_WINDOW_RESIZABLE;
    }
    if (m_props.fullscreen) {
      flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (m_props.maximized) {
      flags |= SDL_WINDOW_MAXIMIZED;
    }
    if (m_props.minimized) {
      flags |= SDL_WINDOW_MINIMIZED;
    }

    // Create SDL3 Window
    m_window = SDL_CreateWindow(m_props.title.c_str(),
                                static_cast<int>(m_props.width),
                                static_cast<int>(m_props.height), flags);

    if (!m_window) {
      std::cerr << "[Window Error] Failed to create SDL3 window: "
                << SDL_GetError() << std::endl;
      m_isOpen = false;
      return false;
    }

    // Create 2D hardware-accelerated SDL3 renderer
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
      std::cerr << "[Window Warning] Failed to create hardware renderer: "
                << SDL_GetError() << std::endl;
    } else {
      // Configure VSync
      setVSync(m_props.vsync);
    }

    // Sync actual window size from OS (especially when maximized or fullscreen)
    int actualW = 0, actualH = 0;
    SDL_GetWindowSize(m_window, &actualW, &actualH);
    if (actualW > 0 && actualH > 0) {
      m_props.width = static_cast<uint32_t>(actualW);
      m_props.height = static_cast<uint32_t>(actualH);
    }

    s_currentWindow = this;
    m_isOpen = true;
    return true;
  }

  // Polls all pending SDL events, updates window, time, and input states, and dispatches callbacks.
  void pollEvents() {
    Time::update();
    Input::nextFrame();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      Input::onEvent(event);
      switch (event.type) {
      case SDL_EVENT_QUIT:
        m_isOpen = false;
        break;

      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        m_isOpen = false;
        break;

      case SDL_EVENT_WINDOW_RESIZED:
      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
        int w = 0, h = 0;
        if (m_window) {
          SDL_GetWindowSize(m_window, &w, &h);
          if (w > 0 && h > 0) {
            m_props.width = static_cast<uint32_t>(w);
            m_props.height = static_cast<uint32_t>(h);
          }
        }
        break;
      }

      default:
        break;
      }

      // Dispatch custom event callback if registered
      if (m_eventCallback) {
        m_eventCallback(event);
      }
    }
  }

  // Clears the window render target using the default configured clear color.
  void clear() {
    clear(m_props.clearColor);
  }

  // Clears the window render target with an MSL Color.
  void clear(const Color &color) {
    if (m_renderer) {
      uint8_t r = static_cast<uint8_t>(std::clamp(color.r * 255.0f + 0.5f, 0.0f, 255.0f));
      uint8_t g = static_cast<uint8_t>(std::clamp(color.g * 255.0f + 0.5f, 0.0f, 255.0f));
      uint8_t b = static_cast<uint8_t>(std::clamp(color.b * 255.0f + 0.5f, 0.0f, 255.0f));
      uint8_t a = static_cast<uint8_t>(std::clamp(color.a * 255.0f + 0.5f, 0.0f, 255.0f));
      SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
      SDL_RenderClear(m_renderer);
    }
  }

  // Clears the window render target with integer RGB(A) values.
  void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    if (m_renderer) {
      SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
      SDL_RenderClear(m_renderer);
    }
  }

  // Sets the active draw color on the SDL renderer using an MSL Color.
  void setDrawColor(const Color &color) {
    if (m_renderer) {
      uint8_t r = static_cast<uint8_t>(std::clamp(color.r * 255.0f + 0.5f, 0.0f, 255.0f));
      uint8_t g = static_cast<uint8_t>(std::clamp(color.g * 255.0f + 0.5f, 0.0f, 255.0f));
      uint8_t b = static_cast<uint8_t>(std::clamp(color.b * 255.0f + 0.5f, 0.0f, 255.0f));
      uint8_t a = static_cast<uint8_t>(std::clamp(color.a * 255.0f + 0.5f, 0.0f, 255.0f));
      SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    }
  }

  // Swaps/presents the rendered frame to the screen.
  void present() {
    if (m_renderer) {
      SDL_RenderPresent(m_renderer);
    }
  }

  // Closes the window and frees associated renderer/window handles.
  void close() {
    if (m_renderer) {
      SDL_DestroyRenderer(m_renderer);
      m_renderer = nullptr;
    }
    if (m_window) {
      SDL_DestroyWindow(m_window);
      m_window = nullptr;
    }
    m_isOpen = false;
  }

  // Maximizes the window.
  void maximize() {
    m_props.maximized = true;
    m_props.minimized = false;
    if (m_window) {
      SDL_MaximizeWindow(m_window);
    }
  }

  // Minimizes the window.
  void minimize() {
    m_props.minimized = true;
    m_props.maximized = false;
    if (m_window) {
      SDL_MinimizeWindow(m_window);
    }
  }

  // Restores the window from maximized or minimized state.
  void restore() {
    m_props.maximized = false;
    m_props.minimized = false;
    if (m_window) {
      SDL_RestoreWindow(m_window);
    }
  }

  // Returns true if the window is currently maximized.
  bool isMaximized() const {
    if (m_window) {
      return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MAXIMIZED) != 0;
    }
    return m_props.maximized;
  }

  // Returns true if the window is currently minimized.
  bool isMinimized() const {
    if (m_window) {
      return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) != 0;
    }
    return m_props.minimized;
  }

  // Sets the default clear color.
  void setClearColor(const Color &color) { m_props.clearColor = color; }

  // Returns the current clear color.
  const Color &getClearColor() const { return m_props.clearColor; }

  // Returns true if the window is currently open and running.
  bool isOpen() const { return m_isOpen; }

  // Returns true if the window should close (alias for !isOpen()).
  bool shouldClose() const { return !m_isOpen; }

  // Returns the width of the window in pixels.
  uint32_t getWidth() const { return m_props.width; }

  // Returns the height of the window in pixels.
  uint32_t getHeight() const { return m_props.height; }

  // Returns the aspect ratio of the window (width / height).
  float getAspectRatio() const {
    return m_props.height > 0
               ? static_cast<float>(m_props.width) / static_cast<float>(m_props.height)
               : 1.0f;
  }

  // Returns the current window title.
  const std::string &getTitle() const { return m_props.title; }

  // Updates the window title.
  void setTitle(const std::string &title) {
    m_props.title = title;
    if (m_window) {
      SDL_SetWindowTitle(m_window, title.c_str());
    }
  }

  // Sets window dimensions.
  void setSize(uint32_t width, uint32_t height) {
    m_props.width = width;
    m_props.height = height;
    if (m_window) {
      SDL_SetWindowSize(m_window, static_cast<int>(width),
                        static_cast<int>(height));
    }
  }

  // Enables or disables Vertical Sync (VSync).
  void setVSync(bool enabled) {
    m_props.vsync = enabled;
    if (m_renderer) {
      SDL_SetRenderVSync(m_renderer, enabled ? 1 : 0);
    }
  }

  // Returns true if VSync is currently enabled.
  bool isVSync() const { return m_props.vsync; }

  // Enables or disables fullscreen mode.
  void setFullscreen(bool fullscreen) {
    m_props.fullscreen = fullscreen;
    if (m_window) {
      SDL_SetWindowFullscreen(m_window, fullscreen);
    }
  }

  // Returns true if the window is currently in fullscreen mode.
  bool isFullscreen() const { return m_props.fullscreen; }

  // Enables or disables window resizing.
  void setResizable(bool resizable) {
    m_props.resizable = resizable;
    if (m_window) {
      SDL_SetWindowResizable(m_window, resizable);
    }
  }

  // Returns true if the window is resizable.
  bool isResizable() const { return m_props.resizable; }

  // Registers a callback to receive all raw polled SDL events.
  void setEventCallback(const EventCallbackFn &callback) {
    m_eventCallback = callback;
  }

  // Static Window / Viewport queries
  static Window *getCurrent() { return s_currentWindow; }
  static Vector2 getViewportSize() {
    return s_currentWindow ? Vector2(static_cast<float>(s_currentWindow->getWidth()),
                                     static_cast<float>(s_currentWindow->getHeight()))
                           : Vector2(1280.0f, 720.0f);
  }
  static Vector2 getViewportCenter() { return getViewportSize() * 0.5f; }

  // Returns the underlying native SDL_Window pointer.
  SDL_Window *getNativeWindow() const { return m_window; }

  // Returns the underlying native SDL_Renderer pointer.
  SDL_Renderer *getRenderer() const { return m_renderer; }

private:
  inline static Window *s_currentWindow = nullptr;
  SDL_Window *m_window = nullptr;
  SDL_Renderer *m_renderer = nullptr;
  WindowProps m_props;
  bool m_isOpen = false;
  EventCallbackFn m_eventCallback;
};

// Inline implementations of Node viewport queries
inline Vector2 Node::getViewportSize() const { return Window::getViewportSize(); }
inline Vector2 Node::getViewportCenter() const { return Window::getViewportCenter(); }