#pragma once

#include "core/Memory.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

enum class ParticleEmissionShape {
  Point,
  Sphere,
  Box,
  Ring
};

struct Particle2D {
  Vector2 position{0.0f, 0.0f};
  Vector2 velocity{0.0f, 0.0f};
  float rotation = 0.0f;
  float angularVelocity = 0.0f;
  float age = 0.0f;
  float lifetime = 1.0f;
  float initialScale = 1.0f;
  Color initialColor = Color::WHITE;
  bool active = false;
};

// 2D CPU Particle Emitter System (inspired by Godot CPUParticles2D)
class CPUParticles2D : public Node2D {
public:
  bool emitting = true;
  int amount = 48;
  float lifetime = 1.5f;
  bool oneShot = false;
  float speedScale = 1.0f;
  float explosiveness = 0.0f;

  ParticleEmissionShape emissionShape = ParticleEmissionShape::Point;
  Vector2 emissionBoxExtents{20.0f, 20.0f};
  float emissionSphereRadius = 20.0f;
  float emissionRingInnerRadius = 15.0f;
  float emissionRingOuterRadius = 30.0f;

  Vector2 direction{0.0f, -1.0f};
  float spreadDegrees = 45.0f;
  float initialVelocityMin = 80.0f;
  float initialVelocityMax = 160.0f;
  float angularVelocityMin = -3.14f;
  float angularVelocityMax = 3.14f;
  Vector2 gravity{0.0f, 200.0f};
  float radialAccel = 0.0f;
  float damping = 0.0f;

  float scaleMin = 4.0f;
  float scaleMax = 8.0f;
  float scaleEnd = 0.0f;
  Color color = Color::WHITE;
  Color colorEnd = Color::from_rgba8(255, 255, 255, 0);
  Ref<Texture2D> texture = nullptr;

  CPUParticles2D() : Node2D("CPUParticles2D") {
    m_particles.resize(amount);
  }

  explicit CPUParticles2D(int particleAmount) : Node2D("CPUParticles2D"), amount(particleAmount) {
    m_particles.resize(amount);
  }

  void restart() {
    for (auto &p : m_particles) {
      p.active = false;
    }
    m_spawnTimer = 0.0f;
    m_emittedCount = 0;
  }

  void onProcess(float delta) override {
    float dt = delta * speedScale;
    if (dt <= 0.0f) return;

    if (m_particles.size() != static_cast<size_t>(amount)) {
      m_particles.resize(amount);
    }

    // 1. Spawning
    if (emitting) {
      float spawnInterval = lifetime / static_cast<float>(amount);
      m_spawnTimer += dt;

      if (explosiveness > 0.0f && m_emittedCount == 0) {
        int burstCount = static_cast<int>(amount * explosiveness);
        for (int i = 0; i < burstCount; ++i) {
          spawnParticle();
        }
      }

      while (m_spawnTimer >= spawnInterval) {
        m_spawnTimer -= spawnInterval;
        if (!oneShot || m_emittedCount < amount) {
          spawnParticle();
        }
      }
    }

    // 2. Physics & Lifetime Updates
    Transform2D globalTransform = getGlobalTransform();
    Vector2 emitterPos = globalTransform.position;

    for (auto &p : m_particles) {
      if (!p.active) continue;

      p.age += dt;
      if (p.age >= p.lifetime) {
        p.active = false;
        continue;
      }

      // Gravity & Acceleration
      p.velocity += gravity * dt;

      if (radialAccel != 0.0f) {
        Vector2 diff = p.position - emitterPos;
        if (!diff.is_zero_approx()) {
          p.velocity += diff.normalized() * (radialAccel * dt);
        }
      }

      if (damping > 0.0f) {
        p.velocity = p.velocity * std::max(0.0f, 1.0f - damping * dt);
      }

      p.position += p.velocity * dt;
      p.rotation += p.angularVelocity * dt;
    }
  }

  void onDraw() override {
    for (const auto &p : m_particles) {
      if (!p.active) continue;

      float t = std::clamp(p.age / p.lifetime, 0.0f, 1.0f);
      float currentScale = p.initialScale * (1.0f - t) + scaleEnd * t;
      Color currentColor = p.initialColor.lerp(colorEnd, t);

      if (texture && texture->isValid()) {
        Vector2 size(currentScale, currentScale);
        Vector2 drawPos = p.position - size * 0.5f;
        Renderer2D::drawTexture(*texture, drawPos, size, currentColor, p.rotation);
      } else {
        // Draw colored circle or rotated square
        Renderer2D::drawCircle(p.position, currentScale * 0.5f, currentColor, true, 12);
      }
    }
  }

private:
  void spawnParticle() {
    for (auto &p : m_particles) {
      if (!p.active) {
        Transform2D globalTransform = getGlobalTransform();
        p.active = true;
        p.age = 0.0f;
        p.lifetime = lifetime * (0.8f + 0.4f * getRandomFloat());

        // Spawn position based on emissionShape
        Vector2 offset{0.0f, 0.0f};
        switch (emissionShape) {
        case ParticleEmissionShape::Point:
          offset = {0.0f, 0.0f};
          break;
        case ParticleEmissionShape::Box:
          offset = {getRandomFloatRange(-emissionBoxExtents.x, emissionBoxExtents.x),
                    getRandomFloatRange(-emissionBoxExtents.y, emissionBoxExtents.y)};
          break;
        case ParticleEmissionShape::Sphere: {
          float ang = getRandomFloatRange(0.0f, 6.2831853f);
          float rad = emissionSphereRadius * std::sqrt(getRandomFloat());
          offset = {rad * std::cos(ang), rad * std::sin(ang)};
          break;
        }
        case ParticleEmissionShape::Ring: {
          float ang = getRandomFloatRange(0.0f, 6.2831853f);
          float rad = getRandomFloatRange(emissionRingInnerRadius, emissionRingOuterRadius);
          offset = {rad * std::cos(ang), rad * std::sin(ang)};
          break;
        }
        }

        p.position = globalTransform.transformPoint(offset);

        // Direction & Spread
        float baseAngle = std::atan2(direction.y, direction.x) + globalTransform.rotation;
        float spreadRad = spreadDegrees * (3.14159265f / 180.0f);
        float randAngle = baseAngle + getRandomFloatRange(-spreadRad * 0.5f, spreadRad * 0.5f);
        float speed = getRandomFloatRange(initialVelocityMin, initialVelocityMax);

        p.velocity = {speed * std::cos(randAngle), speed * std::sin(randAngle)};
        p.rotation = getRandomFloatRange(0.0f, 6.2831853f);
        p.angularVelocity = getRandomFloatRange(angularVelocityMin, angularVelocityMax);
        p.initialScale = getRandomFloatRange(scaleMin, scaleMax);
        p.initialColor = color;

        m_emittedCount++;
        break;
      }
    }
  }

  static float getRandomFloat() {
    static std::mt19937 gen(1337);
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    return dis(gen);
  }

  static float getRandomFloatRange(float minVal, float maxVal) {
    return minVal + (maxVal - minVal) * getRandomFloat();
  }

  std::vector<Particle2D> m_particles;
  float m_spawnTimer = 0.0f;
  int m_emittedCount = 0;
};
