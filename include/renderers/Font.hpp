#pragma once

#include "helper/color/Color.hpp"
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

// Font Resource representing loaded TrueType / OpenType font or built-in crisp fallback font.
class Font {
public:
  Font() {
    createDefaultBitmapFont();
  }

  explicit Font(const std::string &filePath, float fontSize = 36.0f,
                SDL_Renderer *renderer = nullptr) {
    if (!loadFromFile(filePath, fontSize, renderer)) {
      createDefaultBitmapFont();
    }
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
        m_atlasHeight(other.m_atlasHeight), m_fontSize(other.m_fontSize),
        m_isTTF(other.m_isTTF), m_path(std::move(other.m_path)) {
    std::copy(std::begin(other.m_cdata), std::end(other.m_cdata), std::begin(m_cdata));
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
      m_fontSize = other.m_fontSize;
      m_isTTF = other.m_isTTF;
      m_path = std::move(other.m_path);
      std::copy(std::begin(other.m_cdata), std::end(other.m_cdata), std::begin(m_cdata));

      other.m_texture = nullptr;
      other.m_isTTF = false;
    }
    return *this;
  }

  // Resolves file path across common directory layouts
  static std::string resolvePath(const std::string &path) {
    if (std::filesystem::exists(path)) return path;
    if (std::filesystem::exists("../" + path)) return "../" + path;
    if (std::filesystem::exists("../../" + path)) return "../../" + path;
    return path;
  }

  // Returns list of standard system default font candidates for the current OS
  static std::vector<std::string> getSystemFontCandidates() {
    return {
      // Windows standard fonts
      "C:/Windows/Fonts/segoeui.ttf",
      "C:/Windows/Fonts/arial.ttf",
      "C:/Windows/Fonts/calibri.ttf",
      "C:/Windows/Fonts/tahoma.ttf",
      // Linux standard fonts
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
      "/usr/share/fonts/TTF/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
      // macOS standard fonts
      "/System/Library/Fonts/SFNS.ttf",
      "/Library/Fonts/Arial.ttf",
      "/System/Library/Fonts/Helvetica.ttc"
    };
  }

  // Loads a TrueType (.ttf) or OpenType (.otf) font from file.
  bool loadFromFile(const std::string &filePath, float fontSize = 36.0f,
                    SDL_Renderer *renderer = nullptr) {
    destroy();
    m_path = filePath;
    m_fontSize = fontSize;

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

    int res = stbtt_BakeFontBitmap(ttfBuffer.data(), 0, fontSize,
                                   tempBitmap.data(), m_atlasWidth, m_atlasHeight,
                                   32, 96, m_cdata); // ASCII 32..126
    if (res <= 0) {
      return false;
    }

    // Convert 8-bit alpha mask to 32-bit RGBA texture
    std::vector<uint32_t> rgbaPixels(m_atlasWidth * m_atlasHeight);
    for (size_t i = 0; i < tempBitmap.size(); ++i) {
      uint8_t alpha = tempBitmap[i];
      rgbaPixels[i] = (static_cast<uint32_t>(alpha) << 24) | 0x00FFFFFF;
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

  // Returns single-line text dimensions in pixels.
  Vector2 getStringSize(const std::string &text, float customFontSize = 0.0f) const {
    if (text.empty()) return {0.0f, 0.0f};

    float scale = (customFontSize > 0.0f && m_fontSize > 0.0f)
                      ? (customFontSize / m_fontSize)
                      : 1.0f;

    if (!m_isTTF) {
      return {static_cast<float>(text.length()) * 8.0f * scale, 16.0f * scale};
    }

    float totalAdvance = 0.0f;
    for (char c : text) {
      if (c >= 32 && c < 128) {
        stbtt_aligned_quad q;
        float tempX = 0.0f, tempY = 0.0f;
        stbtt_GetBakedQuad(m_cdata, m_atlasWidth, m_atlasHeight,
                           c - 32, &tempX, &tempY, &q, 1);
        totalAdvance += tempX * scale;
      } else if (c == ' ') {
        totalAdvance += (m_fontSize * 0.28f * scale);
      }
    }

    float textHeight = (customFontSize > 0.0f) ? customFontSize : m_fontSize;
    return {totalAdvance, textHeight};
  }

  // Renders text string to the screen with linear subpixel scaling.
  void drawText(SDL_Renderer *renderer, const std::string &text,
                const Vector2 &position, const Color &color,
                float customFontSize = 0.0f) const {
    if (!renderer || text.empty()) return;

    float targetSize = (customFontSize > 0.0f) ? customFontSize : m_fontSize;
    float scale = (m_fontSize > 0.0f) ? (targetSize / m_fontSize) : 1.0f;

    if (m_isTTF && m_texture) {
      SDL_SetTextureColorModFloat(m_texture, color.r, color.g, color.b);
      SDL_SetTextureAlphaModFloat(m_texture, color.a);

      float curX = position.x;
      float curY = position.y + (m_fontSize * scale * 0.78f); // baseline

      for (char c : text) {
        if (c >= 32 && c < 128) {
          stbtt_aligned_quad q;
          float tempX = 0.0f, tempY = 0.0f;
          stbtt_GetBakedQuad(m_cdata, m_atlasWidth, m_atlasHeight,
                             c - 32, &tempX, &tempY, &q, 1);

          float drawX = curX + q.x0 * scale;
          float drawY = curY + q.y0 * scale;
          float quadW = (q.x1 - q.x0) * scale;
          float quadH = (q.y1 - q.y0) * scale;

          SDL_FRect srcRect{q.s0 * m_atlasWidth, q.t0 * m_atlasHeight,
                            (q.s1 - q.s0) * m_atlasWidth, (q.t1 - q.t0) * m_atlasHeight};
          SDL_FRect dstRect{drawX, drawY, quadW, quadH};

          SDL_RenderTexture(renderer, m_texture, &srcRect, &dstRect);
          curX += tempX * scale;
        } else if (c == ' ') {
          curX += (m_fontSize * 0.28f * scale);
        }
      }
    } else {
      drawFallbackBitmapText(renderer, text, position, color, scale);
    }
  }

  // Global default font: loads platform system TrueType font (e.g. Segoe UI / Arial) or fallback
  static std::shared_ptr<Font> getDefaultFont() {
    if (!s_defaultFont) {
      s_defaultFont = std::make_shared<Font>();
      loadPlatformSystemFont(s_defaultFont.get());
    } else if (!s_defaultFont->isTTF() && s_defaultRenderer) {
      loadPlatformSystemFont(s_defaultFont.get());
    }
    return s_defaultFont;
  }

  // Attempts to load standard platform default TrueType font onto a Font instance
  static bool loadPlatformSystemFont(Font *font) {
    if (!font || !s_defaultRenderer) return false;

    for (const auto &cand : getSystemFontCandidates()) {
      if (std::filesystem::exists(cand)) {
        if (font->loadFromFile(cand, 36.0f, s_defaultRenderer)) {
          return true;
        }
      }
    }
    return false;
  }

  static void setDefaultRenderer(SDL_Renderer *renderer) {
    s_defaultRenderer = renderer;
    if (s_defaultFont && !s_defaultFont->isTTF()) {
      loadPlatformSystemFont(s_defaultFont.get());
    }
  }

  void destroy() {
    if (m_texture) {
      SDL_DestroyTexture(m_texture);
      m_texture = nullptr;
    }
    m_isTTF = false;
  }

  bool isTTF() const { return m_isTTF; }
  float getBaseFontSize() const { return m_fontSize; }

private:
  void createDefaultBitmapFont() {
    m_fontSize = 16.0f;
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
  float m_fontSize = 36.0f;
  bool m_isTTF = false;
  std::string m_path;
  stbtt_bakedchar m_cdata[96]{};

  inline static std::shared_ptr<Font> s_defaultFont = nullptr;
  inline static SDL_Renderer *s_defaultRenderer = nullptr;
};
