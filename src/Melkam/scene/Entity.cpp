#include <Melkam/scene/Entity.hpp>
#include <Melkam/scene/Scene.hpp>
#include <Melkam/scene/Components.hpp>

namespace Melkam
{
    Entity::Entity() = default;

    Entity::Entity(Scene *scene, EntityId id) : m_scene(scene), m_id(id)
    {
    }

    EntityId Entity::id() const
    {
        return m_id;
    }

    bool Entity::isValid() const
    {
        return m_scene && m_scene->isValid(m_id);
    }

    const std::string &Entity::name() const
    {
        static const std::string empty;
        if (!m_scene)
        {
            return empty;
        }

        const auto *nameComponent = m_scene->tryGetComponent<NameComponent>(m_id);
        return nameComponent ? nameComponent->name : empty;
    }

    void Entity::setName(const std::string &name)
    {
        if (!m_scene)
        {
            return;
        }

        auto *nameComponent = m_scene->tryGetComponent<NameComponent>(m_id);
        if (nameComponent)
        {
            nameComponent->name = name;
        }
        else
        {
            m_scene->addComponent<NameComponent>(m_id, NameComponent{name});
        }
    }

    Entity Entity::parent() const
    {
        if (!m_scene)
        {
            return Entity();
        }

        const auto *node = m_scene->tryGetComponent<NodeComponent>(m_id);
        if (!node || node->parent == InvalidEntity)
        {
            return Entity();
        }

        return Entity(m_scene, node->parent);
    }

    std::vector<Entity> Entity::children() const
    {
        std::vector<Entity> result;
        if (!m_scene)
        {
            return result;
        }

        const auto *node = m_scene->tryGetComponent<NodeComponent>(m_id);
        if (!node)
        {
            return result;
        }

        result.reserve(node->children.size());
        for (EntityId childId : node->children)
        {
            result.emplace_back(m_scene, childId);
        }
        return result;
    }

    Scene *Entity::scene() const
    {
        return m_scene;
    }
}
