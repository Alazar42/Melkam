#include <Melkam/core/Engine.hpp>
#include <Melkam/core/Logger.hpp>
#include <Melkam/scene/Components.hpp>
#include <Melkam/scene/Scene.hpp>
#include <Melkam/scene/System.hpp>
#include <Melkam/physics/Collider.hpp>
#include <Melkam/platform/Input.hpp>

using namespace Melkam;

class PlayerMovementSystem : public System
{
public:
    void onUpdate(Scene &scene, float dt) override
    {
        for (auto &entity : scene.view<CharacterBody2DComponent, Velocity2DComponent>())
        {
            auto *character = entity.tryGetComponent<CharacterBody2DComponent>();
            auto *velocity = entity.tryGetComponent<Velocity2DComponent>();
            if (!character || !velocity)
            {
                continue;
            }

            Axis2D axis = Input::GetActionAxis2D("move_left", "move_right", "move_up", "move_down");
            velocity->velocity[0] = axis.x * character->speed;
            if (!character->useGravity)
            {
                velocity->velocity[1] = axis.y * character->speed;
            }
            else
            {
                velocity->velocity[1] += character->gravity * dt;
                if (IsOnFloor(entity) && Input::IsActionJustPressed("jump"))
                {
                    velocity->velocity[1] = -character->jumpStrength;
                }
            }

            MoveAndSlide2D(entity, dt);
        }
    }
};

int main()
{
    Engine engine(EngineConfig{1280, 720, "Melkam Engine"});

    Input::AddAction("move_left");
    Input::AddAction("move_right");
    Input::AddAction("move_up");
    Input::AddAction("move_down");
    Input::AddAction("jump");

    Input::BindKey("move_left", Key::A, 1.0f);
    Input::BindKey("move_left", Key::Left, 1.0f);
    Input::BindKey("move_right", Key::D, 1.0f);
    Input::BindKey("move_right", Key::Right, 1.0f);
    Input::BindKey("move_up", Key::W, 1.0f);
    Input::BindKey("move_up", Key::Up, 1.0f);
    Input::BindKey("move_down", Key::S, 1.0f);
    Input::BindKey("move_down", Key::Down, 1.0f);
    Input::BindKey("jump", Key::Space, 1.0f);

    auto scene = engine.createScene("Showcase");
    auto root = scene->createEntity("Root");
    root.addComponent<CameraComponent>();

    auto player = scene->createChild(root, "Player");
    auto &playerTransform = player.addComponent<TransformComponent>();
    playerTransform.position.x = 240.0f;
    playerTransform.position.y = 240.0f;
    auto &playerBody = player.addComponent<CharacterBody2DComponent>();
    playerBody.speed = 260.0f;
    playerBody.useGravity = false;
    player.addComponent<Velocity2DComponent>();
    player.addComponent<ColliderComponent>();
    auto &playerBox = player.addComponent<BoxShape2DComponent>();
    playerBox.size[0] = 40.0f;
    playerBox.size[1] = 40.0f;
    auto &playerRender = player.addComponent<Render2DComponent>();
    playerRender.color[0] = 64;
    playerRender.color[1] = 128;
    playerRender.color[2] = 255;
    playerRender.color[3] = 255;
    auto &playerLayers = player.addComponent<CollisionLayerComponent>();
    playerLayers.layer = 1u;
    playerLayers.mask = 2u;

    auto wall = scene->createChild(root, "Wall");
    auto &wallTransform = wall.addComponent<TransformComponent>();
    wallTransform.position.x = 520.0f;
    wallTransform.position.y = 240.0f;
    wall.addComponent<StaticBody2DComponent>();
    wall.addComponent<ColliderComponent>();
    auto &wallBox = wall.addComponent<BoxShape2DComponent>();
    wallBox.size[0] = 220.0f;
    wallBox.size[1] = 60.0f;
    auto &wallRender = wall.addComponent<Render2DComponent>();
    wallRender.color[0] = 180;
    wallRender.color[1] = 180;
    wallRender.color[2] = 180;
    wallRender.color[3] = 255;
    auto &wallLayers = wall.addComponent<CollisionLayerComponent>();
    wallLayers.layer = 2u;
    wallLayers.mask = 1u;


    auto floor = scene->createChild(root, "Floor");
    auto &floorTransform = floor.addComponent<TransformComponent>();
    floorTransform.position.x = 400.0f;
    floorTransform.position.y = 520.0f;
    floor.addComponent<StaticBody2DComponent>();
    floor.addComponent<ColliderComponent>();
    auto &floorBox = floor.addComponent<BoxShape2DComponent>();
    floorBox.size[0] = 600.0f;
    floorBox.size[1] = 40.0f;
    auto &floorRender = floor.addComponent<Render2DComponent>();
    floorRender.color[0] = 70;
    floorRender.color[1] = 70;
    floorRender.color[2] = 90;
    floorRender.color[3] = 255;
    auto &floorLayers = floor.addComponent<CollisionLayerComponent>();
    floorLayers.layer = 2u;
    floorLayers.mask = 1u;

    auto pillar = scene->createChild(root, "Pillar");
    auto &pillarTransform = pillar.addComponent<TransformComponent>();
    pillarTransform.position.x = 300.0f;
    pillarTransform.position.y = 360.0f;
    pillar.addComponent<StaticBody2DComponent>();
    pillar.addComponent<ColliderComponent>();
    auto &pillarCircle = pillar.addComponent<CircleShape2DComponent>();
    pillarCircle.radius = 30.0f;
    auto &pillarRender = pillar.addComponent<Render2DComponent>();
    pillarRender.color[0] = 230;
    pillarRender.color[1] = 190;
    pillarRender.color[2] = 80;
    pillarRender.color[3] = 255;
    auto &pillarLayers = pillar.addComponent<CollisionLayerComponent>();
    pillarLayers.layer = 2u;
    pillarLayers.mask = 1u;

    auto box3d = scene->createChild(root, "Box3D");
    auto &box3dTransform = box3d.addComponent<TransformComponent>();
    box3dTransform.position.x = -200.0f;
    box3dTransform.position.y = 0.0f;
    box3dTransform.position.z = 0.0f;
    box3d.addComponent<StaticBody3DComponent>();
    auto &box3dCollider = box3d.addComponent<ColliderComponent>();
    box3dCollider.is2D = false;
    auto &box3dShape = box3d.addComponent<BoxShape3DComponent>();
    box3dShape.size[0] = 3.0f;
    box3dShape.size[1] = 3.0f;
    box3dShape.size[2] = 3.0f;
    auto &box3dLayers = box3d.addComponent<CollisionLayerComponent>();
    box3dLayers.layer = 4u;
    box3dLayers.mask = 8u;

    auto sphere3d = scene->createChild(root, "Sphere3D");
    auto &sphere3dTransform = sphere3d.addComponent<TransformComponent>();
    sphere3dTransform.position.x = -210.0f;
    sphere3dTransform.position.y = 0.0f;
    sphere3dTransform.position.z = 5.0f;
    sphere3d.addComponent<StaticBody3DComponent>();
    auto &sphere3dCollider = sphere3d.addComponent<ColliderComponent>();
    sphere3dCollider.is2D = false;
    auto &sphere3dShape = sphere3d.addComponent<SphereShape3DComponent>();
    sphere3dShape.radius = 1.8f;
    auto &sphere3dLayers = sphere3d.addComponent<CollisionLayerComponent>();
    sphere3dLayers.layer = 8u;
    sphere3dLayers.mask = 4u;

    scene->createSystem<PlayerMovementSystem>();
    RegisterColliderSystems(*scene);

    engine.run();
    return 0;
}
