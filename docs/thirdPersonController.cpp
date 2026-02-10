#include <Melkam/core/Engine.hpp>
#include <Melkam/core/Logger.hpp>
#include <Melkam/scene/Components.hpp>
#include <Melkam/scene/Scene.hpp>
#include <Melkam/scene/System.hpp>
#include <Melkam/physics/Collider.hpp>
#include <Melkam/platform/Input.hpp>

#include <raylib.h>
#include <cmath>
#include <raymath.h>

using namespace Melkam;

namespace
{
    float g_cameraYaw = 0.0f;
}

class PlayerMovement3DSystem : public System
{
public:
    void onUpdate(Scene &scene, float dt) override
    {
        for (auto &entity : scene.view<CharacterBody3DComponent, Velocity3DComponent>())
        {
            auto *character = entity.tryGetComponent<CharacterBody3DComponent>();
            auto *velocity = entity.tryGetComponent<Velocity3DComponent>();
            if (!character || !velocity)
            {
                continue;
            }

            const float inputX = Input::GetActionStrength("move_right") - Input::GetActionStrength("move_left");
            const float inputZ = Input::GetActionStrength("move_back") - Input::GetActionStrength("move_forward");

            Vector3 forward = {sinf(g_cameraYaw), 0.0f, cosf(g_cameraYaw)};
            Vector3 right = {cosf(g_cameraYaw), 0.0f, -sinf(g_cameraYaw)};
            Vector3 moveDir = {right.x * inputX + forward.x * inputZ, 0.0f, right.z * inputX + forward.z * inputZ};

            if (Vector3Length(moveDir) > 1.0f)
            {
                moveDir = Vector3Normalize(moveDir);
            }

            velocity->velocity[0] = moveDir.x * character->speed;
            velocity->velocity[2] = moveDir.z * character->speed;

            if (character->useGravity)
            {
                velocity->velocity[1] += character->gravity * dt;
                if (IsOnFloor(entity) && Input::IsActionJustPressed("jump"))
                {
                    velocity->velocity[1] = character->jumpStrength;
                }
            }

            if (Vector3Length(moveDir) > 0.001f)
            {
                if (auto *transform = entity.tryGetComponent<TransformComponent>())
                {
                    transform->rotation.y = atan2f(moveDir.x, moveDir.z);
                }
            }

            MoveAndSlide3D(entity, dt);
        }
    }
};

class ThirdPersonCameraSystem : public System
{
public:
    void onUpdate(Scene &scene, float dt) override
    {
        (void)dt;
        TransformComponent *playerTransform = nullptr;
        for (auto &entity : scene.view<TransformComponent, CharacterBody3DComponent>())
        {
            playerTransform = entity.tryGetComponent<TransformComponent>();
            if (playerTransform)
            {
                break;
            }
        }

        if (!playerTransform)
        {
            return;
        }

        TransformComponent *cameraTransform = nullptr;
        for (auto &entity : scene.view<TransformComponent, CameraComponent>())
        {
            cameraTransform = entity.tryGetComponent<TransformComponent>();
            if (cameraTransform)
            {
                break;
            }
        }

        if (!cameraTransform)
        {
            return;
        }

        const Vector2 delta = GetMouseDelta();
        s_yaw += delta.x * s_sensitivity;
        s_pitch = Clamp(s_pitch - delta.y * s_sensitivity, -1.2f, 0.6f);

        g_cameraYaw = s_yaw;

        const float cosPitch = cosf(s_pitch);
        const Vector3 offset = {
            cosPitch * sinf(s_yaw) * s_distance,
            sinf(s_pitch) * s_distance,
            cosPitch * cosf(s_yaw) * s_distance};

        cameraTransform->position.x = playerTransform->position.x - offset.x;
        cameraTransform->position.y = playerTransform->position.y - offset.y;
        cameraTransform->position.z = playerTransform->position.z - offset.z;
    }

private:
    float s_yaw = 0.0f;
    float s_pitch = -0.3f;
    float s_distance = 8.0f;
    float s_sensitivity = 0.0035f;
};

int main()
{
    Engine engine(EngineConfig{1280, 720, "Melkam Engine"});
    DisableCursor();
    HideCursor();

    Input::AddAction("move_left");
    Input::AddAction("move_right");
    Input::AddAction("move_forward");
    Input::AddAction("move_back");
    Input::AddAction("jump");

    Input::BindKey("move_left", Key::A, 1.0f);
    Input::BindKey("move_left", Key::Left, 1.0f);
    Input::BindKey("move_right", Key::D, 1.0f);
    Input::BindKey("move_right", Key::Right, 1.0f);
    Input::BindKey("move_forward", Key::W, 1.0f);
    Input::BindKey("move_forward", Key::Up, 1.0f);
    Input::BindKey("move_back", Key::S, 1.0f);
    Input::BindKey("move_back", Key::Down, 1.0f);
    Input::BindKey("jump", Key::Space, 1.0f);

    auto scene = engine.createScene("Showcase");
    auto root = scene->createEntity("Root");
    root.addComponent<CameraComponent>();

    auto camera = scene->createChild(root, "Camera");
    auto &cameraTransform = camera.addComponent<TransformComponent>();
    cameraTransform.position = Vector3f{0.0f, 6.0f, 12.0f};
    auto &cameraComponent = camera.addComponent<CameraComponent>();
    cameraComponent.fov = 60.0f;

    auto player = scene->createChild(root, "Player3D");
    auto &playerTransform = player.addComponent<TransformComponent>();
    playerTransform.position = Vector3f{0.0f, 2.0f, 0.0f};
    auto &playerBody = player.addComponent<CharacterBody3DComponent>();
    playerBody.speed = 6.0f;
    playerBody.gravity = -24.0f;
    playerBody.jumpStrength = 9.0f;
    playerBody.useGravity = true;
    player.addComponent<Velocity3DComponent>();
    auto &playerCollider = player.addComponent<ColliderComponent>();
    playerCollider.is2D = false;
    auto &playerBox = player.addComponent<BoxShape3DComponent>();
    playerBox.size[0] = 1.0f;
    playerBox.size[1] = 2.0f;
    playerBox.size[2] = 1.0f;
    auto &playerRender = player.addComponent<Render2DComponent>();
    playerRender.color[0] = 90;
    playerRender.color[1] = 170;
    playerRender.color[2] = 255;
    playerRender.color[3] = 255;
    auto &playerLayers = player.addComponent<CollisionLayerComponent>();
    playerLayers.layer = 1u;
    playerLayers.mask = 2u;

    auto ground = scene->createChild(root, "Ground");
    auto &groundTransform = ground.addComponent<TransformComponent>();
    groundTransform.position = Vector3f{0.0f, -0.5f, 0.0f};
    ground.addComponent<StaticBody3DComponent>();
    auto &groundCollider = ground.addComponent<ColliderComponent>();
    groundCollider.is2D = false;
    auto &groundBox = ground.addComponent<BoxShape3DComponent>();
    groundBox.size[0] = 20.0f;
    groundBox.size[1] = 1.0f;
    groundBox.size[2] = 20.0f;
    auto &groundRender = ground.addComponent<Render2DComponent>();
    groundRender.color[0] = 90;
    groundRender.color[1] = 110;
    groundRender.color[2] = 120;
    groundRender.color[3] = 255;
    auto &groundLayers = ground.addComponent<CollisionLayerComponent>();
    groundLayers.layer = 2u;
    groundLayers.mask = 1u;

    auto wall = scene->createChild(root, "Wall");
    auto &wallTransform = wall.addComponent<TransformComponent>();
    wallTransform.position = Vector3f{4.0f, 1.0f, 0.0f};
    wall.addComponent<StaticBody3DComponent>();
    auto &wallCollider = wall.addComponent<ColliderComponent>();
    wallCollider.is2D = false;
    auto &wallBox = wall.addComponent<BoxShape3DComponent>();
    wallBox.size[0] = 1.0f;
    wallBox.size[1] = 2.0f;
    wallBox.size[2] = 6.0f;
    auto &wallRender = wall.addComponent<Render2DComponent>();
    wallRender.color[0] = 200;
    wallRender.color[1] = 200;
    wallRender.color[2] = 200;
    wallRender.color[3] = 255;
    auto &wallLayers = wall.addComponent<CollisionLayerComponent>();
    wallLayers.layer = 2u;
    wallLayers.mask = 1u;

    auto sphere = scene->createChild(root, "Sphere");
    auto &sphereTransform = sphere.addComponent<TransformComponent>();
    sphereTransform.position = Vector3f{-3.0f, 1.0f, -2.0f};
    sphere.addComponent<StaticBody3DComponent>();
    auto &sphereCollider = sphere.addComponent<ColliderComponent>();
    sphereCollider.is2D = false;
    auto &sphereShape = sphere.addComponent<SphereShape3DComponent>();
    sphereShape.radius = 1.2f;
    auto &sphereRender = sphere.addComponent<Render2DComponent>();
    sphereRender.color[0] = 230;
    sphereRender.color[1] = 160;
    sphereRender.color[2] = 90;
    sphereRender.color[3] = 255;
    auto &sphereLayers = sphere.addComponent<CollisionLayerComponent>();
    sphereLayers.layer = 2u;
    sphereLayers.mask = 1u;

    scene->createSystem<PlayerMovement3DSystem>();
    scene->createSystem<ThirdPersonCameraSystem>();
    RegisterColliderSystems(*scene);

    engine.run();
    return 0;
}
