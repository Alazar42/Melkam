#pragma once

namespace Melkam
{
    class Scene;
    class Entity;

    class System
    {
    public:
        virtual ~System() = default;

        virtual void onUpdate(Scene &scene, float dt) {}
        virtual void onPreUpdate(Scene &scene, Entity &entity, float dt) {}
        virtual void onPostUpdate(Scene &scene, Entity &entity, float dt) {}
    };
}
