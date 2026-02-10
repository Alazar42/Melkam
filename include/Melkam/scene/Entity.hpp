#pragma once
#include <string>
#include <utility>
#include <vector>

#include "Components.hpp"

namespace Melkam
{
    class Scene;

    class Entity
    {
    public:
        Entity();
        Entity(Scene *scene, EntityId id);

        EntityId id() const;
        bool isValid() const;

        const std::string &name() const;
        void setName(const std::string &name);

        Entity parent() const;
        std::vector<Entity> children() const;
        Scene *scene() const;

        template <typename T, typename... Args>
        T &addComponent(Args &&...args);

        template <typename T>
        bool hasComponent() const;

        template <typename T>
        T *tryGetComponent();

        template <typename T>
        const T *tryGetComponent() const;

        template <typename T>
        void removeComponent();

    private:
        Scene *m_scene = nullptr;
        EntityId m_id = InvalidEntity;
    };
}
