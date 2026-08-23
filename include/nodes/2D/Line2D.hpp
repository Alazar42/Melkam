#pragma once

#include "core/Memory.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <vector>

enum class LineJointMode {
  Sharp,
  Bevel,
  Round
};

enum class LineCapMode {
  None,
  Box,
  Round
};

enum class LineTextureMode {
  None,
  Tile,
  Stretch
};

// Multi-Segment 2D Polyline Visual Node (inspired by Godot Line2D)
class Line2D : public Node2D {
public:
  std::vector<Vector2> points;
  float width = 10.0f;
  Color defaultColor = Color::WHITE;
  std::vector<Color> gradient;
  LineJointMode jointMode = LineJointMode::Sharp;
  LineCapMode beginCapMode = LineCapMode::None;
  LineCapMode endCapMode = LineCapMode::None;
  LineTextureMode textureMode = LineTextureMode::None;
  Ref<Texture2D> texture = nullptr;
  bool closed = false;
  bool antialiased = false;

  Line2D() : Node2D("Line2D") {}

  void addPoint(const Vector2 &point) {
    points.push_back(point);
  }

  void setPointPosition(int index, const Vector2 &point) {
    if (index >= 0 && index < static_cast<int>(points.size())) {
      points[index] = point;
    }
  }

  Vector2 getPointPosition(int index) const {
    if (index >= 0 && index < static_cast<int>(points.size())) {
      return points[index];
    }
    return {0.0f, 0.0f};
  }

  void removePoint(int index) {
    if (index >= 0 && index < static_cast<int>(points.size())) {
      points.erase(points.begin() + index);
    }
  }

  void clearPoints() {
    points.clear();
  }

  int getPointCount() const {
    return static_cast<int>(points.size());
  }

  Color getColorAtFraction(float t) const {
    if (gradient.empty()) return defaultColor;
    if (gradient.size() == 1) return gradient[0];
    t = std::clamp(t, 0.0f, 1.0f);
    float scaled = t * static_cast<float>(gradient.size() - 1);
    int idx = static_cast<int>(scaled);
    int next = std::min(idx + 1, static_cast<int>(gradient.size()) - 1);
    float f = scaled - static_cast<float>(idx);
    return gradient[idx].lerp(gradient[next], f);
  }

  void onDraw() override {
    if (points.size() < 2 || width <= 0.0f) return;

    Transform2D globalTransform = getGlobalTransform();
    std::vector<Vector2> worldPoints;
    worldPoints.reserve(points.size() + (closed ? 1 : 0));

    for (const auto &pt : points) {
      worldPoints.push_back(globalTransform.transformPoint(pt));
    }
    if (closed && points.size() > 2) {
      worldPoints.push_back(worldPoints[0]);
    }

    size_t count = worldPoints.size();
    if (count < 2) return;

    // Build line strip quads with miter / normal offsets
    float halfW = (width * globalTransform.scale.x) * 0.5f;

    std::vector<SDL_Vertex> vertices;
    std::vector<int> indices;
    vertices.reserve(count * 2);
    indices.reserve((count - 1) * 6);

    for (size_t i = 0; i < count; ++i) {
      Vector2 p = worldPoints[i];
      Vector2 normal{0.0f, 0.0f};

      if (i == 0) {
        Vector2 dir = (worldPoints[1] - p).normalized();
        normal = dir.orthogonal();
      } else if (i == count - 1) {
        Vector2 dir = (p - worldPoints[i - 1]).normalized();
        normal = dir.orthogonal();
      } else {
        Vector2 dir1 = (p - worldPoints[i - 1]).normalized();
        Vector2 dir2 = (worldPoints[i + 1] - p).normalized();
        Vector2 tangent = (dir1 + dir2).normalized();
        normal = tangent.orthogonal();
        float dot = dir1.dot(tangent);
        if (std::abs(dot) > 0.1f) {
          normal = normal * (1.0f / dot);
        }
      }

      Vector2 topPos = p + normal * halfW;
      Vector2 botPos = p - normal * halfW;

      Vector2 topScreen = Renderer2D::toScreen(topPos);
      Vector2 botScreen = Renderer2D::toScreen(botPos);

      float t = static_cast<float>(i) / static_cast<float>(count - 1);
      Color c = getColorAtFraction(t);
      SDL_FColor sdlC = Renderer2D::toSDLColor(c);

      float u = (textureMode == LineTextureMode::Stretch) ? t : static_cast<float>(i);
      vertices.push_back({{topScreen.x, topScreen.y}, sdlC, {u, 0.0f}});
      vertices.push_back({{botScreen.x, botScreen.y}, sdlC, {u, 1.0f}});

      if (i > 0) {
        int vIdx = static_cast<int>(i) * 2;
        indices.push_back(vIdx - 2);
        indices.push_back(vIdx - 1);
        indices.push_back(vIdx);

        indices.push_back(vIdx);
        indices.push_back(vIdx - 1);
        indices.push_back(vIdx + 1);
      }
    }

    SDL_Renderer *sdlRenderer = Renderer2D::getRenderer();
    if (!sdlRenderer) return;


    SDL_Texture *nativeTex = (texture && texture->isValid()) ? texture->getNativeTexture() : nullptr;
    SDL_RenderGeometry(sdlRenderer, nativeTex, vertices.data(),
                       static_cast<int>(vertices.size()), indices.data(),
                       static_cast<int>(indices.size()));
  }
};
