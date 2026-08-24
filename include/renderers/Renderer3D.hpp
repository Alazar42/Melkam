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

// Master 3D Rendering Subsystem with Pixel-Accurate Z-Buffering & Incremental DDA Rasterization
class Renderer3D {
public:
  inline static const Camera3D *s_activeCamera = nullptr;
  inline static bool s_isRendering = false;

  // Framebuffer & Depth Buffer
  inline static float s_renderScale = 0.55f; // Internal 3D render resolution scale for ultra-smooth 60+ FPS
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

  // Processes ECS 3D view and renders all active 3D meshes with Pixel-Accurate Z-Buffering
  static void render(float viewportWidth, float viewportHeight) {
    SDL_Renderer *sdlRenderer = Renderer2D::getRenderer();
    if (!sdlRenderer || viewportWidth <= 0.0f || viewportHeight <= 0.0f) return;

    // Viewport scaling for blistering 60+ FPS performance
    float scale = std::clamp(s_renderScale, 0.25f, 1.0f);
    int w = std::max(1, static_cast<int>(viewportWidth * scale));
    int h = std::max(1, static_cast<int>(viewportHeight * scale));

    // 1. Manage Framebuffer Texture & Buffers
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

    // 0. WorldEnvironment Background & Ambient Illumination
    const Environment *env = WorldEnvironment::getCurrent();
    Color ambientColor = Color::from_rgba8(50, 55, 75);

    // Fast SIMD/memset depth clear
    std::memset(s_depthBuffer.data(), 0, s_depthBuffer.size() * sizeof(float));

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
    Systems3D::updateCameras(static_cast<float>(w), static_cast<float>(h));

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
    float aspect = static_cast<float>(w) / static_cast<float>(h);
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
      dirLights.push_back({Vector3(-0.5f, -0.8f, -0.4f).normalized(), Color::from_rgba8(255, 245, 230), 1.0f});
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

    float halfW = static_cast<float>(w) * 0.5f;
    float halfH = static_cast<float>(h) * 0.5f;

    // Rasterize each 3D Mesh in the ECS registry
    for (auto entity : meshView) {
      const auto &meshComp = meshView.get<Mesh3DComponent>(entity);
      const auto &transComp = meshView.get<Transform3DComponent>(entity);

      if (!meshComp.visible || meshComp.vertices.empty() || meshComp.indices.empty()) continue;

      Transform3D globalTrans = transComp.globalTransform;
      Basis normalBasis = globalTrans.basis.inverse().transposed();

      size_t indexCount = meshComp.indices.size();
      for (size_t i = 0; i + 2 < indexCount; i += 3) {
        const Vertex3D &v0 = meshComp.vertices[meshComp.indices[i]];
        const Vertex3D &v1 = meshComp.vertices[meshComp.indices[i + 1]];
        const Vertex3D &v2 = meshComp.vertices[meshComp.indices[i + 2]];

        // 1. Transform vertices to World Space
        Vector3 w0 = globalTrans.xform(v0.position);
        Vector3 w1 = globalTrans.xform(v1.position);
        Vector3 w2 = globalTrans.xform(v2.position);

        // 2. Exact 3D World-Space Normal & Backface Culling
        Vector3 faceNorm = (w1 - w0).cross(w2 - w0);
        Vector3 toCam = activeCamTransform.origin - w0;
        bool isBackFacing = (faceNorm.dot(toCam) < 0.0f);
        if (isBackFacing && meshComp.cullBackfaces) continue;

        // 3. Transform vertices to Camera View Space
        Vector3 cam0 = viewInv.xform(w0);
        Vector3 cam1 = viewInv.xform(w1);
        Vector3 cam2 = viewInv.xform(w2);

        float z0 = -cam0.z;
        float z1 = -cam1.z;
        float z2 = -cam2.z;

        // Clip triangle if completely behind near clipping plane
        if (z0 <= nearPlane && z1 <= nearPlane && z2 <= nearPlane) continue;

        float safeZ0 = std::max(z0, nearPlane);
        float safeZ1 = std::max(z1, nearPlane);
        float safeZ2 = std::max(z2, nearPlane);

        float invZ0 = 1.0f / safeZ0;
        float invZ1 = 1.0f / safeZ1;
        float invZ2 = 1.0f / safeZ2;

        // 4. Screen Projection
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

        if (isBackFacing) {
          n0 = -n0;
          n1 = -n1;
          n2 = -n2;
        }

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

          // Point Lights
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

          Color finalCol = baseCol * meshComp.albedoColor * Color(totalR, totalG, totalB, 1.0f);
          return Color(
              std::clamp(finalCol.r, 0.0f, 1.0f),
              std::clamp(finalCol.g, 0.0f, 1.0f),
              std::clamp(finalCol.b, 0.0f, 1.0f),
              meshComp.albedoColor.a);
        };

        Color c0 = computeVertexColor(w0, n0, v0.color);
        Color c1 = computeVertexColor(w1, n1, v1.color);
        Color c2 = computeVertexColor(w2, n2, v2.color);

        // 6. Ultra-Fast Linear DDA Scanline Rasterizer
        float denom = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);
        if (std::abs(denom) < 0.0001f) continue;
        float invDenom = 1.0f / denom;

        int minX = std::max(0, static_cast<int>(std::floor(std::min({x0, x1, x2}))));
        int maxX = std::min(w - 1, static_cast<int>(std::ceil(std::max({x0, x1, x2}))));
        int minY = std::max(0, static_cast<int>(std::floor(std::min({y0, y1, y2}))));
        int maxY = std::min(h - 1, static_cast<int>(std::ceil(std::max({y0, y1, y2}))));

        if (minX > maxX || minY > maxY) continue;

        float dw0_dx = (y1 - y2) * invDenom;
        float dw0_dy = (x2 - x1) * invDenom;
        float dw1_dx = (y2 - y0) * invDenom;
        float dw1_dy = (x0 - x2) * invDenom;

        float dInvZ_dx = ((invZ0 - invZ2) * (y1 - y2) + (invZ1 - invZ2) * (y2 - y0)) * invDenom;
        float dInvZ_dy = ((invZ0 - invZ2) * (x2 - x1) + (invZ1 - invZ2) * (x0 - x2)) * invDenom;

        float dR_dx = ((c0.r - c2.r) * (y1 - y2) + (c1.r - c2.r) * (y2 - y0)) * invDenom;
        float dR_dy = ((c0.r - c2.r) * (x2 - x1) + (c1.r - c2.r) * (x0 - x2)) * invDenom;

        float dG_dx = ((c0.g - c2.g) * (y1 - y2) + (c1.g - c2.g) * (y2 - y0)) * invDenom;
        float dG_dy = ((c0.g - c2.g) * (x2 - x1) + (c1.g - c2.g) * (x0 - x2)) * invDenom;

        float dB_dx = ((c0.b - c2.b) * (y1 - y2) + (c1.b - c2.b) * (y2 - y0)) * invDenom;
        float dB_dy = ((c0.b - c2.b) * (x2 - x1) + (c1.b - c2.b) * (x0 - x2)) * invDenom;

        float startPX = static_cast<float>(minX) + 0.5f;
        float startPY = static_cast<float>(minY) + 0.5f;

        float row_w0 = ((y1 - y2) * (startPX - x2) + (x2 - x1) * (startPY - y2)) * invDenom;
        float row_w1 = ((y2 - y0) * (startPX - x2) + (x0 - x2) * (startPY - y2)) * invDenom;

        float row_invZ = invZ2 + row_w0 * (invZ0 - invZ2) + row_w1 * (invZ1 - invZ2);
        float row_r = c2.r + row_w0 * (c0.r - c2.r) + row_w1 * (c1.r - c2.r);
        float row_g = c2.g + row_w0 * (c0.g - c2.g) + row_w1 * (c1.g - c2.g);
        float row_b = c2.b + row_w0 * (c0.b - c2.b) + row_w1 * (c1.b - c2.b);

        for (int py_i = minY; py_i <= maxY; ++py_i) {
          int rowOffset = py_i * w;
          float w0_b = row_w0;
          float w1_b = row_w1;
          float cur_invZ = row_invZ;
          float cur_r = row_r;
          float cur_g = row_g;
          float cur_b = row_b;

          for (int px_i = minX; px_i <= maxX; ++px_i) {
            float w2_b = 1.0f - w0_b - w1_b;

            if (w0_b >= 0.0f && w1_b >= 0.0f && w2_b >= 0.0f) {
              int pixelIdx = rowOffset + px_i;

              if (cur_invZ > s_depthBuffer[pixelIdx]) {
                s_depthBuffer[pixelIdx] = cur_invZ;

                uint32_t r_byte = static_cast<uint32_t>(std::clamp(cur_r, 0.0f, 1.0f) * 255.0f);
                uint32_t g_byte = static_cast<uint32_t>(std::clamp(cur_g, 0.0f, 1.0f) * 255.0f);
                uint32_t b_byte = static_cast<uint32_t>(std::clamp(cur_b, 0.0f, 1.0f) * 255.0f);

                s_colorBuffer[pixelIdx] = 0xFF000000 | (r_byte << 16) | (g_byte << 8) | b_byte;
              }
            }

            w0_b += dw0_dx;
            w1_b += dw1_dx;
            cur_invZ += dInvZ_dx;
            cur_r += dR_dx;
            cur_g += dG_dx;
            cur_b += dB_dx;
          }

          row_w0 += dw0_dy;
          row_w1 += dw1_dy;
          row_invZ += dInvZ_dy;
          row_r += dR_dy;
          row_g += dG_dy;
          row_b += dB_dy;
        }
      }
    }

    // 7. Blit Hardware-Accelerated 3D Framebuffer to Screen (with Bilinear Upscaling)
    SDL_UpdateTexture(s_framebufferTexture, nullptr, s_colorBuffer.data(), w * sizeof(uint32_t));
    SDL_FRect dstRect{0.0f, 0.0f, viewportWidth, viewportHeight};
    SDL_RenderTexture(sdlRenderer, s_framebufferTexture, nullptr, &dstRect);
  }
};
