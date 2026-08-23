#pragma once

#include "nodes/2D/Light2D.hpp"
#include "renderers/Renderer2D.hpp"
#include "window.hpp"
#include <SDL3/SDL.h>
#include <algorithm>

// Global Directional 2D Light Node (inspired by Godot DirectionalLight2D)
class DirectionalLight2D : public Light2D {
public:
  float height = 0.0f;
  float maxDistance = 10000.0f;

  DirectionalLight2D() : Light2D("DirectionalLight2D") {}

  explicit DirectionalLight2D(Color lightColor, float lightEnergy = 1.0f)
      : Light2D("DirectionalLight2D") {
    color = lightColor;
    energy = lightEnergy;
  }

  void onDraw() override {
    if (!enabled || energy <= 0.0f) return;

    SDL_Renderer *sdlRenderer = Renderer2D::getRenderer();
    if (!sdlRenderer) return;


    SDL_BlendMode origBlend = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(sdlRenderer, &origBlend);

    if (blendMode == Light2DBlendMode::Add) {
      SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_ADD);
    }

    Vector2 vp = Window::getViewportSize();
    Color drawCol = color;
    drawCol.a = std::clamp(color.a * energy * 0.5f, 0.0f, 1.0f);
    Renderer2D::drawRectScreen(Vector2(0.0f, 0.0f), vp, drawCol, true);

    SDL_SetRenderDrawBlendMode(sdlRenderer, origBlend);
  }
};
