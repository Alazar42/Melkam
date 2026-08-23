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

// 2D Textured & Colored Polygon Visual Node (inspired by Godot Polygon2D)
class Polygon2D : public Node2D {
public:
  std::vector<Vector2> polygon;
  std::vector<Vector2> uv;
  std::vector<Color> vertexColors;
  Color color = Color::WHITE;
  Ref<Texture2D> texture = nullptr;
  Vector2 textureOffset{0.0f, 0.0f};
  Vector2 textureScale{1.0f, 1.0f};
  float textureRotation = 0.0f;
  bool antialiased = false;

  Polygon2D() : Node2D("Polygon2D") {}

  void setPolygon(std::vector<Vector2> points) {
    polygon = std::move(points);
    m_trianglesCache.clear();
  }

  void onDraw() override {
    if (polygon.size() < 3) return;

    if (m_trianglesCache.empty() || m_cachedVertexCount != polygon.size()) {
      triangulate();
    }
    if (m_trianglesCache.empty()) return;

    Transform2D globalTransform = getGlobalTransform();
    SDL_FColor defaultSdlCol = Renderer2D::toSDLColor(color);

    std::vector<SDL_Vertex> vertices;
    vertices.reserve(polygon.size());

    for (size_t i = 0; i < polygon.size(); ++i) {
      Vector2 worldPt = globalTransform.transformPoint(polygon[i]);
      Vector2 screenPt = Renderer2D::toScreen(worldPt);

      SDL_FColor vCol = defaultSdlCol;
      if (i < vertexColors.size()) {
        vCol = Renderer2D::toSDLColor(vertexColors[i] * color);
      }

      Vector2 uvCoord{0.0f, 0.0f};
      if (i < uv.size()) {
        uvCoord = uv[i];
      } else {
        // Auto UV mapping from vertex position
        uvCoord = (polygon[i] + textureOffset) * textureScale;
      }

      vertices.push_back({{screenPt.x, screenPt.y}, vCol, {uvCoord.x, uvCoord.y}});
    }

    // Safety guard: ensure indices are within vertices bounds
    for (int idx : m_trianglesCache) {
      if (idx < 0 || idx >= static_cast<int>(vertices.size())) {
        return;
      }
    }

    SDL_Renderer *sdlRenderer = Renderer2D::getRenderer();
    if (!sdlRenderer) return;

    SDL_Texture *nativeTex = (texture && texture->isValid()) ? texture->getNativeTexture() : nullptr;
    SDL_RenderGeometry(sdlRenderer, nativeTex, vertices.data(),
                       static_cast<int>(vertices.size()), m_trianglesCache.data(),
                       static_cast<int>(m_trianglesCache.size()));
  }


private:
  // Ear-Clipping Triangulation for 2D Polygons
  void triangulate() {
    m_trianglesCache.clear();
    m_cachedVertexCount = polygon.size();
    int n = static_cast<int>(polygon.size());
    if (n < 3) return;

    std::vector<int> V(n);
    if (computePolygonArea(polygon) > 0.0f) {
      for (int v = 0; v < n; ++v) V[v] = v;
    } else {
      for (int v = 0; v < n; ++v) V[v] = (n - 1) - v;
    }

    int nv = n;
    int count = 2 * nv;
    for (int v = nv - 1; nv > 2;) {
      if ((count--) <= 0) break; // Error detection

      int u = v; if (nv <= u) u = 0;
      v = u + 1; if (nv <= v) v = 0;
      int w = v + 1; if (nv <= w) w = 0;

      if (snip(polygon, u, v, w, nv, V)) {
        int a = V[u];
        int b = V[v];
        int c = V[w];
        m_trianglesCache.push_back(a);
        m_trianglesCache.push_back(b);
        m_trianglesCache.push_back(c);

        for (int s = v, t = v + 1; t < nv; ++s, ++t) {
          V[s] = V[t];
        }
        nv--;
        count = 2 * nv;
      }
    }
  }

  static float computePolygonArea(const std::vector<Vector2> &poly) {
    int n = static_cast<int>(poly.size());
    float A = 0.0f;
    for (int p = n - 1, q = 0; q < n; p = q++) {
      A += poly[p].x * poly[q].y - poly[q].x * poly[p].y;
    }
    return A * 0.5f;
  }

  static bool isInsideTriangle(const Vector2 &A, const Vector2 &B, const Vector2 &C, const Vector2 &P) {
    float ax = C.x - B.x; float ay = C.y - B.y;
    float bx = A.x - C.x; float by = A.y - C.y;
    float cx = B.x - A.x; float cy = B.y - A.y;
    float apx = P.x - A.x; float apy = P.y - A.y;
    float bpx = P.x - B.x; float bpy = P.y - B.y;
    float cpx = P.x - C.x; float cpy = P.y - C.y;

    float aCrossbp = ax * bpy - ay * bpx;
    float cCrossap = cx * apy - cy * apx;
    float bCrosscp = bx * cpy - by * cpx;

    return ((aCrossbp >= 0.0f) && (bCrosscp >= 0.0f) && (cCrossap >= 0.0f));
  }

  static bool snip(const std::vector<Vector2> &poly, int u, int v, int w, int n, const std::vector<int> &V) {
    const Vector2 &A = poly[V[u]];
    const Vector2 &B = poly[V[v]];
    const Vector2 &C = poly[V[w]];

    if (1e-6f > (((B.x - A.x) * (C.y - A.y)) - ((B.y - A.y) * (C.x - A.x)))) {
      return false;
    }

    for (int p = 0; p < n; ++p) {
      if ((p == u) || (p == v) || (p == w)) continue;
      const Vector2 &P = poly[V[p]];
      if (isInsideTriangle(A, B, C, P)) return false;
    }
    return true;
  }

  std::vector<int> m_trianglesCache;
  size_t m_cachedVertexCount = 0;
};
