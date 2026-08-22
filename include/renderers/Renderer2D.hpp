#pragma once

#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Camera2D.hpp"
#include "renderers/Texture2D.hpp"
#include "window.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <vector>

// Hardware-accelerated 2D Primitive and Batch Renderer.
class Renderer2D {
public:
  // Initializes the 2D renderer with an active SDL_Renderer.
  static void init(SDL_Renderer *renderer) {
    s_renderer = renderer;
    Texture2D::setDefaultRenderer(renderer);
  }

  // Initializes the 2D renderer using a Window instance.
  static void init(Window &window) {
    s_renderer = window.getRenderer();
    Texture2D::setDefaultRenderer(s_renderer);
  }

  // Begins a 2D rendering pass without camera transformation (screen coordinates).
  static void begin() {
    s_activeCamera = nullptr;
  }

  // Begins a 2D rendering pass with world-to-screen Camera2D transformation.
  static void begin(const Camera2D &camera) {
    s_activeCamera = &camera;
  }

  // Ends the current 2D rendering pass.
  static void end() {
    s_activeCamera = nullptr;
  }

  // Draws a 2D rectangle.
  static void drawRect(const Vector2 &position, const Vector2 &size,
                       const Color &color, bool filled = true) {
    if (!s_renderer) return;

    if (!s_activeCamera) {
      setDrawColor(color);
      SDL_FRect rect{position.x, position.y, size.x, size.y};
      if (filled) {
        SDL_RenderFillRect(s_renderer, &rect);
      } else {
        SDL_RenderRect(s_renderer, &rect);
      }
      return;
    }

    // When camera is active, transform quad corners
    Vector2 p0 = toScreen(position);
    Vector2 p1 = toScreen(position + Vector2(size.x, 0.0f));
    Vector2 p2 = toScreen(position + size);
    Vector2 p3 = toScreen(position + Vector2(0.0f, size.y));

    if (filled) {
      drawQuadGeometry(p0, p1, p2, p3, color);
    } else {
      SDL_FPoint points[5] = {
          {p0.x, p0.y}, {p1.x, p1.y}, {p2.x, p2.y}, {p3.x, p3.y}, {p0.x, p0.y}};
      setDrawColor(color);
      SDL_RenderLines(s_renderer, points, 5);
    }
  }

  // Draws a rotated 2D rectangle (rotation in radians, rotated around center).
  static void drawRectRotated(const Vector2 &position, const Vector2 &size,
                              float rotationRadians, const Color &color,
                              bool filled = true) {
    if (!s_renderer) return;

    Vector2 halfSize = size * 0.5f;
    Vector2 center = position + halfSize;

    // Corner offsets from center
    Vector2 c0(-halfSize.x, -halfSize.y);
    Vector2 c1(halfSize.x, -halfSize.y);
    Vector2 c2(halfSize.x, halfSize.y);
    Vector2 c3(-halfSize.x, halfSize.y);

    if (rotationRadians != 0.0f) {
      c0 = c0.rotated(rotationRadians);
      c1 = c1.rotated(rotationRadians);
      c2 = c2.rotated(rotationRadians);
      c3 = c3.rotated(rotationRadians);
    }

    Vector2 p0 = toScreen(center + c0);
    Vector2 p1 = toScreen(center + c1);
    Vector2 p2 = toScreen(center + c2);
    Vector2 p3 = toScreen(center + c3);

    if (filled) {
      drawQuadGeometry(p0, p1, p2, p3, color);
    } else {
      SDL_FPoint points[5] = {
          {p0.x, p0.y}, {p1.x, p1.y}, {p2.x, p2.y}, {p3.x, p3.y}, {p0.x, p0.y}};
      setDrawColor(color);
      SDL_RenderLines(s_renderer, points, 5);
    }
  }

  // Draws a 2D circle with configurable segment precision.
  static void drawCircle(const Vector2 &center, float radius,
                         const Color &color, bool filled = true,
                         int segments = 32) {
    if (!s_renderer || segments < 3 || radius <= 0.0f) return;

    Vector2 screenCenter = toScreen(center);
    float screenRadius = s_activeCamera ? (radius * s_activeCamera->zoom) : radius;
    float step = (2.0f * 3.14159265358979323846f) / static_cast<float>(segments);

    if (filled) {
      SDL_FColor sdlColor = toSDLColor(color);
      std::vector<SDL_Vertex> vertices;
      vertices.reserve(segments + 2);

      // Center vertex
      vertices.push_back({{screenCenter.x, screenCenter.y}, sdlColor, {0.0f, 0.0f}});

      for (int i = 0; i <= segments; ++i) {
        float angle = static_cast<float>(i) * step;
        float x = screenCenter.x + std::cos(angle) * screenRadius;
        float y = screenCenter.y + std::sin(angle) * screenRadius;
        vertices.push_back({{x, y}, sdlColor, {0.0f, 0.0f}});
      }

      std::vector<int> indices;
      indices.reserve(segments * 3);
      for (int i = 1; i <= segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
      }

      SDL_RenderGeometry(s_renderer, nullptr, vertices.data(),
                         static_cast<int>(vertices.size()), indices.data(),
                         static_cast<int>(indices.size()));
    } else {
      std::vector<SDL_FPoint> points;
      points.reserve(segments + 1);
      for (int i = 0; i <= segments; ++i) {
        float angle = static_cast<float>(i) * step;
        points.push_back({screenCenter.x + std::cos(angle) * screenRadius,
                          screenCenter.y + std::sin(angle) * screenRadius});
      }
      setDrawColor(color);
      SDL_RenderLines(s_renderer, points.data(), static_cast<int>(points.size()));
    }
  }

  // Draws a 2D line with custom thickness.
  static void drawLine(const Vector2 &start, const Vector2 &end,
                       const Color &color, float thickness = 1.0f) {
    if (!s_renderer) return;

    Vector2 p1 = toScreen(start);
    Vector2 p2 = toScreen(end);

    if (thickness <= 1.0f) {
      setDrawColor(color);
      SDL_RenderLine(s_renderer, p1.x, p1.y, p2.x, p2.y);
      return;
    }

    // Thick line rendered as a quad
    Vector2 dir = p2 - p1;
    if (dir.is_zero_approx()) return;

    Vector2 normal = dir.orthogonal().normalized() * (thickness * 0.5f);
    Vector2 v0 = p1 + normal;
    Vector2 v1 = p2 + normal;
    Vector2 v2 = p2 - normal;
    Vector2 v3 = p1 - normal;

    drawQuadGeometry(v0, v1, v2, v3, color);
  }

  // Draws a single 2D pixel point.
  static void drawPoint(const Vector2 &position, const Color &color) {
    if (!s_renderer) return;
    Vector2 p = toScreen(position);
    setDrawColor(color);
    SDL_RenderPoint(s_renderer, p.x, p.y);
  }

  // Draws a 2D triangle.
  static void drawTriangle(const Vector2 &p1, const Vector2 &p2,
                           const Vector2 &p3, const Color &color,
                           bool filled = true) {
    if (!s_renderer) return;

    Vector2 sp1 = toScreen(p1);
    Vector2 sp2 = toScreen(p2);
    Vector2 sp3 = toScreen(p3);

    if (filled) {
      SDL_FColor sdlColor = toSDLColor(color);
      SDL_Vertex verts[3] = {
          {{sp1.x, sp1.y}, sdlColor, {0.0f, 0.0f}},
          {{sp2.x, sp2.y}, sdlColor, {0.0f, 0.0f}},
          {{sp3.x, sp3.y}, sdlColor, {0.0f, 0.0f}}};
      int indices[3] = {0, 1, 2};
      SDL_RenderGeometry(s_renderer, nullptr, verts, 3, indices, 3);
    } else {
      SDL_FPoint points[4] = {{sp1.x, sp1.y},
                              {sp2.x, sp2.y},
                              {sp3.x, sp3.y},
                              {sp1.x, sp1.y}};
      setDrawColor(color);
      SDL_RenderLines(s_renderer, points, 4);
    }
  }

  // Draws a connected sequence of line segments.
  static void drawLines(const std::vector<Vector2> &points, const Color &color,
                        bool closed = false) {
    if (!s_renderer || points.size() < 2) return;

    std::vector<SDL_FPoint> sdlPoints;
    sdlPoints.reserve(points.size() + (closed ? 1 : 0));

    for (const auto &pt : points) {
      Vector2 sp = toScreen(pt);
      sdlPoints.push_back({sp.x, sp.y});
    }

    if (closed) {
      Vector2 sp0 = toScreen(points[0]);
      sdlPoints.push_back({sp0.x, sp0.y});
    }

    setDrawColor(color);
    SDL_RenderLines(s_renderer, sdlPoints.data(), static_cast<int>(sdlPoints.size()));
  }

  // Draws a 2D texture with optional tint, rotation, and flipping.
  static void drawTexture(const Texture2D &texture, const Vector2 &position,
                          const Vector2 &size, const Color &tint = Color::WHITE,
                          float rotationRadians = 0.0f, bool flipH = false,
                          bool flipV = false) {
    if (!s_renderer || !texture.isValid()) return;

    SDL_Texture *nativeTex = texture.getNativeTexture();
    SDL_SetTextureColorModFloat(nativeTex, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaModFloat(nativeTex, tint.a);

    Vector2 screenPos = toScreen(position);
    Vector2 screenSize = s_activeCamera ? (size * s_activeCamera->zoom) : size;
    SDL_FRect dstRect{screenPos.x, screenPos.y, screenSize.x, screenSize.y};

    float totalRot = rotationRadians + (s_activeCamera ? s_activeCamera->getRotation() : 0.0f);
    float angleDegrees = totalRot * (180.0f / 3.14159265358979323846f);

    SDL_FlipMode flip = SDL_FLIP_NONE;
    if (flipH && flipV) {
      flip = static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
    } else if (flipH) {
      flip = SDL_FLIP_HORIZONTAL;
    } else if (flipV) {
      flip = SDL_FLIP_VERTICAL;
    }

    if (totalRot == 0.0f && flip == SDL_FLIP_NONE) {
      SDL_RenderTexture(s_renderer, nativeTex, nullptr, &dstRect);
    } else {
      SDL_FPoint center{screenSize.x * 0.5f, screenSize.y * 0.5f};
      SDL_RenderTextureRotated(s_renderer, nativeTex, nullptr, &dstRect,
                               angleDegrees, &center, flip);
    }
  }

private:
  // Converts world position to active screen coordinate.
  static Vector2 toScreen(const Vector2 &worldPos) {
    return s_activeCamera ? s_activeCamera->worldToScreen(worldPos) : worldPos;
  }

  // Sets active draw color on SDL_Renderer.
  static void setDrawColor(const Color &color) {
    uint8_t r = static_cast<uint8_t>(std::clamp(color.r * 255.0f + 0.5f, 0.0f, 255.0f));
    uint8_t g = static_cast<uint8_t>(std::clamp(color.g * 255.0f + 0.5f, 0.0f, 255.0f));
    uint8_t b = static_cast<uint8_t>(std::clamp(color.b * 255.0f + 0.5f, 0.0f, 255.0f));
    uint8_t a = static_cast<uint8_t>(std::clamp(color.a * 255.0f + 0.5f, 0.0f, 255.0f));
    SDL_SetRenderDrawColor(s_renderer, r, g, b, a);
  }

  // Converts MSL Color to SDL_FColor.
  static SDL_FColor toSDLColor(const Color &color) {
    return SDL_FColor{color.r, color.g, color.b, color.a};
  }

  // Renders a 4-vertex quad using SDL_RenderGeometry.
  static void drawQuadGeometry(const Vector2 &p0, const Vector2 &p1,
                               const Vector2 &p2, const Vector2 &p3,
                               const Color &color) {
    SDL_FColor sdlColor = toSDLColor(color);
    SDL_Vertex verts[4] = {
        {{p0.x, p0.y}, sdlColor, {0.0f, 0.0f}},
        {{p1.x, p1.y}, sdlColor, {0.0f, 0.0f}},
        {{p2.x, p2.y}, sdlColor, {0.0f, 0.0f}},
        {{p3.x, p3.y}, sdlColor, {0.0f, 0.0f}}};
    int indices[6] = {0, 1, 2, 2, 3, 0};
    SDL_RenderGeometry(s_renderer, nullptr, verts, 4, indices, 6);
  }

  inline static SDL_Renderer *s_renderer = nullptr;
  inline static const Camera2D *s_activeCamera = nullptr;
};
