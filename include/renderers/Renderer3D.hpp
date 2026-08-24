#pragma once

#include "components/Components3D.hpp"
#include "nodes/3D/Camera3D.hpp"
#include "nodes/3D/WorldEnvironment.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/vulkan/RenderingDevice3D.hpp"
#include "systems/Systems3D.hpp"
#include "window.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// Master 3D Rendering Subsystem with Pixel-Accurate Z-Buffering & High-Performance Span Rasterization
class Renderer3D {
public:
  inline static const Camera3D *s_activeCamera = nullptr;
  inline static bool s_isRendering = false;

  // Framebuffer & Depth Buffer
  inline static SDL_Texture *s_framebufferTexture = nullptr;
  inline static int s_fbWidth = 0;
  inline static int s_fbHeight = 0;
  inline static std::vector<uint32_t> s_colorBuffer;
  inline static std::vector<float> s_depthBuffer;

  static void init(Window &window) {
    std::cout << "[MelkamEngine::Renderer3D] Initializing 3D Hardware Subsystem..." << std::endl;
    RenderingDevice3D::get().init(window.getNativeWindow());
  }

  static void shutdown() {
    if (s_framebufferTexture) {
      SDL_DestroyTexture(s_framebufferTexture);
      s_framebufferTexture = nullptr;
    }
    s_fbWidth = 0;
    s_fbHeight = 0;
    s_colorBuffer.clear();
    s_depthBuffer.clear();
    RenderingDevice3D::get().shutdown();
  }

  static void begin() {
    s_activeCamera = Camera3D::getCurrent();
    s_isRendering = true;
  }

  static void begin(const Camera3D &camera) {
    s_activeCamera = &camera;
    s_isRendering = true;
  }

  static void end() {
    s_activeCamera = nullptr;
    s_isRendering = false;
  }

  // Renders all ECS 3D meshes with Pixel-Accurate Z-Buffering (No disappearing geometry)
  static void render(float viewportWidth, float viewportHeight) {
    SDL_Renderer *sdlRenderer = Renderer2D::getRenderer();
    if (!sdlRenderer || viewportWidth <= 0.0f || viewportHeight <= 0.0f) return;

    int w = static_cast<int>(viewportWidth);
    int h = static_cast<int>(viewportHeight);

    // 1. Manage Framebuffer Texture & Z-Buffers
    if (w != s_fbWidth || h != s_fbHeight || !s_framebufferTexture) {
      if (s_framebufferTexture) {
        SDL_DestroyTexture(s_framebufferTexture);
      }
      s_framebufferTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
      if (s_framebufferTexture) {
        SDL_SetTextureBlendMode(s_framebufferTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(s_framebufferTexture, SDL_SCALEMODE_LINEAR);
      }
      s_fbWidth = w;
      s_fbHeight = h;
      s_colorBuffer.resize(w * h);
      s_depthBuffer.resize(w * h);
    }

    if (!s_framebufferTexture || s_colorBuffer.empty()) return;

    // 2. Clear Z-Buffer (0.0f represents infinitely far since we use 1/Z)
    std::fill(s_depthBuffer.begin(), s_depthBuffer.end(), 0.0f);

    // 3. Clear Color Buffer / Sky
    const Environment *env = WorldEnvironment::getCurrent();
    Color ambientColor = Color::from_rgba8(55, 65, 85);

    if (env) {
      ambientColor = env->ambientLightColor * env->ambientLightEnergy;
      if (env->backgroundMode == EnvironmentBGMode::Sky) {
        for (int y = 0; y < h; ++y) {
          float t = static_cast<float>(y) / static_cast<float>(std::max(1, h));
          uint8_t r = static_cast<uint8_t>(16.0f + t * 45.0f);
          uint8_t g = static_cast<uint8_t>(22.0f + t * 50.0f);
          uint8_t b = static_cast<uint8_t>(42.0f + t * 65.0f);
          uint32_t skyRowColor = (0xFF << 24) | (r << 16) | (g << 8) | b;
          uint32_t *rowPtr = &s_colorBuffer[y * w];
          std::fill_n(rowPtr, w, skyRowColor);
        }
      } else {
        uint8_t a = static_cast<uint8_t>(std::clamp(env->backgroundColor.a * 255.0f, 0.0f, 255.0f));
        uint8_t r = static_cast<uint8_t>(std::clamp(env->backgroundColor.r * 255.0f, 0.0f, 255.0f));
        uint8_t g = static_cast<uint8_t>(std::clamp(env->backgroundColor.g * 255.0f, 0.0f, 255.0f));
        uint8_t b = static_cast<uint8_t>(std::clamp(env->backgroundColor.b * 255.0f, 0.0f, 255.0f));
        uint32_t clearColor = (a << 24) | (r << 16) | (g << 8) | b;
        std::fill(s_colorBuffer.begin(), s_colorBuffer.end(), clearColor);
      }
    } else {
      std::fill(s_colorBuffer.begin(), s_colorBuffer.end(), 0x00000000);
    }

    Systems3D::updateTransforms();
    Systems3D::updateCameras(viewportWidth, viewportHeight);

    auto meshView = Entity::getRegistry().view<Mesh3DComponent, Transform3DComponent>();
    auto cameraView = Entity::getRegistry().view<Camera3DComponent, Transform3DComponent>();
    auto dirLightView = Entity::getRegistry().view<DirectionalLight3DComponent, Transform3DComponent>();
    auto pointLightView = Entity::getRegistry().view<PointLight3DComponent, Transform3DComponent>();

    const Camera3DComponent *activeCamComp = nullptr;
    Transform3D activeCamTransform;

    for (auto camEntity : cameraView) {
      auto &cam = cameraView.get<Camera3DComponent>(camEntity);
      if (cam.isCurrent) {
        activeCamComp = &cam;
        activeCamTransform = cameraView.get<Transform3DComponent>(camEntity).globalTransform;
        break;
      }
    }

    if (!activeCamComp) {
      if (s_activeCamera) {
        activeCamTransform = s_activeCamera->getGlobalTransform();
      } else {
        return; // No active 3D camera
      }
    }

    // Camera projection parameters
    Transform3D viewInv = activeCamTransform.affine_inverse();
    float fovDeg = activeCamComp ? activeCamComp->fov : 75.0f;
    float fovRad = fovDeg * 3.14159265f / 180.0f;
    float f = 1.0f / std::tan(fovRad * 0.5f);
    float aspect = viewportWidth / viewportHeight;
    float nearPlane = activeCamComp ? activeCamComp->nearPlane : 0.05f;

    // Collect Lights
    struct DirLightData {
      Vector3 dir;
      Color color;
      float energy;
    };
    std::vector<DirLightData> dirLights;
    for (auto e : dirLightView) {
      const auto &l = dirLightView.get<DirectionalLight3DComponent>(e);
      const auto &t = dirLightView.get<Transform3DComponent>(e);
      Vector3 forward = t.globalTransform.basis.xform(l.direction).normalized();
      dirLights.push_back({forward, l.color, l.energy});
    }
    if (dirLights.empty()) {
      dirLights.push_back({Vector3(-0.5f, -0.8f, -0.4f).normalized(), Color::from_rgba8(255, 245, 230), 1.3f});
    }

    struct PointLightData {
      Vector3 pos;
      Color color;
      float energy;
      float range;
    };
    std::vector<PointLightData> pointLights;
    for (auto e : pointLightView) {
      const auto &l = pointLightView.get<PointLight3DComponent>(e);
      const auto &t = pointLightView.get<Transform3DComponent>(e);
      pointLights.push_back({t.globalTransform.origin, l.color, l.energy, l.range});
    }

    struct GlowingHalo {
      Vector2 screenPos;
      float radius;
      Color color;
    };
    std::vector<GlowingHalo> glowHalos;

    float halfW = viewportWidth * 0.5f;
    float halfH = viewportHeight * 0.5f;

    // Rasterize all 3D meshes with Pixel-Accurate Z-Buffering
    for (auto entity : meshView) {
      const auto &meshComp = meshView.get<Mesh3DComponent>(entity);
      const auto &transComp = meshView.get<Transform3DComponent>(entity);

      if (!meshComp.visible || meshComp.vertices.empty() || meshComp.indices.empty()) continue;

      Transform3D globalTrans = transComp.globalTransform;
      Basis normalBasis = globalTrans.basis.inverse().transposed();

      // Emissive radiant light halo detection for coins
      if (meshComp.emissionEnergy > 0.1f) {
        Vector3 centerWorld = globalTrans.origin;
        Vector3 camCenter = viewInv.xform(centerWorld);
        if (-camCenter.z > nearPlane) {
          float invZ = 1.0f / (-camCenter.z);
          float sx = (camCenter.x * f / aspect) * invZ * halfW + halfW;
          float sy = (1.0f - (camCenter.y * f) * invZ) * halfH;
          float haloRad = std::clamp(85.0f * invZ, 12.0f, 150.0f);
          Color haloCol = (meshComp.emissionColor * meshComp.emissionEnergy);
          glowHalos.push_back({{sx, sy}, haloRad, haloCol});
        }
      }

      size_t indexCount = meshComp.indices.size();
      for (size_t i = 0; i + 2 < indexCount; i += 3) {
        const Vertex3D &v0 = meshComp.vertices[meshComp.indices[i]];
        const Vertex3D &v1 = meshComp.vertices[meshComp.indices[i + 1]];
        const Vertex3D &v2 = meshComp.vertices[meshComp.indices[i + 2]];

        // 1. World Space Transformation
        Vector3 w0 = globalTrans.xform(v0.position);
        Vector3 w1 = globalTrans.xform(v1.position);
        Vector3 w2 = globalTrans.xform(v2.position);

        // 2. Normal & Backface Culling
        Vector3 faceNorm = (w1 - w0).cross(w2 - w0);
        Vector3 toCam = activeCamTransform.origin - w0;
        bool isBackFacing = (faceNorm.dot(toCam) < 0.0f);
        if (isBackFacing && meshComp.cullBackfaces) continue;

        // 3. Camera View Space Transformation
        Vector3 cam0 = viewInv.xform(w0);
        Vector3 cam1 = viewInv.xform(w1);
        Vector3 cam2 = viewInv.xform(w2);

        float z0 = -cam0.z;
        float z1 = -cam1.z;
        float z2 = -cam2.z;

        if (z0 <= nearPlane && z1 <= nearPlane && z2 <= nearPlane) continue;

        float safeZ0 = std::max(z0, nearPlane);
        float safeZ1 = std::max(z1, nearPlane);
        float safeZ2 = std::max(z2, nearPlane);

        float invZ0 = 1.0f / safeZ0;
        float invZ1 = 1.0f / safeZ1;
        float invZ2 = 1.0f / safeZ2;

        // 4. Sub-Pixel Screen Projection
        float x0 = (cam0.x * f / aspect) * invZ0 * halfW + halfW;
        float y0 = (1.0f - (cam0.y * f) * invZ0) * halfH;

        float x1 = (cam1.x * f / aspect) * invZ1 * halfW + halfW;
        float y1 = (1.0f - (cam1.y * f) * invZ1) * halfH;

        float x2 = (cam2.x * f / aspect) * invZ2 * halfW + halfW;
        float y2 = (1.0f - (cam2.y * f) * invZ2) * halfH;

        // 5. Lighting Calculations
        Vector3 n0 = normalBasis.xform(v0.normal).normalized();
        Vector3 n1 = normalBasis.xform(v1.normal).normalized();
        Vector3 n2 = normalBasis.xform(v2.normal).normalized();
        if (isBackFacing) { n0 = -n0; n1 = -n1; n2 = -n2; }

        auto computeVertexColor = [&](const Vector3 &pos, const Vector3 &norm, const Color &baseCol) -> Color {
          float totalR = ambientColor.r;
          float totalG = ambientColor.g;
          float totalB = ambientColor.b;

          // Directional Sun Lights
          for (const auto &dl : dirLights) {
            float ndotl = std::max(0.0f, norm.dot(-dl.dir));
            totalR += dl.color.r * dl.energy * ndotl;
            totalG += dl.color.g * dl.energy * ndotl;
            totalB += dl.color.b * dl.energy * ndotl;
          }

          // Omni Point Lights
          for (const auto &pl : pointLights) {
            Vector3 toLight = pl.pos - pos;
            float dist = toLight.length();
            if (dist < pl.range && dist > 0.001f) {
              Vector3 ldir = toLight / dist;
              float ndotl = std::max(0.0f, norm.dot(ldir));
              float atten = std::max(0.0f, 1.0f - (dist / pl.range));
              atten *= atten;
              totalR += pl.color.r * pl.energy * ndotl * atten;
              totalG += pl.color.g * pl.energy * ndotl * atten;
              totalB += pl.color.b * pl.energy * ndotl * atten;
            }
          }

          Color finalCol = baseCol * meshComp.albedoColor * Color(totalR, totalG, totalB, 1.0f) + (meshComp.emissionColor * meshComp.emissionEnergy);
          return Color(
              std::clamp(finalCol.r, 0.0f, 1.0f),
              std::clamp(finalCol.g, 0.0f, 1.0f),
              std::clamp(finalCol.b, 0.0f, 1.0f),
              meshComp.albedoColor.a);
        };

        Color c0 = computeVertexColor(w0, n0, v0.color);
        Color c1 = computeVertexColor(w1, n1, v1.color);
        Color c2 = computeVertexColor(w2, n2, v2.color);

        // 6. High-Performance Edge-Stepping Span Rasterizer with Z-Buffering
        float denom = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);
        if (std::abs(denom) < 0.0001f) continue;
        float invDenom = 1.0f / denom;

        int minY = std::max(0, static_cast<int>(std::floor(std::min({y0, y1, y2}))));
        int maxY = std::min(h - 1, static_cast<int>(std::ceil(std::max({y0, y1, y2}))));
        if (minY > maxY) continue;

        float dInvZ_dx = ((invZ0 - invZ2) * (y1 - y2) + (invZ1 - invZ2) * (y2 - y0)) * invDenom;
        float dR_dx = ((c0.r - c2.r) * (y1 - y2) + (c1.r - c2.r) * (y2 - y0)) * invDenom;
        float dG_dx = ((c0.g - c2.g) * (y1 - y2) + (c1.g - c2.g) * (y2 - y0)) * invDenom;
        float dB_dx = ((c0.b - c2.b) * (y1 - y2) + (c1.b - c2.b) * (y2 - y0)) * invDenom;

        for (int py = minY; py <= maxY; ++py) {
          float curY = static_cast<float>(py) + 0.5f;

          // Find span endpoints for scanline py
          float xA = 0.0f, xB = 0.0f;
          bool hasA = false, hasB = false;

          auto checkEdge = [&](float ax, float ay, float bx, float by) {
            if ((ay <= curY && by > curY) || (by <= curY && ay > curY)) {
              float t = (curY - ay) / (by - ay);
              float ix = ax + t * (bx - ax);
              if (!hasA) { xA = ix; hasA = true; }
              else { xB = ix; hasB = true; }
            }
          };

          checkEdge(x0, y0, x1, y1);
          checkEdge(x1, y1, x2, y2);
          checkEdge(x2, y2, x0, y0);

          if (!hasA || !hasB) continue;
          if (xA > xB) std::swap(xA, xB);

          int startX = std::max(0, static_cast<int>(std::ceil(xA)));
          int endX = std::min(w - 1, static_cast<int>(std::floor(xB)));
          if (startX > endX) continue;

          float startPX = static_cast<float>(startX) + 0.5f;
          float w0 = ((y1 - y2) * (startPX - x2) + (x2 - x1) * (curY - y2)) * invDenom;
          float w1 = ((y2 - y0) * (startPX - x2) + (x0 - x2) * (curY - y2)) * invDenom;

          float curInvZ = invZ2 + w0 * (invZ0 - invZ2) + w1 * (invZ1 - invZ2);
          float curR = c2.r + w0 * (c0.r - c2.r) + w1 * (c1.r - c2.r);
          float curG = c2.g + w0 * (c0.g - c2.g) + w1 * (c1.g - c2.g);
          float curB = c2.b + w0 * (c0.b - c2.b) + w1 * (c1.b - c2.b);

          int rowOffset = py * w;
          float *depthRow = &s_depthBuffer[rowOffset];
          uint32_t *colorRow = &s_colorBuffer[rowOffset];

          for (int px = startX; px <= endX; ++px) {
            if (curInvZ > depthRow[px]) {
              depthRow[px] = curInvZ;

              int r_int = static_cast<int>(curR * 255.0f);
              int g_int = static_cast<int>(curG * 255.0f);
              int b_int = static_cast<int>(curB * 255.0f);

              uint32_t r_byte = (r_int < 0) ? 0 : ((r_int > 255) ? 255 : r_int);
              uint32_t g_byte = (g_int < 0) ? 0 : ((g_int > 255) ? 255 : g_int);
              uint32_t b_byte = (b_int < 0) ? 0 : ((b_int > 255) ? 255 : b_int);

              colorRow[px] = 0xFF000000 | (r_byte << 16) | (g_byte << 8) | b_byte;
            }
            curInvZ += dInvZ_dx;
            curR += dR_dx;
            curG += dG_dx;
            curB += dB_dx;
          }
        }
      }
    }

    // 7. Blit Pixel-Accurate 3D Framebuffer to Screen
    SDL_FRect dstRect{0.0f, 0.0f, viewportWidth, viewportHeight};
    SDL_UpdateTexture(s_framebufferTexture, nullptr, s_colorBuffer.data(), w * sizeof(uint32_t));
    SDL_RenderTexture(sdlRenderer, s_framebufferTexture, nullptr, &dstRect);

    // 8. Radiant Glowing Halos for Coins via Additive GPU Blending
    if (env && env->glowEnabled && !glowHalos.empty()) {
      for (const auto &halo : glowHalos) {
        float r = halo.radius;
        SDL_FColor innerCol{std::clamp(halo.color.r * 0.9f, 0.0f, 1.0f),
                            std::clamp(halo.color.g * 0.9f, 0.0f, 1.0f),
                            std::clamp(halo.color.b * 0.9f, 0.0f, 1.0f), 0.75f};
        SDL_FColor outerCol{halo.color.r * 0.35f, halo.color.g * 0.35f, halo.color.b * 0.35f, 0.0f};

        SDL_Vertex haloVerts[5] = {
            {{halo.screenPos.x, halo.screenPos.y}, innerCol, {0.5f, 0.5f}},
            {{halo.screenPos.x - r, halo.screenPos.y - r}, outerCol, {0.0f, 0.0f}},
            {{halo.screenPos.x + r, halo.screenPos.y - r}, outerCol, {1.0f, 0.0f}},
            {{halo.screenPos.x + r, halo.screenPos.y + r}, outerCol, {1.0f, 1.0f}},
            {{halo.screenPos.x - r, halo.screenPos.y + r}, outerCol, {0.0f, 1.0f}}
        };
        int haloIndices[12] = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 4,
            0, 4, 1
        };
        SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_ADD);
        SDL_RenderGeometry(sdlRenderer, nullptr, haloVerts, 5, haloIndices, 12);
        SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
      }
    }
  }
};
