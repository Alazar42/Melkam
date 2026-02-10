#include <Melkam/scene/Systems2D.hpp>

#include <Melkam/scene/Components.hpp>
#include <Melkam/scene/Entity.hpp>
#include <Melkam/scene/Scene.hpp>
#include <Melkam/scene/System.hpp>
#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace Melkam
{
    namespace
    {
        struct Aabb2D
        {
            float minX;
            float minY;
            float maxX;
            float maxY;
        };

        Aabb2D makeAabb(const TransformComponent &transform, const BoxShape2DComponent &shape)
        {
            const float halfX = shape.size[0] * 0.5f;
            const float halfY = shape.size[1] * 0.5f;
            const float centerX = transform.position.x;
            const float centerY = transform.position.y;
            return {centerX - halfX, centerY - halfY, centerX + halfX, centerY + halfY};
        }

        bool intersects(const Aabb2D &a, const Aabb2D &b)
        {
            return a.minX < b.maxX && a.maxX > b.minX && a.minY < b.maxY && a.maxY > b.minY;
        }

        class PlayerInputSystem : public System
        {
        public:
            void onUpdate(Scene &scene, float dt) override
            {
                (void)dt;
                for (auto &entity : scene.view<Input2DComponent>())
                {
                    auto *input = entity.tryGetComponent<Input2DComponent>();
                    if (!input)
                    {
                        continue;
                    }

                    float x = 0.0f;
                    float y = 0.0f;
                    if (IsKeyDown(KEY_A))
                    {
                        x -= 1.0f;
                    }
                    if (IsKeyDown(KEY_D))
                    {
                        x += 1.0f;
                    }
                    if (IsKeyDown(KEY_W))
                    {
                        y -= 1.0f;
                    }
                    if (IsKeyDown(KEY_S))
                    {
                        y += 1.0f;
                    }

                    const float length = std::sqrt(x * x + y * y);
                    if (length > 0.001f)
                    {
                        x /= length;
                        y /= length;
                    }

                    input->direction[0] = x;
                    input->direction[1] = y;
                }
            }
        };

        class Physics2DSystem : public System
        {
        public:
            void onUpdate(Scene &scene, float dt) override
            {
                m_accumulator += dt;
                const float fixedDt = 1.0f / 120.0f;
                const int maxSteps = 5;

                int steps = 0;
                while (m_accumulator >= fixedDt && steps < maxSteps)
                {
                    step(scene, fixedDt);
                    m_accumulator -= fixedDt;
                    ++steps;
                }
            }

        private:
            void step(Scene &scene, float dt)
            {
                auto staticBodies = scene.view<TransformComponent, BoxShape2DComponent, StaticBodyComponent>();

                for (auto &entity : scene.view<TransformComponent, BoxShape2DComponent, Velocity2DComponent>())
                {
                    auto *transform = entity.tryGetComponent<TransformComponent>();
                    auto *shape = entity.tryGetComponent<BoxShape2DComponent>();
                    auto *velocity = entity.tryGetComponent<Velocity2DComponent>();
                    auto *controller = entity.tryGetComponent<CharacterController2DComponent>();
                    auto *input = entity.tryGetComponent<Input2DComponent>();
                    auto *layers = entity.tryGetComponent<CollisionLayerComponent>();
                    if (!transform || !shape || !velocity)
                    {
                        continue;
                    }

                    if (controller && input)
                    {
                        const float targetX = input->direction[0] * controller->maxSpeed;
                        const float targetY = input->direction[1] * controller->maxSpeed;

                        const float accel = std::max(controller->acceleration, 0.0f);
                        velocity->velocity[0] += (targetX - velocity->velocity[0]) * std::min(1.0f, accel * dt);
                        velocity->velocity[1] += (targetY - velocity->velocity[1]) * std::min(1.0f, accel * dt);

                        const float damping = std::max(controller->damping, 0.0f);
                        const float dampFactor = 1.0f / (1.0f + damping * dt);
                        velocity->velocity[0] *= dampFactor;
                        velocity->velocity[1] *= dampFactor;
                    }

                    const std::uint32_t moverLayer = layers ? layers->layer : 1u;
                    const std::uint32_t moverMask = layers ? layers->mask : 0xFFFFFFFFu;

                    const float dx = velocity->velocity[0] * dt;
                    const float dy = velocity->velocity[1] * dt;

                    transform->position.x += dx;
                    Aabb2D moverX = makeAabb(*transform, *shape);
                    for (auto &wall : staticBodies)
                    {
                        auto *wallTransform = wall.tryGetComponent<TransformComponent>();
                        auto *wallShape = wall.tryGetComponent<BoxShape2DComponent>();
                        auto *wallLayer = wall.tryGetComponent<CollisionLayerComponent>();
                        if (!wallTransform || !wallShape)
                        {
                            continue;
                        }

                        const std::uint32_t wallBits = wallLayer ? wallLayer->layer : 1u;
                        const std::uint32_t wallMask = wallLayer ? wallLayer->mask : 0xFFFFFFFFu;
                        if ((moverMask & wallBits) == 0u || (wallMask & moverLayer) == 0u)
                        {
                            continue;
                        }

                        const Aabb2D obstacle = makeAabb(*wallTransform, *wallShape);
                        if (!intersects(moverX, obstacle))
                        {
                            continue;
                        }

                        const float overlapX1 = obstacle.maxX - moverX.minX;
                        const float overlapX2 = moverX.maxX - obstacle.minX;
                        const float resolveX = (overlapX1 < overlapX2) ? -overlapX1 : overlapX2;
                        transform->position.x += resolveX;
                        velocity->velocity[0] = 0.0f;
                        moverX = makeAabb(*transform, *shape);
                    }

                    transform->position.y += dy;
                    Aabb2D moverY = makeAabb(*transform, *shape);
                    for (auto &wall : staticBodies)
                    {
                        auto *wallTransform = wall.tryGetComponent<TransformComponent>();
                        auto *wallShape = wall.tryGetComponent<BoxShape2DComponent>();
                        auto *wallLayer = wall.tryGetComponent<CollisionLayerComponent>();
                        if (!wallTransform || !wallShape)
                        {
                            continue;
                        }

                        const std::uint32_t wallBits = wallLayer ? wallLayer->layer : 1u;
                        const std::uint32_t wallMask = wallLayer ? wallLayer->mask : 0xFFFFFFFFu;
                        if ((moverMask & wallBits) == 0u || (wallMask & moverLayer) == 0u)
                        {
                            continue;
                        }

                        const Aabb2D obstacle = makeAabb(*wallTransform, *wallShape);
                        if (!intersects(moverY, obstacle))
                        {
                            continue;
                        }

                        const float overlapY1 = obstacle.maxY - moverY.minY;
                        const float overlapY2 = moverY.maxY - obstacle.minY;
                        const float resolveY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;
                        transform->position.y += resolveY;
                        velocity->velocity[1] = 0.0f;
                        moverY = makeAabb(*transform, *shape);
                    }
                }
            }

            float m_accumulator = 0.0f;
        };

        class Render2DSystem : public System
        {
        public:
            void onUpdate(Scene &scene, float dt) override
            {
                (void)dt;
                BeginDrawing();
                ClearBackground({18, 24, 36, 255});

                for (auto &entity : scene.view<TransformComponent, BoxShape2DComponent, Render2DComponent>())
                {
                    auto *transform = entity.tryGetComponent<TransformComponent>();
                    auto *shape = entity.tryGetComponent<BoxShape2DComponent>();
                    auto *render = entity.tryGetComponent<Render2DComponent>();
                    if (!transform || !shape || !render)
                    {
                        continue;
                    }

                    const float x = transform->position.x - shape->size[0] * 0.5f;
                    const float y = transform->position.y - shape->size[1] * 0.5f;
                    Color color = {render->color[0], render->color[1], render->color[2], render->color[3]};
                    DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(shape->size[0]), static_cast<int>(shape->size[1]), color);
                }

                DrawText("WASD to move", 20, 20, 20, RAYWHITE);
                EndDrawing();
            }
        };
    }

    void Register2DSystems(Scene &scene)
    {
        scene.createSystem<PlayerInputSystem>();
        scene.createSystem<Physics2DSystem>();
        scene.createSystem<Render2DSystem>();
    }
}
