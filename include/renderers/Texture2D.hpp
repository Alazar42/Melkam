#pragma once
#include "core/Resource.hpp"
#include "helper/Rect2.hpp"
#include "helper/color/Color.hpp"
#include "helper/nanosvg.h"
#include "helper/nanosvgrast.h"
#include "helper/stb_image.h"
#include "helper/vectors/Vector2.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// 2D Hardware GPU Texture representation with SVG/PNG/JPG/BMP/TGA decoding and deferred GPU loading.
class Texture2D : public Resource {
public:
  Texture2D() : Resource() {}

  // Constructs and loads a texture from an image file (SVG, PNG, JPG, BMP, TGA).
  explicit Texture2D(const std::string &filePath, SDL_Renderer *renderer = nullptr)
      : Resource(filePath) {
    loadFromFile(filePath, renderer);
  }

  bool load(const std::string &path) override {
    return loadFromFile(path, nullptr);
  }

  // Destructor frees native SDL_Texture handle.
  ~Texture2D() override {
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

  // Resolves file path across common directory layouts
  static std::string resolvePath(const std::string &path) {
    std::string cleanPath = path;
    if (cleanPath.rfind("res://", 0) == 0) {
      cleanPath = cleanPath.substr(6);
    }
    if (std::filesystem::exists(cleanPath)) return cleanPath;
    if (std::filesystem::exists("../" + cleanPath)) return "../" + cleanPath;
    if (std::filesystem::exists("../../" + cleanPath)) return "../../" + cleanPath;
    if (std::filesystem::exists("../../../" + cleanPath)) return "../../../" + cleanPath;
    return cleanPath;
  }

  // Loads a texture from an image or vector file path (SVG, PNG, JPG, BMP, TGA).
  bool loadFromFile(const std::string &filePath, SDL_Renderer *renderer = nullptr) {
    destroy();
    m_path = filePath;

    if (!renderer) {
      renderer = s_defaultRenderer;
    }

    if (!renderer) {
      // Defer GPU upload until active renderer is available
      return true;
    }

    std::string actualPath = resolvePath(filePath);
    if (!std::filesystem::exists(actualPath)) {
      std::cerr << "[Texture2D Error] File not found: " << filePath << std::endl;
      return false;
    }

    // 1. Check for SVG Vector Format
    if (actualPath.size() >= 4 &&
        (actualPath.compare(actualPath.size() - 4, 4, ".svg") == 0 ||
         actualPath.compare(actualPath.size() - 4, 4, ".SVG") == 0)) {
      NSVGimage *svgImage = nsvgParseFromFile(actualPath.c_str(), "px", 96.0f);
      if (svgImage) {
        int w = static_cast<int>(svgImage->width);
        int h = static_cast<int>(svgImage->height);
        if (w <= 0) w = 1000;
        if (h <= 0) h = 1000;

        NSVGrasterizer *rast = nsvgCreateRasterizer();
        if (rast) {
          std::vector<unsigned char> imgData(w * h * 4, 0);
          nsvgRasterize(rast, svgImage, 0.0f, 0.0f, 1.0f, imgData.data(), w, h, w * 4);
          nsvgDeleteRasterizer(rast);

          SDL_Surface *surface = SDL_CreateSurfaceFrom(
              w, h, SDL_PIXELFORMAT_RGBA32, imgData.data(), w * 4);
          if (surface) {
            m_width = w;
            m_height = h;
            m_texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_DestroySurface(surface);
          }
        }
        nsvgDelete(svgImage);

        if (m_texture) {
          SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
          return true;
        }
      }
      return false;
    }

    // 2. Load Raster Bitmap (PNG, JPG, BMP, TGA) via STB Image
    int w = 0, h = 0, channels = 0;
    unsigned char *data = stbi_load(actualPath.c_str(), &w, &h, &channels, 4);
    if (!data) {
      std::cerr << "[Texture2D Error] Failed to decode image: " << actualPath
                << " (stb_image error: " << stbi_failure_reason() << ")" << std::endl;
      return false;
    }

    m_pixels.resize(w * h);
    for (int i = 0; i < w * h; ++i) {
      uint8_t r = data[i * 4 + 0];
      uint8_t g = data[i * 4 + 1];
      uint8_t b = data[i * 4 + 2];
      uint8_t a = data[i * 4 + 3];
      m_pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    SDL_Surface *surface = SDL_CreateSurfaceFrom(
        w, h, SDL_PIXELFORMAT_RGBA32, data, w * 4);
    if (!surface) {
      stbi_image_free(data);
      return false;
    }

    m_width = w;
    m_height = h;
    m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    stbi_image_free(data);

    if (m_texture) {
      SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
      return true;
    }

    return false;
  }

  // Samples texture color at normalized UV coordinates with wrapping (0.0 to 1.0)
  uint32_t sampleUV(float u, float v) const {
    if (m_pixels.empty() || m_width <= 0 || m_height <= 0) return 0xFFFFFFFF;
    u = u - std::floor(u);
    v = v - std::floor(v);
    int x = std::clamp(static_cast<int>(u * static_cast<float>(m_width)), 0, m_width - 1);
    int y = std::clamp(static_cast<int>(v * static_cast<float>(m_height)), 0, m_height - 1);
    return m_pixels[y * m_width + x];
  }

  // Ultra-fast integer fixed-point (16.16) texture sampling with zero float overhead
  inline uint32_t sampleFast(int u_fixed, int v_fixed) const {
    if (m_pixels.empty() || m_width <= 0 || m_height <= 0) return 0xFFFFFFFF;
    int u = (u_fixed >> 16) % m_width;
    if (u < 0) u += m_width;
    int v = (v_fixed >> 16) % m_height;
    if (v < 0) v += m_height;
    return m_pixels[v * m_width + u];
  }

  Color sampleColor(float u, float v) const {
    uint32_t p = sampleUV(u, v);
    float a = static_cast<float>((p >> 24) & 0xFF) / 255.0f;
    float r = static_cast<float>((p >> 16) & 0xFF) / 255.0f;
    float g = static_cast<float>((p >> 8) & 0xFF) / 255.0f;
    float b = static_cast<float>(p & 0xFF) / 255.0f;
    return Color(r, g, b, a);
  }

  const std::vector<uint32_t> &getPixels() const { return m_pixels; }
  const uint32_t *getRawPixelData() const { return m_pixels.data(); }
  bool hasPixels() const { return !m_pixels.empty(); }

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
    m_pixels.clear();
    m_width = 0;
    m_height = 0;
  }

  // Returns true if the texture is valid and loaded on the GPU.
  bool isValid() const {
    ensureLoaded();
    return m_texture != nullptr;
  }

  // Returns the width of the texture in pixels.
  int getWidth() const {
    ensureLoaded();
    return m_width;
  }

  // Returns the height of the texture in pixels.
  int getHeight() const {
    ensureLoaded();
    return m_height;
  }

  // Returns dimensions as a Vector2.
  Vector2 getSize() const {
    ensureLoaded();
    return Vector2(static_cast<float>(m_width), static_cast<float>(m_height));
  }

  // Returns full bounding Rect2 of the texture.
  Rect2 getRect() const {
    ensureLoaded();
    return Rect2(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
  }

  // Returns the underlying native SDL_Texture pointer.
  SDL_Texture *getNativeTexture() const {
    ensureLoaded();
    return m_texture;
  }

  // Returns the file path used to load this texture.
  const std::string &getPath() const { return m_path; }

private:
  void ensureLoaded() const {
    if (!m_texture && !m_path.empty() && s_defaultRenderer) {
      const_cast<Texture2D *>(this)->loadFromFile(m_path, s_defaultRenderer);
    }
  }

  SDL_Texture *m_texture = nullptr;
  std::vector<uint32_t> m_pixels;
  int m_width = 0;
  int m_height = 0;
  std::string m_path;

  inline static SDL_Renderer *s_defaultRenderer = nullptr;
};
