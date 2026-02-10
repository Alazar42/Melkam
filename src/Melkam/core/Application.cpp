#include <Melkam/core/Application.hpp>
#include <Melkam/scene/Components.hpp>
#include <Melkam/scene/Entity.hpp>
#include <Melkam/scene/Scene.hpp>
#include <Melkam/scene/System.hpp>

namespace Melkam
{
    namespace
    {
        class SpinSystem : public System
        {
        public:
            void onPreUpdate(Scene &scene, Entity &entity, float dt) override
            {
                auto *transform = entity.tryGetComponent<TransformComponent>();
                if (transform)
                {
                    transform->rotation.y += dt;
                }
            }
        };
    }

    void Application::Run(Melkam::Engine &engine)
    {
        auto scene = engine.activeScene();
        if (!scene)
        {
            scene = engine.createScene("Main");
            auto root = scene->createEntity("Root");
            root.addComponent<CameraComponent>();

            auto child = scene->createChild(root, "Child");
            child.addComponent<MeshComponent>(MeshComponent{"meshes/cube.mesh", "materials/default.mat"});
            child.addComponent<RigidBodyComponent>();

            scene->createSystem<SpinSystem>();

            auto renderables = scene->view<TransformComponent, MeshComponent>();
            (void)renderables;
        }
    }
}
