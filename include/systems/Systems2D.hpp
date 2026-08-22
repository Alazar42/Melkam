#pragma once

#include "ECS.hpp"
#include "components/Components2D.hpp"
#include "renderers/Renderer2D.hpp"

// General 2D ECS Simulation and Rendering Systems.
class Systems2D {
public:
  // Integrates Velocity2D linear and angular velocities into Transform2D.
  static void updateMovement(float dt) {
    Entity::each<Transform2D, Velocity2D>(
        [dt](Entity entity, Transform2D &transform, const Velocity2D &velocity) {
          (void)entity;
          transform.position += velocity.linear * dt;
          transform.rotation += velocity.angular * dt;
        });
  }

  // Renders all active ECS Shape2D and Sprite2D components to Renderer2D.
  static void render() {
    // 1. Render all Shape2D entities
    Entity::each<Transform2D, Shape2D>(
        [](Entity entity, const Transform2D &transform, const Shape2D &shape) {
          (void)entity;
          shape.draw(transform.position, transform.rotation, transform.scale);
        });

    // 2. Render all Sprite2D entities
    Entity::each<Transform2D, Sprite2D>(
        [](Entity entity, const Transform2D &transform, const Sprite2D &sprite) {
          (void)entity;
          sprite.draw(transform.position, transform.rotation, transform.scale);
        });
  }
};
