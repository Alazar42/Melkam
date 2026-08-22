#pragma once

#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// 2D Hardware GPU Texture representation.
class Texture2D {
public:
  Texture2D() = default;

  // Constructs and loads a texture from an image file (e.g. BMP).
  Texture2D(const std::string &filePath, SDL_Renderer *renderer = nullptr) {
    loadFromFile(filePath, renderer);
  }

  // Destructor frees native SDL_Texture handle.
  ~Texture2D() {
    destroy();
  }

  // Non-copyable
  Texture2D(const Texture2D &) = delete;
  Texture2D &operator=(const Texture2D &) = delete;

  // Move-constructible
  Texture2D(Texture2D &&other) noexcept
      : m_texture(other.m_texture), m_width(other.m_width),
        m_height(other.m_height), m_path(std::move(other.m_path)) {
    other.m_texture = nullptr;
    other.m_width = 0;
    other.m_height = 0;
  }

  // Move-assignable
  Texture2D &operator=(Texture2D &&other) noexcept {
    if (this != &other) {
      destroy();
      m_texture = other.m_texture;
      m_width = other.m_width;
      m_height = other.m_height;
      m_path = std::move(other.m_path);

      other.m_texture = nullptr;
      other.m_width = 0;
      other.m_height = 0;
    }
    return *this;
  }

  // Loads a texture from an image file path.
  bool loadFromFile(const std::string &filePath, SDL_Renderer *renderer = nullptr) {
    destroy();
    m_path = filePath;

    if (!renderer) {
      renderer = s_defaultRenderer;
    }

    if (!renderer) {
      std::cerr << "[Texture2D Error] No active SDL_Renderer provided to load: "
                << filePath << std::endl;
      return false;
    }

    // Load surface using SDL3
    SDL_Surface *surface = SDL_LoadBMP(filePath.c_str());
    if (!surface) {
      std::cerr << "[Texture2D Error] Failed to load image '" << filePath
                << "': " << SDL_GetError() << std::endl;
      return false;
    }

    m_width = surface->w;
    m_height = surface->h;

    // Create hardware texture from surface
    m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (!m_texture) {
      std::cerr << "[Texture2D Error] Failed to create texture from surface: "
                << SDL_GetError() << std::endl;
      return false;
    }

    return true;
  }

  // Creates a solid single-color texture.
  static std::shared_ptr<Texture2D> createSolid(int width, int height,
                                                const Color &color,
                                                SDL_Renderer *renderer = nullptr) {
    auto tex = std::make_shared<Texture2D>();
    if (!renderer) {
      renderer = s_defaultRenderer;
    }

    if (!renderer || width <= 0 || height <= 0) return tex;

    SDL_Surface *surface =
        SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return tex;

    uint8_t r = static_cast<uint8_t>(std::clamp(color.r * 255.0f + 0.5f, 0.0f, 255.0f));
    uint8_t g = static_cast<uint8_t>(std::clamp(color.g * 255.0f + 0.5f, 0.0f, 255.0f));
    uint8_t b = static_cast<uint8_t>(std::clamp(color.b * 255.0f + 0.5f, 0.0f, 255.0f));
    uint8_t a = static_cast<uint8_t>(std::clamp(color.a * 255.0f + 0.5f, 0.0f, 255.0f));

    SDL_FillSurfaceRect(surface, nullptr,
                        SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format),
                                    nullptr, r, g, b, a));

    tex->m_width = width;
    tex->m_height = height;
    tex->m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return tex;
  }

  // Sets default fallback renderer for texture generation.
  static void setDefaultRenderer(SDL_Renderer *renderer) {
    s_defaultRenderer = renderer;
  }

  // Frees the GPU texture resource.
  void destroy() {
    if (m_texture) {
      SDL_DestroyTexture(m_texture);
      m_texture = nullptr;
    }
    m_width = 0;
    m_height = 0;
  }

  // Returns true if the texture is valid and loaded on the GPU.
  bool isValid() const { return m_texture != nullptr; }

  // Returns the width of the texture in pixels.
  int getWidth() const { return m_width; }

  // Returns the height of the texture in pixels.
  int getHeight() const { return m_height; }

  // Returns dimensions as a Vector2.
  Vector2 getSize() const {
    return Vector2(static_cast<float>(m_width), static_cast<float>(m_height));
  }

  // Returns the underlying native SDL_Texture pointer.
  SDL_Texture *getNativeTexture() const { return m_texture; }

  // Returns the file path used to load this texture.
  const std::string &getPath() const { return m_path; }

private:
  SDL_Texture *m_texture = nullptr;
  int m_width = 0;
  int m_height = 0;
  std::string m_path;

  inline static SDL_Renderer *s_defaultRenderer = nullptr;
};
