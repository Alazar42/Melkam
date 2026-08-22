#pragma once

#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include "time.hpp"
#include "window.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

// Configuration for Godot-style Engine Boot Splash Screen.
struct BootSplashConfig {
  bool enabled = true;
  std::string imagePath = "logo.png";
  Color backgroundColor = Color::from_rgba8(0, 0, 0); // Black background
  float minDuration = 1.0f;      // Full visibility hold time (seconds)
  float fadeInDuration = 0.35f;  // Smooth fade-in time (seconds)
  float fadeOutDuration = 0.35f; // Smooth fade-out time (seconds)
  bool allowSkip = true;         // Allow skipping via click or keypress
  bool fullsize = false;         // Centered logo with black background (no fullsize stretch)
};

// Engine Boot Splash Manager (inspired by Godot's BootSplash system).
class BootSplash {
public:
  // Resolves file path across common working directories (e.g. root, build/, build/Debug/)
  static std::string resolvePath(const std::string &path) {
    if (std::filesystem::exists(path)) return path;
    if (std::filesystem::exists("../" + path)) return "../" + path;
    if (std::filesystem::exists("../../" + path)) return "../../" + path;
    return path;
  }

  // Executes the boot splash sequence on the active Window and Renderer.
  static void run(Window &window, const BootSplashConfig &config = BootSplashConfig{}) {
    if (!config.enabled) return;

    std::string resolved = resolvePath(config.imagePath);
    if (!std::filesystem::exists(resolved)) {
      if (std::filesystem::exists(resolvePath("logo.svg"))) resolved = resolvePath("logo.svg");
      else if (std::filesystem::exists(resolvePath("MelkamLogo.png"))) resolved = resolvePath("MelkamLogo.png");
      else if (std::filesystem::exists(resolvePath("logo.png"))) resolved = resolvePath("logo.png");
      else return; // Logo not found, proceed directly to scene
    }

    auto logoTex = std::make_shared<Texture2D>(resolved, window.getRenderer());
    if (!logoTex || !logoTex->isValid()) {
      return;
    }

    // 1. Temporarily disable logical presentation so splash renders to full physical window pixels
    SDL_Renderer *sdlRenderer = window.getRenderer();
    if (sdlRenderer) {
      SDL_SetRenderLogicalPresentation(sdlRenderer, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED);
    }

    float totalTime = config.fadeInDuration + config.minDuration + config.fadeOutDuration;
    float elapsed = 0.0f;
    bool skipped = false;

    // Reset time delta
    Time::update();

    while (window.isOpen() && elapsed < totalTime && !skipped) {
      Time::update();
      float dt = Time::getDeltaTime();
      elapsed += dt;

      // Poll window events to allow window dragging, closing, or skipping
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
          window.close();
          return;
        }
        if (config.allowSkip) {
          if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            skipped = true;
          }
        }
      }

      // Calculate smooth alpha curve (Fade In -> Hold -> Fade Out)
      float alpha = 1.0f;
      if (elapsed < config.fadeInDuration) {
        float t = elapsed / std::max(0.01f, config.fadeInDuration);
        alpha = std::sin(t * 1.5707963f); // Ease out sine
      } else if (elapsed > (config.fadeInDuration + config.minDuration)) {
        float fadeOutElapsed = elapsed - (config.fadeInDuration + config.minDuration);
        float t = std::clamp(fadeOutElapsed / std::max(0.01f, config.fadeOutDuration), 0.0f, 1.0f);
        alpha = std::cos(t * 1.5707963f); // Ease in sine
      }

      alpha = std::clamp(alpha, 0.0f, 1.0f);

      // Query current physical window size
      int physW = 0, physH = 0;
      SDL_GetWindowSize(window.getNativeWindow(), &physW, &physH);
      if (physW <= 0) physW = static_cast<int>(window.getWidth());
      if (physH <= 0) physH = static_cast<int>(window.getHeight());

      // Render physical background
      SDL_SetRenderDrawColor(sdlRenderer,
                             static_cast<uint8_t>(config.backgroundColor.r * 255.0f),
                             static_cast<uint8_t>(config.backgroundColor.g * 255.0f),
                             static_cast<uint8_t>(config.backgroundColor.b * 255.0f),
                             255);
      SDL_RenderClear(sdlRenderer);

      // Setup destination rect for full physical window
      SDL_FRect dstRect{0.0f, 0.0f, static_cast<float>(physW), static_cast<float>(physH)};

      if (!config.fullsize) {
        // Centered native size
        Vector2 imgSize = logoTex->getSize();
        dstRect.x = (static_cast<float>(physW) - imgSize.x) * 0.5f;
        dstRect.y = (static_cast<float>(physH) - imgSize.y) * 0.5f;
        dstRect.w = imgSize.x;
        dstRect.h = imgSize.y;
      }

      // Apply fade alpha and render logo
      SDL_Texture *nativeTex = logoTex->getNativeTexture();
      SDL_SetTextureColorModFloat(nativeTex, 1.0f, 1.0f, 1.0f);
      SDL_SetTextureAlphaModFloat(nativeTex, alpha);
      SDL_RenderTexture(sdlRenderer, nativeTex, nullptr, &dstRect);

      window.present();

      // Cap framerate during splash
      SDL_Delay(1);
    }

    // 2. Restore engine logical presentation and stretch mode for gameplay
    window.applyLogicalPresentation();
  }
};
