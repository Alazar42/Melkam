#pragma once

#include "components/Components3D.hpp"
#include "nodes/3D/Camera3D.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/vulkan/RenderingDevice3D.hpp"
#include "systems/Systems3D.hpp"
#include "window.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// Master 3D Rendering Subsystem with Hardware-Accelerated Projection, Shading & Depth Sorting
class Renderer3D {
public:
  inline static const Camera3D *s_activeCamera = nullptr;
  inline static bool s_isRendering = false;

  struct RenderTriangle3D {
    SDL_Vertex vertices[3];
    float avgDepth = 0.0f;
  };

  struct MeshBatch3D {
    float sortDistance = 0.0f;
    std::vector<RenderTriangle3D> triangles;
  };

  static void init(Window &window) {
    std::cout << "[MelkamEngine::Renderer3D] Initializing 3D Hardware Subsystem..." << std::endl;
    RenderingDevice3D::get().init(window.getNativeWindow());
  }

  static void shutdown() {
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

  // Processes ECS 3D view and renders all active visible 3D mesh instances
  static void render(float viewportWidth, float viewportHeight) {
    SDL_Renderer *sdlRenderer = Renderer2D::getRenderer();
    if (!sdlRenderer || viewportWidth <= 0.0f || viewportHeight <= 0.0f) return;

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

    // Camera parameters
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
      // Default ambient directional sun
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

    Color ambientColor = Color::from_rgba8(50, 55, 75);

    std::vector<MeshBatch3D> batches;

    // Process each 3D Mesh in the ECS registry
    for (auto entity : meshView) {
      const auto &meshComp = meshView.get<Mesh3DComponent>(entity);
      const auto &transComp = meshView.get<Transform3DComponent>(entity);

      if (!meshComp.visible || meshComp.vertices.empty() || meshComp.indices.empty()) continue;

      Transform3D globalTrans = transComp.globalTransform;
      Basis normalBasis = globalTrans.basis.inverse().transposed();

      MeshBatch3D batch{};
      // Calculate object distance to camera for inter-object sorting
      float distToCam = (globalTrans.origin - activeCamTransform.origin).length();
      // Large floor / ground planes are classified as background layer
      if (meshComp.aabb.size.x >= 10.0f || globalTrans.origin.y <= 0.05f) {
        batch.sortDistance = distToCam + 1000.0f;
      } else {
        batch.sortDistance = distToCam;
      }

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
        if (isBackFacing && meshComp.cullBackfaces) continue; // Cull back-facing triangles on solid geometry

        // 3. Transform vertices to Camera View Space
        Vector3 cam0 = viewInv.xform(w0);
        Vector3 cam1 = viewInv.xform(w1);
        Vector3 cam2 = viewInv.xform(w2);

        float z0 = -cam0.z;
        float z1 = -cam1.z;
        float z2 = -cam2.z;

        // Skip triangle if completely behind near clipping plane
        if (z0 <= nearPlane && z1 <= nearPlane && z2 <= nearPlane) continue;

        // Clamp depth to nearPlane to avoid division by zero or negative depth
        float safeZ0 = std::max(z0, nearPlane);
        float safeZ1 = std::max(z1, nearPlane);
        float safeZ2 = std::max(z2, nearPlane);

        // 4. Screen Projection
        Vector2 s0(
            (cam0.x * f / aspect) / safeZ0 * (viewportWidth * 0.5f) + (viewportWidth * 0.5f),
            (1.0f - (cam0.y * f) / safeZ0) * (viewportHeight * 0.5f));

        Vector2 s1(
            (cam1.x * f / aspect) / safeZ1 * (viewportWidth * 0.5f) + (viewportWidth * 0.5f),
            (1.0f - (cam1.y * f) / safeZ1) * (viewportHeight * 0.5f));

        Vector2 s2(
            (cam2.x * f / aspect) / safeZ2 * (viewportWidth * 0.5f) + (viewportWidth * 0.5f),
            (1.0f - (cam2.y * f) / safeZ2) * (viewportHeight * 0.5f));

        // 5. Lighting Calculations
        Vector3 n0 = normalBasis.xform(v0.normal).normalized();
        Vector3 n1 = normalBasis.xform(v1.normal).normalized();
        Vector3 n2 = normalBasis.xform(v2.normal).normalized();

        if (isBackFacing) {
          n0 = -n0;
          n1 = -n1;
          n2 = -n2;
        }

        auto computeVertexColor = [&](const Vector3 &pos, const Vector3 &norm, const Color &baseCol) -> SDL_FColor {
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
          return SDL_FColor{
              std::clamp(finalCol.r, 0.0f, 1.0f),
              std::clamp(finalCol.g, 0.0f, 1.0f),
              std::clamp(finalCol.b, 0.0f, 1.0f),
              meshComp.albedoColor.a};
        };

        SDL_FColor c0 = computeVertexColor(w0, n0, v0.color);
        SDL_FColor c1 = computeVertexColor(w1, n1, v1.color);
        SDL_FColor c2 = computeVertexColor(w2, n2, v2.color);

        RenderTriangle3D tri{};
        tri.vertices[0] = SDL_Vertex{{s0.x, s0.y}, c0, {v0.uv.x, v0.uv.y}};
        tri.vertices[1] = SDL_Vertex{{s1.x, s1.y}, c1, {v1.uv.x, v1.uv.y}};
        tri.vertices[2] = SDL_Vertex{{s2.x, s2.y}, c2, {v2.uv.x, v2.uv.y}};
        tri.avgDepth = (safeZ0 + safeZ1 + safeZ2) * 0.3333333f;

        batch.triangles.push_back(tri);
      }

      if (!batch.triangles.empty()) {
        // Sort triangles within the mesh
        std::sort(batch.triangles.begin(), batch.triangles.end(), [](const RenderTriangle3D &a, const RenderTriangle3D &b) {
          return a.avgDepth > b.avgDepth;
        });
        batches.push_back(std::move(batch));
      }
    }

    if (batches.empty()) return;

    // Sort batches by distance (farthest background objects first, then foreground models)
    std::sort(batches.begin(), batches.end(), [](const MeshBatch3D &a, const MeshBatch3D &b) {
      return a.sortDistance > b.sortDistance;
    });

    // Hardware Batch Draw Submission via SDL_RenderGeometry
    std::vector<SDL_Vertex> batchVertices;
    std::vector<int> batchIndices;

    for (const auto &batch : batches) {
      batchVertices.clear();
      batchIndices.clear();
      batchVertices.reserve(batch.triangles.size() * 3);
      batchIndices.reserve(batch.triangles.size() * 3);

      for (size_t t = 0; t < batch.triangles.size(); ++t) {
        int baseIdx = static_cast<int>(batchVertices.size());
        batchVertices.push_back(batch.triangles[t].vertices[0]);
        batchVertices.push_back(batch.triangles[t].vertices[1]);
        batchVertices.push_back(batch.triangles[t].vertices[2]);

        batchIndices.push_back(baseIdx + 0);
        batchIndices.push_back(baseIdx + 1);
        batchIndices.push_back(baseIdx + 2);
      }

      if (!batchVertices.empty()) {
        SDL_RenderGeometry(sdlRenderer, nullptr,
                           batchVertices.data(), static_cast<int>(batchVertices.size()),
                           batchIndices.data(), static_cast<int>(batchIndices.size()));
      }
    }
  }
};
