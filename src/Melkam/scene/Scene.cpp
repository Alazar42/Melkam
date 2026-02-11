#include <Melkam/scene/Scene.hpp>
#include <Melkam/scene/Entity.hpp>
#include <Melkam/scene/System.hpp>

namespace Melkam
{
    Scene::Scene(std::string name) : m_name(std::move(name))
    {
    }

    Scene::~Scene() = default;

    const std::string &Scene::name() const
    {
        return m_name;
    }

    Entity Scene::createEntity(const std::string &name)
    {
        EntityId id = ++m_nextId;
        m_entities.push_back(id);
        m_entitySet.insert(id);

        addComponent<NameComponent>(id, NameComponent{name});
        addComponent<NodeComponent>(id, NodeComponent{});
        addComponent<TransformComponent>(id, TransformComponent{});

        return Entity(this, id);
    }

    Entity Scene::createChild(Entity parent, const std::string &name)
    {
        Entity child = createEntity(name);
        setParent(child, parent);
        return child;
    }

    void Scene::destroyEntity(Entity entity)
    {
        if (!entity.isValid())
        {
            return;
        }

        const EntityId id = entity.id();
        auto *node = tryGetComponent<NodeComponent>(id);
        if (node)
        {
            if (node->parent != InvalidEntity)
            {
                auto *parentNode = tryGetComponent<NodeComponent>(node->parent);
                if (parentNode)
                {
                    auto &siblings = parentNode->children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), id), siblings.end());
                }
            }

            for (EntityId childId : node->children)
            {
                auto *childNode = tryGetComponent<NodeComponent>(childId);
                if (childNode)
                {
                    childNode->parent = InvalidEntity;
                }
            }
        }

        for (auto &pair : m_components)
        {
            pair.second->remove(id);
        }

        m_entitySet.erase(id);
        m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), id), m_entities.end());
    }

    void Scene::setParent(Entity child, Entity parent)
    {
        if (!child.isValid())
        {
            return;
        }

        const EntityId childId = child.id();
        const EntityId parentId = parent.isValid() ? parent.id() : InvalidEntity;

        if (parentId != InvalidEntity && !isValid(parentId))
        {
            return;
        }

        auto *childNode = tryGetComponent<NodeComponent>(childId);
        if (!childNode)
        {
            return;
        }

        if (childNode->parent == parentId)
        {
            return;
        }

        if (childNode->parent != InvalidEntity)
        {
            auto *oldParentNode = tryGetComponent<NodeComponent>(childNode->parent);
            if (oldParentNode)
            {
                auto &siblings = oldParentNode->children;
                siblings.erase(std::remove(siblings.begin(), siblings.end(), childId), siblings.end());
            }
        }

        childNode->parent = parentId;

        if (parentId != InvalidEntity)
        {
            auto *parentNode = tryGetComponent<NodeComponent>(parentId);
            if (parentNode)
            {
                parentNode->children.push_back(childId);
            }
        }
    }

    std::vector<Entity> Scene::rootEntities() const
    {
        std::vector<Entity> roots;
        for (EntityId id : m_entities)
        {
            const auto *node = tryGetComponent<NodeComponent>(id);
            if (node && node->parent == InvalidEntity)
            {
                roots.emplace_back(const_cast<Scene *>(this), id);
            }
        }
        return roots;
    }

    void Scene::update(float dt)
    {
        for (auto &system : m_systems)
        {
            system->onUpdate(*this, dt);
        }

        traverse(
            [this, dt](Entity &entity)
            {
                for (auto &system : m_systems)
                {
                    system->onPreUpdate(*this, entity, dt);
                }
            },
            [this, dt](Entity &entity)
            {
                for (auto &system : m_systems)
                {
                    system->onPostUpdate(*this, entity, dt);
                }
            });
    }

    void Scene::traverse(const std::function<void(Entity &)> &pre,
                         const std::function<void(Entity &)> &post)
    {
        auto roots = rootEntities();
        for (const auto &root : roots)
        {
            traverseRecursive(root, pre, post);
        }
    }

    void Scene::addSystem(std::unique_ptr<System> system)
    {
        if (!system)
        {
            return;
        }

        m_systems.push_back(std::move(system));
    }

    void Scene::clearSystems()
    {
        m_systems.clear();
    }

    void Scene::setBuilder(Builder builder)
    {
        m_builder = std::move(builder);
    }

    bool Scene::rebuild()
    {
        if (!m_builder)
        {
            return false;
        }

        clear();
        m_builder(*this);
        return true;
    }

    void Scene::clear()
    {
        m_components.clear();
        m_entities.clear();
        m_entitySet.clear();
        m_systems.clear();
        m_nextId = InvalidEntity;
    }

    bool Scene::isValid(EntityId id) const
    {
        return m_entitySet.find(id) != m_entitySet.end();
    }

    void Scene::traverseRecursive(Entity entity,
                                  const std::function<void(Entity &)> &pre,
                                  const std::function<void(Entity &)> &post)
    {
        if (!entity.isValid())
        {
            return;
        }

        if (pre)
        {
            pre(entity);
        }

        const auto *node = tryGetComponent<NodeComponent>(entity.id());
        if (node)
        {
            for (EntityId childId : node->children)
            {
                if (isValid(childId))
                {
                    traverseRecursive(Entity(this, childId), pre, post);
                }
            }
        }

        if (post)
        {
            post(entity);
        }
    }
}
