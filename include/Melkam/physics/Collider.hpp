#pragma once

#include <functional>

#include <Melkam/scene/Components.hpp>

namespace Melkam
{
    class Scene;
    class Entity;

    struct CollisionInfo
    {
        bool hit = false;
        EntityId collider = InvalidEntity;
        float normal[3] = {0.0f, 0.0f, 0.0f};
        float travel = 0.0f;
    };

    using CollisionCallback = std::function<void(Entity self, Entity other, const CollisionInfo &info)>;
    using AreaCallback = std::function<void(Entity area, Entity body)>;

    void RegisterColliderSystems(Scene &scene);
    void SetSlideSettings(float epsilon, int maxSlides);
    bool MoveAndSlide2D(Entity &entity, float dt);
    bool MoveAndSlide3D(Entity &entity, float dt);
    bool MoveAndCollide2D(Entity &entity, const float motion[2], float dt);
    bool MoveAndCollide3D(Entity &entity, const float motion[3], float dt);
    bool MoveAndCollide2D(Entity &entity, const float motion[2], float dt, CollisionInfo &outInfo);
    bool MoveAndCollide3D(Entity &entity, const float motion[3], float dt, CollisionInfo &outInfo);
    bool IsOnFloor(const Entity &entity);
    bool IsOnWall(const Entity &entity);
    bool IsOnCeiling(const Entity &entity);
    void GetFloorNormal(const Entity &entity, float outNormal[3]);

    void ConnectCollisionSignal(Entity entity, CollisionCallback callback);
    void ConnectAreaBodyEntered(Entity area, AreaCallback callback);
    void ConnectAreaBodyExited(Entity area, AreaCallback callback);
}
