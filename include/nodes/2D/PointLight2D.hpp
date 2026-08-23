#pragma once

#include "nodes/2D/Light2D.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include <SDL3/SDL.h>
#include <cmath>
#include <vector>

// Radial/Omni 2D Light Node (inspired by Godot PointLight2D)
class PointLight2D : public Light2D {
public:
  float radius = 180.0f;
  float attenuation = 1.0f;
  Ref<Texture2D> texture = nullptr;
  float textureScale = 1.0f;

  PointLight2D() : Light2D("PointLight2D") {}

  explicit PointLight2D(float lightRadius, Color lightColor = Color::WHITE, float lightEnergy = 1.0f)
      : Light2D("PointLight2D"), radius(lightRadius) {
    color = lightColor;
    energy = lightEnergy;
  }

  void onDraw() override {
    if (!enabled || energy <= 0.0f) return;

    Transform2D globalTransform = getGlobalTransform();
    Vector2 worldPos = globalTransform.position;
    float currentRadius = radius * globalTransform.scale.x * textureScale;
    if (currentRadius <= 0.0f) return;

    SDL_Renderer *sdlRenderer = Renderer2D::getRenderer();
    if (!sdlRenderer) return;


    // Set Additive / Blend mode
    SDL_BlendMode origBlend = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(sdlRenderer, &origBlend);

    if (blendMode == Light2DBlendMode::Add) {
      SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_ADD);
    }

    if (texture && texture->isValid()) {
      Vector2 size(currentRadius * 2.0f, currentRadius * 2.0f);
      Vector2 pos = worldPos - size * 0.5f;
      Color tint = color;
      tint.a = std::clamp(color.a * energy, 0.0f, 1.0f);
      Renderer2D::drawTexture(*texture, pos, size, tint, globalTransform.rotation);
    } else {
      // Procedural radial light falloff using concentric gradient rings
      Vector2 screenCenter = Renderer2D::toScreen(worldPos);
      const Camera2D *cam = Camera2D::getCurrent();
      float zoom = cam ? cam->zoom : 1.0f;
      float screenRadius = currentRadius * zoom;

      int segments = 24;
      int rings = 8;
      float stepAngle = (2.0f * 3.1415926535f) / static_cast<float>(segments);

      std::vector<SDL_Vertex> vertices;
      std::vector<int> indices;

      // Center vertex with energy scaling
      Color centerCol = color * energy;
      centerCol.a = std::clamp(color.a * energy, 0.0f, 1.0f);
      SDL_FColor sdlCenter = Renderer2D::toSDLColor(centerCol);
      vertices.push_back({{screenCenter.x, screenCenter.y}, sdlCenter, {0.5f, 0.5f}});

      for (int r = 1; r <= rings; ++r) {
        float frac = static_cast<float>(r) / static_cast<float>(rings);
        float rRad = screenRadius * frac;
        float falloff = std::pow(1.0f - frac, attenuation);
        Color ringCol = color * (energy * falloff);
        ringCol.a = std::clamp(color.a * energy * falloff, 0.0f, 1.0f);
        SDL_FColor sdlRing = Renderer2D::toSDLColor(ringCol);


        int ringStartIdx = static_cast<int>(vertices.size());

        for (int s = 0; s < segments; ++s) {
          float ang = static_cast<float>(s) * stepAngle;
          float x = screenCenter.x + std::cos(ang) * rRad;
          float y = screenCenter.y + std::sin(ang) * rRad;
          vertices.push_back({{x, y}, sdlRing, {0.0f, 0.0f}});

          if (r == 1) {
            // First ring connected to center
            int nextS = (s + 1) % segments;
            indices.push_back(0);
            indices.push_back(ringStartIdx + s);
            indices.push_back(ringStartIdx + nextS);
          } else {
            // Outer rings connected to previous ring
            int prevRingStart = ringStartIdx - segments;
            int nextS = (s + 1) % segments;

            indices.push_back(prevRingStart + s);
            indices.push_back(ringStartIdx + s);
            indices.push_back(ringStartIdx + nextS);

            indices.push_back(prevRingStart + s);
            indices.push_back(ringStartIdx + nextS);
            indices.push_back(prevRingStart + nextS);
          }
        }
      }

      SDL_RenderGeometry(sdlRenderer, nullptr, vertices.data(),
                         static_cast<int>(vertices.size()), indices.data(),
                         static_cast<int>(indices.size()));
    }

    // Restore blend mode
    SDL_SetRenderDrawBlendMode(sdlRenderer, origBlend);
  }
};
