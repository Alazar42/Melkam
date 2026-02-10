#pragma once

namespace Melkam
{
    template <typename T, typename... Args>
    T &Entity::addComponent(Args &&...args)
    {
        return m_scene->addComponent<T>(m_id, std::forward<Args>(args)...);
    }

    template <typename T>
    bool Entity::hasComponent() const
    {
        return m_scene && m_scene->hasComponent<T>(m_id);
    }

    template <typename T>
    T *Entity::tryGetComponent()
    {
        return m_scene ? m_scene->tryGetComponent<T>(m_id) : nullptr;
    }

    template <typename T>
    const T *Entity::tryGetComponent() const
    {
        return m_scene ? m_scene->tryGetComponent<T>(m_id) : nullptr;
    }

    template <typename T>
    void Entity::removeComponent()
    {
        if (m_scene)
        {
            m_scene->removeComponent<T>(m_id);
        }
    }
}
