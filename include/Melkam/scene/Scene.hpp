#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Components.hpp"
#include "Entity.hpp"

namespace Melkam
{
    class System;

    class Scene
    {
    public:
        explicit Scene(std::string name);
        ~Scene();

        const std::string &name() const;

        Entity createEntity(const std::string &name = "Entity");
        Entity createChild(Entity parent, const std::string &name = "Entity");
        void destroyEntity(Entity entity);

        void setParent(Entity child, Entity parent);
        std::vector<Entity> rootEntities() const;

        bool isValid(EntityId id) const;

        void update(float dt);
        void traverse(const std::function<void(Entity &)> &pre,
                  const std::function<void(Entity &)> &post);

        void addSystem(std::unique_ptr<System> system);
        void clearSystems();

        template <typename T, typename... Args>
        T &createSystem(Args &&...args)
        {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            auto *ptr = system.get();
            addSystem(std::move(system));
            return *ptr;
        }

        template <typename... Components>
        std::vector<Entity> view() const
        {
            std::vector<Entity> result;
            for (EntityId id : m_entities)
            {
                if ((hasComponent<Components>(id) && ...))
                {
                    result.emplace_back(const_cast<Scene *>(this), id);
                }
            }
            return result;
        }

        template <typename T, typename... Args>
        T &addComponent(EntityId id, Args &&...args)
        {
            auto &storage = getOrCreateStorage<T>();
            auto result = storage.data.emplace(id, T(std::forward<Args>(args)...));
            if (!result.second)
            {
                result.first->second = T(std::forward<Args>(args)...);
            }
            return result.first->second;
        }

        template <typename T>
        bool hasComponent(EntityId id) const
        {
            const auto *storage = findStorage<T>();
            return storage ? storage->has(id) : false;
        }

        template <typename T>
        T *tryGetComponent(EntityId id)
        {
            auto *storage = findStorage<T>();
            if (!storage)
            {
                return nullptr;
            }
            auto it = storage->data.find(id);
            return it == storage->data.end() ? nullptr : &it->second;
        }

        template <typename T>
        const T *tryGetComponent(EntityId id) const
        {
            const auto *storage = findStorage<T>();
            if (!storage)
            {
                return nullptr;
            }
            auto it = storage->data.find(id);
            return it == storage->data.end() ? nullptr : &it->second;
        }

        template <typename T>
        void removeComponent(EntityId id)
        {
            auto *storage = findStorage<T>();
            if (storage)
            {
                storage->data.erase(id);
            }
        }

    private:
        struct IComponentStorage
        {
            virtual ~IComponentStorage() = default;
            virtual void remove(EntityId id) = 0;
            virtual bool has(EntityId id) const = 0;
        };

        template <typename T>
        struct ComponentStorage : IComponentStorage
        {
            std::unordered_map<EntityId, T> data;

            void remove(EntityId id) override
            {
                data.erase(id);
            }

            bool has(EntityId id) const override
            {
                return data.find(id) != data.end();
            }
        };

        template <typename T>
        ComponentStorage<T> &getOrCreateStorage()
        {
            const auto type = std::type_index(typeid(T));
            auto it = m_components.find(type);
            if (it == m_components.end())
            {
                auto storage = std::make_unique<ComponentStorage<T>>();
                auto *ptr = storage.get();
                m_components.emplace(type, std::move(storage));
                return *ptr;
            }
            return *static_cast<ComponentStorage<T> *>(it->second.get());
        }

        template <typename T>
        ComponentStorage<T> *findStorage()
        {
            const auto type = std::type_index(typeid(T));
            auto it = m_components.find(type);
            return it == m_components.end() ? nullptr : static_cast<ComponentStorage<T> *>(it->second.get());
        }

        template <typename T>
        const ComponentStorage<T> *findStorage() const
        {
            const auto type = std::type_index(typeid(T));
            auto it = m_components.find(type);
            return it == m_components.end() ? nullptr : static_cast<const ComponentStorage<T> *>(it->second.get());
        }

        std::string m_name;
        EntityId m_nextId = InvalidEntity;
        std::vector<EntityId> m_entities;
        std::unordered_set<EntityId> m_entitySet;
        std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> m_components;
        std::vector<std::unique_ptr<System>> m_systems;

        void traverseRecursive(Entity entity,
                               const std::function<void(Entity &)> &pre,
                               const std::function<void(Entity &)> &post);
    };
}

#include "Entity.inl"
