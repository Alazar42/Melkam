#pragma once

#include "helper/color/Color.hpp"
#include "helper/IconsFontAwesome6.hpp"
#include "helper/stb_rect_pack.h"
#include "helper/stb_truetype.h"
#include "helper/vectors/Vector2.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Helper to decode UTF-8 byte stream into a single Unicode Codepoint
inline uint32_t decodeUtf8Codepoint(const char *&ptr, const char *end) {
  if (ptr >= end) return 0;
  uint8_t c = static_cast<uint8_t>(*ptr++);
  if (c < 0x80) return c;
  if ((c & 0xE0) == 0xC0 && ptr < end) {
    uint8_t c2 = static_cast<uint8_t>(*ptr++);
    return ((c & 0x1F) << 6) | (c2 & 0x3F);
  }
  if ((c & 0xF0) == 0xE0 && ptr + 1 < end) {
    uint8_t c2 = static_cast<uint8_t>(*ptr++);
    uint8_t c3 = static_cast<uint8_t>(*ptr++);
    return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
  }
  if ((c & 0xF8) == 0xF0 && ptr + 2 < end) {
    uint8_t c2 = static_cast<uint8_t>(*ptr++);
    uint8_t c3 = static_cast<uint8_t>(*ptr++);
    uint8_t c4 = static_cast<uint8_t>(*ptr++);
    return ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
  }
  return c;
}

// High-Fidelity Subpixel Antialiased Font Resource (powered by stb_truetype & FontAwesome 6 icon fallback).
class Font {
public:
  Font() {
    createDefaultBitmapFont();
  }

  explicit Font(const std::string &filePath, float fontSize = 18.0f,
                SDL_Renderer *renderer = nullptr, bool isIconFont = false) {
    if (!loadFromFile(filePath, fontSize, renderer, isIconFont)) {
      createDefaultBitmapFont();
    }
  }

  void destroy() {
    if (m_texture) {
      SDL_DestroyTexture(m_texture);
      m_texture = nullptr;
    }
    m_ranges.clear();
    m_atlasWidth = 0;
    m_atlasHeight = 0;
    m_isTTF = false;
  }

  ~Font() {
    destroy();
  }

  // Non-copyable
  Font(const Font &) = delete;
  Font &operator=(const Font &) = delete;

  // Move-constructible
  Font(Font &&other) noexcept
      : m_texture(other.m_texture), m_atlasWidth(other.m_atlasWidth),
        m_atlasHeight(other.m_atlasHeight), m_baseFontSize(other.m_baseFontSize),
        m_isTTF(other.m_isTTF), m_isIconFont(other.m_isIconFont),
        m_path(std::move(other.m_path)), m_ranges(std::move(other.m_ranges)) {
    other.m_texture = nullptr;
    other.m_isTTF = false;
  }

  // Move-assignable
  Font &operator=(Font &&other) noexcept {
    if (this != &other) {
      destroy();
      m_texture = other.m_texture;
      m_atlasWidth = other.m_atlasWidth;
      m_atlasHeight = other.m_atlasHeight;
      m_baseFontSize = other.m_baseFontSize;
      m_isTTF = other.m_isTTF;
      m_isIconFont = other.m_isIconFont;
      m_path = std::move(other.m_path);
      m_ranges = std::move(other.m_ranges);

      other.m_texture = nullptr;
      other.m_isTTF = false;
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

  static std::vector<std::string> getSystemFontCandidates() {
    return {
      "assets/fonts/Roboto/static/Roboto-Medium.ttf",
      "assets/fonts/Roboto/static/Roboto-Regular.ttf",
      "assets/fonts/Roboto/static/Roboto-SemiBold.ttf",
      "assets/fonts/Inter/static/Inter_18pt-Medium.ttf",
      "assets/fonts/Inter/static/Inter_18pt-Regular.ttf",
      "assets/fonts/MedievalSharp/MedievalSharp-Regular.ttf",
      "assets/fonts/Basic/Basic-Regular.ttf",
      "C:/Windows/Fonts/segoeui.ttf",
      "C:/Windows/Fonts/arial.ttf",
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    };
  }

  // Loads and packs a TrueType font with multi-size 2x2 subpixel oversampling
  bool loadFromFile(const std::string &filePath, float defaultSize = 18.0f,
                    SDL_Renderer *renderer = nullptr, bool isIconFont = false) {
    destroy();
    m_path = filePath;
    m_baseFontSize = defaultSize;
    m_isIconFont = isIconFont;

    if (!renderer) renderer = s_defaultRenderer;
    if (!renderer) return false;

    std::string actualPath = resolvePath(filePath);
    std::ifstream file(actualPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> ttfBuffer(size);
    if (!file.read(reinterpret_cast<char *>(ttfBuffer.data()), size)) {
      return false;
    }

    m_atlasWidth = 1024;
    m_atlasHeight = 1024;
    std::vector<unsigned char> tempBitmap(m_atlasWidth * m_atlasHeight, 0);

    std::vector<float> targetSizes = {14.0f, 16.0f, 18.0f, 20.0f, 24.0f, 32.0f};
    m_ranges.resize(targetSizes.size());

    stbtt_pack_context spc;
    if (!stbtt_PackBegin(&spc, tempBitmap.data(), m_atlasWidth, m_atlasHeight, 0, 1, nullptr)) {
      return false;
    }

    stbtt_PackSetOversampling(&spc, 2, 2);

    for (size_t i = 0; i < targetSizes.size(); ++i) {
      m_ranges[i].fontSize = targetSizes[i];

      if (isIconFont) {
        stbtt_PackFontRange(&spc, ttfBuffer.data(), 0, targetSizes[i], 0xF000, 256, m_ranges[i].faCharData);
        m_ranges[i].hasFA = true;
      } else {
        stbtt_PackFontRange(&spc, ttfBuffer.data(), 0, targetSizes[i], 32, 96, m_ranges[i].charData);
        m_ranges[i].hasFA = false;
      }
    }
    stbtt_PackEnd(&spc);

    // Convert 8-bit alpha mask to 32-bit RGBA texture
    std::vector<uint32_t> rgbaPixels(m_atlasWidth * m_atlasHeight);
    for (size_t i = 0; i < tempBitmap.size(); ++i) {
      uint8_t rawAlpha = tempBitmap[i];
      if (rawAlpha > 0) {
        float a = rawAlpha / 255.0f;
        a = std::pow(a, 0.85f);
        uint8_t alpha = static_cast<uint8_t>(std::clamp(a * 255.0f + 6.0f, 0.0f, 255.0f));
        rgbaPixels[i] = (static_cast<uint32_t>(alpha) << 24) | 0x00FFFFFF;
      } else {
        rgbaPixels[i] = 0x00000000;
      }
    }

    SDL_Surface *surface = SDL_CreateSurfaceFrom(
        m_atlasWidth, m_atlasHeight, SDL_PIXELFORMAT_RGBA32,
        rgbaPixels.data(), m_atlasWidth * 4);
    if (!surface) return false;

    m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (m_texture) {
      SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
      SDL_SetTextureScaleMode(m_texture, SDL_SCALEMODE_LINEAR);
      m_isTTF = true;
      return true;
    }
    return false;
  }

  // Returns single/multi-line text dimensions in screen pixels.
  Vector2 getStringSize(const std::string &text, float customFontSize = 0.0f) const {
    if (text.empty()) return {0.0f, 0.0f};

    float reqSize = (customFontSize > 0.0f) ? customFontSize : m_baseFontSize;

    if (!m_isTTF || m_ranges.empty()) {
      float curX = 0.0f, maxX = 0.0f, totalH = reqSize;
      for (char c : text) {
        if (c == '\n') {
          if (curX > maxX) maxX = curX;
          curX = 0.0f;
          totalH += reqSize * 1.35f;
        } else {
          curX += 8.0f * (reqSize / 16.0f);
        }
      }
      if (curX > maxX) maxX = curX;
      return {maxX, totalH};
    }

    const SizeRange &range = getClosestRange(reqSize);
    float curX = 0.0f;
    float maxX = 0.0f;
    float totalH = reqSize;

    const char *ptr = text.c_str();
    const char *end = ptr + text.size();

    while (ptr < end) {
      uint32_t codepoint = decodeUtf8Codepoint(ptr, end);

      if (codepoint == '\n') {
        if (curX > maxX) maxX = curX;
        curX = 0.0f;
        totalH += (reqSize * 1.35f);
        continue;
      }

      if (codepoint >= 32 && codepoint <= 126 && !m_isIconFont) {
        int idx = static_cast<int>(codepoint - 32);
        stbtt_aligned_quad q;
        float dummyY = 0.0f;
        stbtt_GetPackedQuad(range.charData, m_atlasWidth, m_atlasHeight, idx, &curX, &dummyY, &q, 0);
      } else if (codepoint >= 0xF000 && codepoint < 0xF000 + 256) {
        int idx = static_cast<int>(codepoint - 0xF000);
        if (m_isIconFont) {
          stbtt_aligned_quad q;
          float dummyY = 0.0f;
          stbtt_GetPackedQuad(range.faCharData, m_atlasWidth, m_atlasHeight, idx, &curX, &dummyY, &q, 0);
        } else {
          auto iconFont = getIconFont();
          if (iconFont && iconFont.get() != this && iconFont->m_isTTF && !iconFont->m_ranges.empty()) {
            const auto &iconRange = iconFont->getClosestRange(reqSize);
            stbtt_aligned_quad q;
            float dummyY = 0.0f;
            stbtt_GetPackedQuad(iconRange.faCharData, iconFont->m_atlasWidth, iconFont->m_atlasHeight, idx, &curX, &dummyY, &q, 0);
          } else {
            curX += reqSize;
          }
        }
      } else if (codepoint == ' ') {
        curX += (reqSize * 0.28f);
      } else {
        curX += (reqSize * 0.5f);
      }
    }

    if (curX > maxX) maxX = curX;
    return {maxX, totalH};
  }

  // Renders text with automatic FontAwesome icon font fallback
  void drawText(SDL_Renderer *renderer, const std::string &text,
                const Vector2 &position, const Color &color = Color::WHITE,
                float customFontSize = 0.0f) const {
    if (!renderer || text.empty()) return;

    float reqSize = (customFontSize > 0.0f) ? customFontSize : m_baseFontSize;

    if (m_isTTF && m_texture && !m_ranges.empty()) {
      const SizeRange &range = getClosestRange(reqSize);
      float scale = reqSize / range.fontSize;

      SDL_SetTextureColorModFloat(m_texture, color.r, color.g, color.b);
      SDL_SetTextureAlphaModFloat(m_texture, color.a);

      float curX = position.x;
      float curY = position.y + (range.fontSize * 0.85f * scale);

      const char *ptr = text.c_str();
      const char *end = ptr + text.size();

      while (ptr < end) {
        uint32_t codepoint = decodeUtf8Codepoint(ptr, end);

        if (codepoint == '\n') {
          curX = position.x;
          curY += (reqSize * 1.35f);
          continue;
        }

        if (codepoint >= 32 && codepoint <= 126 && !m_isIconFont) {
          int idx = static_cast<int>(codepoint - 32);
          stbtt_aligned_quad q;
          float prevX = curX;
          float prevY = curY;

          stbtt_GetPackedQuad(range.charData, m_atlasWidth, m_atlasHeight, idx, &curX, &curY, &q, 0);

          float quadW = (q.x1 - q.x0) * scale;
          float quadH = (q.y1 - q.y0) * scale;
          float drawX = prevX + (q.x0 - prevX) * scale;
          float drawY = prevY + (q.y0 - prevY) * scale;

          SDL_FRect srcRect{q.s0 * m_atlasWidth, q.t0 * m_atlasHeight,
                            (q.s1 - q.s0) * m_atlasWidth, (q.t1 - q.t0) * m_atlasHeight};
          SDL_FRect dstRect{drawX, drawY, quadW, quadH};

          SDL_RenderTexture(renderer, m_texture, &srcRect, &dstRect);
          curX = prevX + (curX - prevX) * scale;
          curY = prevY;
        } else if (codepoint >= 0xF000 && codepoint < 0xF000 + 256) {
          int idx = static_cast<int>(codepoint - 0xF000);

          if (m_isIconFont) {
            stbtt_aligned_quad q;
            float prevX = curX;
            float prevY = curY;

            stbtt_GetPackedQuad(range.faCharData, m_atlasWidth, m_atlasHeight, idx, &curX, &curY, &q, 0);

            float quadW = (q.x1 - q.x0) * scale;
            float quadH = (q.y1 - q.y0) * scale;
            float drawX = prevX + (q.x0 - prevX) * scale;
            float drawY = prevY + (q.y0 - prevY) * scale;

            SDL_FRect srcRect{q.s0 * m_atlasWidth, q.t0 * m_atlasHeight,
                              (q.s1 - q.s0) * m_atlasWidth, (q.t1 - q.t0) * m_atlasHeight};
            SDL_FRect dstRect{drawX, drawY, quadW, quadH};

            SDL_RenderTexture(renderer, m_texture, &srcRect, &dstRect);
            curX = prevX + (curX - prevX) * scale;
            curY = prevY;
          } else {
            auto iconFont = getIconFont();
            if (iconFont && iconFont.get() != this && iconFont->m_isTTF && !iconFont->m_ranges.empty()) {
              const auto &iconRange = iconFont->getClosestRange(reqSize);
              float iconScale = reqSize / iconRange.fontSize;

              SDL_SetTextureColorModFloat(iconFont->m_texture, color.r, color.g, color.b);
              SDL_SetTextureAlphaModFloat(iconFont->m_texture, color.a);

              stbtt_aligned_quad q;
              float prevX = curX;
              float prevY = curY;

              stbtt_GetPackedQuad(iconRange.faCharData, iconFont->m_atlasWidth, iconFont->m_atlasHeight, idx, &curX, &curY, &q, 0);

              float quadW = (q.x1 - q.x0) * iconScale;
              float quadH = (q.y1 - q.y0) * iconScale;
              float drawX = prevX + (q.x0 - prevX) * iconScale;
              float drawY = prevY + (q.y0 - prevY) * iconScale;

              SDL_FRect srcRect{q.s0 * iconFont->m_atlasWidth, q.t0 * iconFont->m_atlasHeight,
                                (q.s1 - q.s0) * iconFont->m_atlasWidth, (q.t1 - q.t0) * iconFont->m_atlasHeight};
              SDL_FRect dstRect{drawX, drawY, quadW, quadH};

              SDL_RenderTexture(renderer, iconFont->m_texture, &srcRect, &dstRect);
              curX = prevX + (curX - prevX) * iconScale;
              curY = prevY;

              // Restore texture color mod on this font
              SDL_SetTextureColorModFloat(m_texture, color.r, color.g, color.b);
              SDL_SetTextureAlphaModFloat(m_texture, color.a);
            }
          }
        } else if (codepoint == ' ') {
          curX += (reqSize * 0.28f);
        }
      }
    } else {
      drawFallbackBitmapText(renderer, text, position, color, reqSize / 16.0f);
    }
  }

  // Global default font
  static std::shared_ptr<Font> getDefaultFont() {
    if (!s_defaultFont) {
      s_defaultFont = std::make_shared<Font>();
      loadPlatformSystemFont(s_defaultFont.get());
    } else if (!s_defaultFont->isTTF() && s_defaultRenderer) {
      loadPlatformSystemFont(s_defaultFont.get());
    }
    return s_defaultFont;
  }

  // Dedicated FontAwesome Icon Font
  static std::shared_ptr<Font> getIconFont() {
    if (!s_iconFont && s_defaultRenderer) {
      std::string faPath = resolvePath("assets/fonts/fontawesome/fa-solid-900.ttf");
      if (std::filesystem::exists(faPath)) {
        auto faFont = std::make_shared<Font>();
        if (faFont->loadFromFile(faPath, 18.0f, s_defaultRenderer, true)) {
          s_iconFont = faFont;
          std::cout << "[MelkamEngine] FontAwesome 6 Icon Font Engine Loaded: " << faPath << std::endl;
        }
      }
    }
    return s_iconFont;
  }

  static bool loadPlatformSystemFont(Font *font) {
    if (!font || !s_defaultRenderer) return false;

    for (const auto &cand : getSystemFontCandidates()) {
      std::string actualPath = resolvePath(cand);
      if (std::filesystem::exists(actualPath)) {
        if (font->loadFromFile(actualPath, 18.0f, s_defaultRenderer, false)) {
          std::cout << "[MelkamEngine] Subpixel Font Engine Loaded: " << actualPath << std::endl;
          return true;
        }
      }
    }
    return false;
  }

  static void setDefaultRenderer(SDL_Renderer *renderer) {
    s_defaultRenderer = renderer;
    if (s_defaultFont) {
      loadPlatformSystemFont(s_defaultFont.get());
    }
    getIconFont(); // Preload FontAwesome icon font
  }

  bool isTTF() const { return m_isTTF; }
  float getBaseFontSize() const { return m_baseFontSize; }

private:
  struct SizeRange {
    float fontSize = 18.0f;
    stbtt_packedchar charData[96]{};
    stbtt_packedchar faCharData[256]{};
    bool hasFA = false;
  };

  const SizeRange &getClosestRange(float targetSize) const {
    if (m_ranges.empty()) {
      static SizeRange dummy;
      return dummy;
    }
    size_t bestIdx = 0;
    float bestDiff = std::abs(m_ranges[0].fontSize - targetSize);
    for (size_t i = 1; i < m_ranges.size(); ++i) {
      float diff = std::abs(m_ranges[i].fontSize - targetSize);
      if (diff < bestDiff) {
        bestDiff = diff;
        bestIdx = i;
      }
    }
    return m_ranges[bestIdx];
  }

  void createDefaultBitmapFont() {
    m_baseFontSize = 16.0f;
    m_isTTF = false;
  }

  void drawFallbackBitmapText(SDL_Renderer *renderer, const std::string &text,
                              const Vector2 &pos, const Color &color, float scale) const {
    SDL_SetRenderDrawColor(renderer,
                           static_cast<uint8_t>(color.r * 255.0f),
                           static_cast<uint8_t>(color.g * 255.0f),
                           static_cast<uint8_t>(color.b * 255.0f),
                           static_cast<uint8_t>(color.a * 255.0f));

    float pixelW = std::max(1.0f, scale);
    float pixelH = std::max(1.0f, scale);
    float curX = pos.x;
    float curY = pos.y;

    for (char c : text) {
      if (c == '\n') {
        curX = pos.x;
        curY += (10.0f * pixelH);
      } else if (c == ' ') {
        curX += (4 * pixelW);
      } else {
        SDL_FRect pixelRect{curX, curY, 6.0f * pixelW, 8.0f * pixelH};
        SDL_RenderFillRect(renderer, &pixelRect);
        curX += (7 * pixelW);
      }
    }
  }

  SDL_Texture *m_texture = nullptr;
  int m_atlasWidth = 0;
  int m_atlasHeight = 0;
  float m_baseFontSize = 18.0f;
  bool m_isTTF = false;
  bool m_isIconFont = false;
  std::string m_path;
  std::vector<SizeRange> m_ranges;

  inline static std::shared_ptr<Font> s_defaultFont = nullptr;
  inline static std::shared_ptr<Font> s_iconFont = nullptr;
  inline static SDL_Renderer *s_defaultRenderer = nullptr;
};
