#pragma once

#include "helper/color/Color.hpp"
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

// High-Fidelity Subpixel Antialiased Font Resource (powered by stb_truetype & oversampled packing).
class Font {
public:
  Font() {
    createDefaultBitmapFont();
  }

  explicit Font(const std::string &filePath, float fontSize = 18.0f,
                SDL_Renderer *renderer = nullptr) {
    if (!loadFromFile(filePath, fontSize, renderer)) {
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
        m_isTTF(other.m_isTTF), m_path(std::move(other.m_path)),
        m_ranges(std::move(other.m_ranges)) {
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

  // Returns list of standard system and project font candidates
  static std::vector<std::string> getSystemFontCandidates() {
    return {
      // 1. Project Asset Fonts (User Preferred Default)
      "assets/fonts/Roboto/static/Roboto-Medium.ttf",
      "assets/fonts/Roboto/static/Roboto-Regular.ttf",
      "assets/fonts/Roboto/static/Roboto-SemiBold.ttf",
      "assets/fonts/Inter/static/Inter_18pt-Medium.ttf",
      "assets/fonts/Inter/static/Inter_18pt-SemiBold.ttf",
      "assets/fonts/Inter/static/Inter_18pt-Regular.ttf",
      "assets/fonts/MedievalSharp/MedievalSharp-Regular.ttf",
      "assets/fonts/Basic/Basic-Regular.ttf",
      "assets/fonts/basics/Basic-Regular.ttf",
      "assets/fonts/basics/basic-regular.ttf",
      // 2. Windows standard fonts
      "C:/Windows/Fonts/segoeui.ttf",
      "C:/Windows/Fonts/arial.ttf",
      "C:/Windows/Fonts/calibri.ttf",
      "C:/Windows/Fonts/tahoma.ttf",
      // 3. Linux standard fonts
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
      "/usr/share/fonts/TTF/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
      // 4. macOS standard fonts
      "/System/Library/Fonts/SFNS.ttf",
      "/Library/Fonts/Arial.ttf",
      "/System/Library/Fonts/Helvetica.ttc"
    };
  }

  // Loads and packs a TrueType font with multi-size 2x2 subpixel oversampling
  bool loadFromFile(const std::string &filePath, float defaultSize = 18.0f,
                    SDL_Renderer *renderer = nullptr) {
    destroy();
    m_path = filePath;
    m_baseFontSize = defaultSize;

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

    // Multi-size resolution packing: 14, 16, 18, 20, 24, 32, 48
    std::vector<float> targetSizes = {14.0f, 16.0f, 18.0f, 20.0f, 24.0f, 32.0f, 48.0f};
    m_ranges.resize(targetSizes.size());

    stbtt_pack_context spc;
    if (!stbtt_PackBegin(&spc, tempBitmap.data(), m_atlasWidth, m_atlasHeight, 0, 1, nullptr)) {
      return false;
    }

    // 2x2 Subpixel Oversampling for razor-sharp antialiasing
    stbtt_PackSetOversampling(&spc, 2, 2);

    for (size_t i = 0; i < targetSizes.size(); ++i) {
      m_ranges[i].fontSize = targetSizes[i];
      if (!stbtt_PackFontRange(&spc, ttfBuffer.data(), 0, targetSizes[i], 32, 96,
                               m_ranges[i].charData)) {
        stbtt_PackEnd(&spc);
        return false;
      }
    }
    stbtt_PackEnd(&spc);

    // Convert 8-bit alpha mask to 32-bit RGBA texture with perceptual gamma contrast
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

  // Returns single-line text dimensions in screen pixels.
  Vector2 getStringSize(const std::string &text, float customFontSize = 0.0f) const {
    if (text.empty()) return {0.0f, 0.0f};

    float reqSize = (customFontSize > 0.0f) ? customFontSize : m_baseFontSize;

    if (!m_isTTF || m_ranges.empty()) {
      return {static_cast<float>(text.length()) * 8.0f * (reqSize / 16.0f), reqSize};
    }

    const SizeRange &range = getClosestRange(reqSize);
    float scale = reqSize / range.fontSize;

    float curX = 0.0f;
    float curY = 0.0f;

    for (char c : text) {
      if (c >= 32 && c < 128) {
        stbtt_aligned_quad q;
        stbtt_GetPackedQuad(range.charData, m_atlasWidth, m_atlasHeight, c - 32,
                            &curX, &curY, &q, 0);
      } else if (c == ' ') {
        curX += (range.fontSize * 0.28f);
      }
    }

    return {curX * scale, reqSize};
  }

  // Renders text string to screen with subpixel oversampled precision.
  void drawText(SDL_Renderer *renderer, const std::string &text,
                const Vector2 &position, const Color &color,
                float customFontSize = 0.0f) const {
    if (!renderer || text.empty()) return;

    float reqSize = (customFontSize > 0.0f) ? customFontSize : m_baseFontSize;

    if (m_isTTF && m_texture && !m_ranges.empty()) {
      const SizeRange &range = getClosestRange(reqSize);
      float scale = reqSize / range.fontSize;

      SDL_SetTextureColorModFloat(m_texture, color.r, color.g, color.b);
      SDL_SetTextureAlphaModFloat(m_texture, color.a);

      float curX = position.x;
      float curY = position.y + (reqSize * 0.82f); // baseline

      for (char c : text) {
        if (c >= 32 && c < 128) {
          stbtt_aligned_quad q;
          float prevX = curX, prevY = curY;
          stbtt_GetPackedQuad(range.charData, m_atlasWidth, m_atlasHeight, c - 32,
                              &curX, &curY, &q, 0);

          float quadW = (q.x1 - q.x0) * scale;
          float quadH = (q.y1 - q.y0) * scale;
          float drawX = prevX + (q.x0 - prevX) * scale;
          float drawY = prevY + (q.y0 - prevY) * scale;

          SDL_FRect srcRect{q.s0 * m_atlasWidth, q.t0 * m_atlasHeight,
                            (q.s1 - q.s0) * m_atlasWidth, (q.t1 - q.t0) * m_atlasHeight};
          SDL_FRect dstRect{drawX, drawY, quadW, quadH};

          SDL_RenderTexture(renderer, m_texture, &srcRect, &dstRect);
          curX = prevX + (curX - prevX) * scale;
        } else if (c == ' ') {
          curX += (reqSize * 0.28f);
        }
      }
    } else {
      drawFallbackBitmapText(renderer, text, position, color, reqSize / 16.0f);
    }
  }

  // Global default font: loads platform system or asset TrueType font
  static std::shared_ptr<Font> getDefaultFont() {
    if (!s_defaultFont) {
      s_defaultFont = std::make_shared<Font>();
      loadPlatformSystemFont(s_defaultFont.get());
    } else if (!s_defaultFont->isTTF() && s_defaultRenderer) {
      loadPlatformSystemFont(s_defaultFont.get());
    }
    return s_defaultFont;
  }

  // Attempts to load standard platform or project asset default TrueType font onto a Font instance
  static bool loadPlatformSystemFont(Font *font) {
    if (!font || !s_defaultRenderer) return false;

    for (const auto &cand : getSystemFontCandidates()) {
      std::string actualPath = resolvePath(cand);
      if (std::filesystem::exists(actualPath)) {
        if (font->loadFromFile(actualPath, 18.0f, s_defaultRenderer)) {
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
  }

  bool isTTF() const { return m_isTTF; }
  float getBaseFontSize() const { return m_baseFontSize; }

private:
  struct SizeRange {
    float fontSize = 18.0f;
    stbtt_packedchar charData[96]{};
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

    static const uint8_t font5x7[][5] = {
      {0x00,0x00,0x00,0x00,0x00}, // ' ' (32)
      {0x00,0x00,0x5F,0x00,0x00}, // '!'
      {0x00,0x07,0x00,0x07,0x00}, // '"'
      {0x14,0x7F,0x14,0x7F,0x14}, // '#'
      {0x24,0x2A,0x7F,0x2A,0x12}, // '$'
      {0x23,0x13,0x08,0x64,0x62}, // '%'
      {0x36,0x49,0x55,0x22,0x50}, // '&'
      {0x00,0x05,0x03,0x00,0x00}, // '''
      {0x00,0x1C,0x22,0x41,0x00}, // '('
      {0x00,0x41,0x22,0x1C,0x00}, // ')'
      {0x08,0x2A,0x1C,0x2A,0x08}, // '*'
      {0x08,0x08,0x3E,0x08,0x08}, // '+'
      {0x00,0x50,0x30,0x00,0x00}, // ','
      {0x08,0x08,0x08,0x08,0x08}, // '-'
      {0x00,0x60,0x60,0x00,0x00}, // '.'
      {0x20,0x10,0x08,0x04,0x02}, // '/'
      {0x3E,0x51,0x49,0x45,0x3E}, // '0'
      {0x00,0x42,0x7F,0x40,0x00}, // '1'
      {0x42,0x61,0x51,0x49,0x46}, // '2'
      {0x21,0x41,0x45,0x4B,0x31}, // '3'
      {0x18,0x14,0x12,0x7F,0x10}, // '4'
      {0x27,0x45,0x45,0x45,0x39}, // '5'
      {0x3C,0x4A,0x49,0x49,0x30}, // '6'
      {0x01,0x71,0x09,0x05,0x03}, // '7'
      {0x36,0x49,0x49,0x49,0x36}, // '8'
      {0x06,0x49,0x49,0x29,0x1E}, // '9'
      {0x00,0x36,0x36,0x00,0x00}, // ':'
      {0x00,0x56,0x36,0x00,0x00}, // ';'
      {0x08,0x14,0x22,0x41,0x00}, // '<'
      {0x14,0x14,0x14,0x14,0x14}, // '='
      {0x00,0x41,0x22,0x14,0x08}, // '>'
      {0x02,0x01,0x51,0x09,0x06}, // '?'
      {0x32,0x49,0x79,0x41,0x3E}, // '@'
      {0x7E,0x11,0x11,0x11,0x7E}, // 'A'
      {0x7F,0x49,0x49,0x49,0x36}, // 'B'
      {0x3E,0x41,0x41,0x41,0x22}, // 'C'
      {0x7F,0x41,0x41,0x22,0x1C}, // 'D'
      {0x7F,0x49,0x49,0x49,0x41}, // 'E'
      {0x7F,0x09,0x09,0x09,0x01}, // 'F'
      {0x3E,0x41,0x49,0x49,0x7A}, // 'G'
      {0x7F,0x08,0x08,0x08,0x7F}, // 'H'
      {0x00,0x41,0x7F,0x41,0x00}, // 'I'
      {0x20,0x40,0x41,0x3F,0x01}, // 'J'
      {0x7F,0x08,0x14,0x22,0x41}, // 'K'
      {0x7F,0x40,0x40,0x40,0x40}, // 'L'
      {0x7F,0x02,0x0C,0x02,0x7F}, // 'M'
      {0x7F,0x04,0x08,0x10,0x7F}, // 'N'
      {0x3E,0x41,0x41,0x41,0x3E}, // 'O'
      {0x7F,0x09,0x09,0x09,0x06}, // 'P'
      {0x3E,0x41,0x51,0x21,0x5E}, // 'Q'
      {0x7F,0x09,0x19,0x29,0x46}, // 'R'
      {0x46,0x49,0x49,0x49,0x31}, // 'S'
      {0x01,0x01,0x7F,0x01,0x01}, // 'T'
      {0x3F,0x40,0x40,0x40,0x3F}, // 'U'
      {0x1F,0x20,0x40,0x20,0x1F}, // 'V'
      {0x3F,0x40,0x38,0x40,0x3F}, // 'W'
      {0x63,0x14,0x08,0x14,0x63}, // 'X'
      {0x07,0x08,0x70,0x08,0x07}, // 'Y'
      {0x61,0x51,0x49,0x45,0x43}, // 'Z'
      {0x00,0x7F,0x41,0x41,0x00}, // '['
      {0x02,0x04,0x08,0x10,0x20}, // '\'
      {0x00,0x41,0x41,0x7F,0x00}, // ']'
      {0x04,0x02,0x01,0x02,0x04}, // '^'
      {0x40,0x40,0x40,0x40,0x40}, // '_'
      {0x00,0x01,0x02,0x04,0x00}, // '`'
      {0x20,0x54,0x54,0x54,0x78}, // 'a'
      {0x7F,0x48,0x44,0x44,0x38}, // 'b'
      {0x38,0x44,0x44,0x44,0x20}, // 'c'
      {0x38,0x44,0x44,0x48,0x7F}, // 'd'
      {0x38,0x54,0x54,0x54,0x18}, // 'e'
      {0x08,0x7E,0x09,0x01,0x02}, // 'f'
      {0x0C,0x52,0x52,0x52,0x3E}, // 'g'
      {0x7F,0x08,0x04,0x04,0x78}, // 'h'
      {0x00,0x44,0x7D,0x40,0x00}, // 'i'
      {0x20,0x40,0x44,0x3D,0x00}, // 'j'
      {0x7F,0x10,0x28,0x44,0x00}, // 'k'
      {0x00,0x41,0x7F,0x40,0x00}, // 'l'
      {0x7C,0x04,0x18,0x04,0x78}, // 'm'
      {0x7C,0x08,0x04,0x04,0x78}, // 'n'
      {0x38,0x44,0x44,0x44,0x38}, // 'o'
      {0x7C,0x14,0x14,0x14,0x08}, // 'p'
      {0x08,0x14,0x14,0x18,0x7C}, // 'q'
      {0x7C,0x08,0x04,0x04,0x08}, // 'r'
      {0x48,0x54,0x54,0x54,0x20}, // 's'
      {0x04,0x3F,0x44,0x40,0x20}, // 't'
      {0x3C,0x40,0x40,0x20,0x7C}, // 'u'
      {0x1C,0x20,0x40,0x20,0x1C}, // 'v'
      {0x3C,0x40,0x30,0x40,0x3C}, // 'w'
      {0x44,0x28,0x10,0x28,0x44}, // 'x'
      {0x0C,0x50,0x50,0x50,0x3C}, // 'y'
      {0x44,0x64,0x54,0x4C,0x44}  // 'z'
    };

    float pixelW = std::max(1.0f, scale);
    float pixelH = std::max(1.0f, scale);
    float curX = pos.x;

    for (char c : text) {
      if (c >= 32 && c <= 122) {
        int glyphIdx = c - 32;
        const uint8_t *cols = font5x7[glyphIdx];

        for (int col = 0; col < 5; ++col) {
          uint8_t line = cols[col];
          for (int row = 0; row < 7; ++row) {
            if (line & (1 << row)) {
              SDL_FRect pixelRect{curX + col * pixelW, pos.y + row * pixelH,
                                  pixelW, pixelH};
              SDL_RenderFillRect(renderer, &pixelRect);
            }
          }
        }
        curX += (6 * pixelW);
      } else if (c == ' ') {
        curX += (4 * pixelW);
      }
    }
  }

  SDL_Texture *m_texture = nullptr;
  int m_atlasWidth = 0;
  int m_atlasHeight = 0;
  float m_baseFontSize = 18.0f;
  bool m_isTTF = false;
  std::string m_path;
  std::vector<SizeRange> m_ranges;

  inline static std::shared_ptr<Font> s_defaultFont = nullptr;
  inline static SDL_Renderer *s_defaultRenderer = nullptr;
};
