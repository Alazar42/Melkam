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
#include <memory>
#include <string>

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

            const float inputX = Input::GetActionStrength("move_left") - Input::GetActionStrength("move_right");
            const float inputZ = Input::GetActionStrength("move_forward") - Input::GetActionStrength("move_back");

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

        if (IsWindowFocused())
        {
            if (!s_mouseCaptured)
            {
                Input::SetMouseMode(MouseMode::Captured);
                s_mouseCaptured = true;
            }
        }
        else if (s_mouseCaptured)
        {
            Input::SetMouseMode(MouseMode::Visible);
            s_mouseCaptured = false;
        }

        const Axis2D delta = Input::GetMouseDelta();
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
    bool s_mouseCaptured = false;
};

int main()
{
    Engine engine(EngineConfig{1280, 720, "Melkam Engine"});

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
    auto nextScene = engine.createScene("SecondScene");
    auto showcaseScene = scene;
    auto changeToken = std::make_shared<bool>(false);
    auto returnToken = std::make_shared<bool>(false);

    auto buildSecond = [&](Scene &scene)
    {
        *returnToken = false;

        auto root = scene.createEntity("Root");
        root.addComponent<CameraComponent>();

        auto camera = scene.createChild(root, "Camera");
        auto &cameraTransform = camera.addComponent<TransformComponent>();
        cameraTransform.position = Vector3f{0.0f, 5.0f, 10.0f};
        auto &cameraComponent = camera.addComponent<CameraComponent>();
        cameraComponent.fov = 60.0f;

        auto player = scene.createChild(root, "Player3D");
        auto &playerTransform = player.addComponent<TransformComponent>();
        playerTransform.position = Vector3f{0.0f, 2.0f, -2.0f};
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
        playerRender.color[0] = 120;
        playerRender.color[1] = 200;
        playerRender.color[2] = 255;
        playerRender.color[3] = 255;
        auto &playerLayers = player.addComponent<CollisionLayerComponent>();
        playerLayers.layer = 1u;
        playerLayers.mask = 2u;

        auto ground = scene.createChild(root, "Ground");
        auto &groundTransform = ground.addComponent<TransformComponent>();
        groundTransform.position = Vector3f{0.0f, -0.5f, 0.0f};
        ground.addComponent<StaticBody3DComponent>();
        auto &groundCollider = ground.addComponent<ColliderComponent>();
        groundCollider.is2D = false;
        auto &groundBox = ground.addComponent<BoxShape3DComponent>();
        groundBox.size[0] = 16.0f;
        groundBox.size[1] = 1.0f;
        groundBox.size[2] = 16.0f;
        auto &groundRender = ground.addComponent<Render2DComponent>();
        groundRender.color[0] = 80;
        groundRender.color[1] = 60;
        groundRender.color[2] = 140;
        groundRender.color[3] = 255;
        auto &groundLayers = ground.addComponent<CollisionLayerComponent>();
        groundLayers.layer = 2u;
        groundLayers.mask = 1u;

        auto marker = scene.createChild(root, "Marker");
        auto &markerTransform = marker.addComponent<TransformComponent>();
        markerTransform.position = Vector3f{0.0f, 1.0f, 0.0f};
        marker.addComponent<StaticBody3DComponent>();
        auto &markerCollider = marker.addComponent<ColliderComponent>();
        markerCollider.is2D = false;
        auto &markerBox = marker.addComponent<BoxShape3DComponent>();
        markerBox.size[0] = 1.5f;
        markerBox.size[1] = 1.5f;
        markerBox.size[2] = 1.5f;
        auto &markerRender = marker.addComponent<Render2DComponent>();
        markerRender.color[0] = 140;
        markerRender.color[1] = 200;
        markerRender.color[2] = 140;
        markerRender.color[3] = 255;
        auto &markerLayers = marker.addComponent<CollisionLayerComponent>();
        markerLayers.layer = 2u;
        markerLayers.mask = 1u;

        auto coin = scene.createChild(root, "CoinReturn");
        auto &coinTransform = coin.addComponent<TransformComponent>();
        coinTransform.position = Vector3f{2.5f, 1.0f, 1.5f};
        coin.addComponent<Area3DComponent>();
        auto &coinCollider = coin.addComponent<ColliderComponent>();
        coinCollider.is2D = false;
        coinCollider.isTrigger = true;
        auto &coinShape = coin.addComponent<SphereShape3DComponent>();
        coinShape.radius = 0.6f;
        auto &coinRender = coin.addComponent<Render2DComponent>();
        coinRender.color[0] = 250;
        coinRender.color[1] = 200;
        coinRender.color[2] = 90;
        coinRender.color[3] = 255;
        auto &coinLayers = coin.addComponent<CollisionLayerComponent>();
        coinLayers.layer = 2u;
        coinLayers.mask = 1u;

        ConnectAreaBodyEntered(coin, [&, showcaseScene](Entity area, Entity body)
                               {
                                   (void)area;
                                   std::string message = "Return coin touched by " + body.name();
                                   Logger::Info(message);

                                   if (*returnToken)
                                   {
                                       return;
                                   }

                                   *returnToken = true;
                                   showcaseScene->rebuild();
                                   engine.requestSceneChange(showcaseScene);
                               });

        auto hudLayer = scene.createChild(root, "HudLayer");
        hudLayer.addComponent<CanvasLayerComponent>().layer = 1;

        auto hudRoot = scene.createChild(hudLayer, "HudRoot");
        auto &hudControl = hudRoot.addComponent<ControlComponent>();
        hudControl.anchorLeft = 0.0f;
        hudControl.anchorTop = 0.0f;
        hudControl.anchorRight = 1.0f;
        hudControl.anchorBottom = 1.0f;
        hudControl.offsetLeft = 0.0f;
        hudControl.offsetTop = 0.0f;
        hudControl.offsetRight = 0.0f;
        hudControl.offsetBottom = 0.0f;

        auto panel = scene.createChild(hudRoot, "HudPanel");
        auto &panelControl = panel.addComponent<ControlComponent>();
        panelControl.anchorLeft = 1.0f;
        panelControl.anchorTop = 0.0f;
        panelControl.anchorRight = 1.0f;
        panelControl.anchorBottom = 0.0f;
        panelControl.offsetLeft = -300.0f;
        panelControl.offsetTop = 16.0f;
        panelControl.offsetRight = -16.0f;
        panelControl.offsetBottom = 96.0f;
        auto &panelRect = panel.addComponent<ColorRectComponent>();
        panelRect.color[0] = 28;
        panelRect.color[1] = 20;
        panelRect.color[2] = 40;
        panelRect.color[3] = 200;

        auto title = scene.createChild(panel, "HudTitle");
        auto &titleControl = title.addComponent<ControlComponent>();
        titleControl.anchorLeft = 0.0f;
        titleControl.anchorTop = 0.0f;
        titleControl.anchorRight = 0.0f;
        titleControl.anchorBottom = 0.0f;
        titleControl.offsetLeft = 12.0f;
        titleControl.offsetTop = 10.0f;
        titleControl.offsetRight = 280.0f;
        titleControl.offsetBottom = 32.0f;
        auto &titleLabel = title.addComponent<LabelComponent>();
        titleLabel.text = "HUD: Second Scene";
        titleLabel.fontSize = 22;

        auto hint = scene.createChild(panel, "HudHint");
        auto &hintControl = hint.addComponent<ControlComponent>();
        hintControl.anchorLeft = 0.0f;
        hintControl.anchorTop = 0.0f;
        hintControl.anchorRight = 0.0f;
        hintControl.anchorBottom = 0.0f;
        hintControl.offsetLeft = 12.0f;
        hintControl.offsetTop = 38.0f;
        hintControl.offsetRight = 280.0f;
        hintControl.offsetBottom = 62.0f;
        auto &hintLabel = hint.addComponent<LabelComponent>();
        hintLabel.text = "Touch the coin to return";
        hintLabel.fontSize = 16;

        scene.createSystem<PlayerMovement3DSystem>();
        scene.createSystem<ThirdPersonCameraSystem>();
        RegisterColliderSystems(scene);
    };

    auto buildShowcase = [&](Scene &scene)
    {
        *changeToken = false;

        auto root = scene.createEntity("Root");
        root.addComponent<CameraComponent>();

        auto camera = scene.createChild(root, "Camera");
        auto &cameraTransform = camera.addComponent<TransformComponent>();
        cameraTransform.position = Vector3f{0.0f, 6.0f, 12.0f};
        auto &cameraComponent = camera.addComponent<CameraComponent>();
        cameraComponent.fov = 60.0f;

        auto player = scene.createChild(root, "Player3D");
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

        auto ground = scene.createChild(root, "Ground");
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

        auto wall = scene.createChild(root, "Wall");
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

        auto sphere = scene.createChild(root, "Sphere");
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

        auto coin = scene.createChild(root, "Coin");
        auto &coinTransform = coin.addComponent<TransformComponent>();
        coinTransform.position = Vector3f{0.0f, 1.0f, 4.0f};
        coin.addComponent<Area3DComponent>();
        auto &coinCollider = coin.addComponent<ColliderComponent>();
        coinCollider.is2D = false;
        coinCollider.isTrigger = true;
        auto &coinShape = coin.addComponent<SphereShape3DComponent>();
        coinShape.radius = 0.6f;
        auto &coinRender = coin.addComponent<Render2DComponent>();
        coinRender.color[0] = 250;
        coinRender.color[1] = 210;
        coinRender.color[2] = 70;
        coinRender.color[3] = 255;
        auto &coinLayers = coin.addComponent<CollisionLayerComponent>();
        coinLayers.layer = 2u;
        coinLayers.mask = 1u;

        ConnectAreaBodyEntered(coin, [&, nextScene](Entity area, Entity body)
                               {
                                   (void)area;
                                   std::string message = "Coin touched by " + body.name();
                                   Logger::Info(message);

                                   if (*changeToken)
                                   {
                                       return;
                                   }

                                   *changeToken = true;
                                   nextScene->rebuild();
                                   engine.requestSceneChange(nextScene);
                               });

        auto hudLayer = scene.createChild(root, "HudLayer");
        hudLayer.addComponent<CanvasLayerComponent>().layer = 1;

        auto hudRoot = scene.createChild(hudLayer, "HudRoot");
        auto &hudControl = hudRoot.addComponent<ControlComponent>();
        hudControl.anchorLeft = 0.0f;
        hudControl.anchorTop = 0.0f;
        hudControl.anchorRight = 1.0f;
        hudControl.anchorBottom = 1.0f;
        hudControl.offsetLeft = 0.0f;
        hudControl.offsetTop = 0.0f;
        hudControl.offsetRight = 0.0f;
        hudControl.offsetBottom = 0.0f;

        auto panel = scene.createChild(hudRoot, "HudPanel");
        auto &panelControl = panel.addComponent<ControlComponent>();
        panelControl.anchorLeft = 0.0f;
        panelControl.anchorTop = 0.0f;
        panelControl.anchorRight = 0.0f;
        panelControl.anchorBottom = 0.0f;
        panelControl.offsetLeft = 16.0f;
        panelControl.offsetTop = 16.0f;
        panelControl.offsetRight = 300.0f;
        panelControl.offsetBottom = 96.0f;
        auto &panelRect = panel.addComponent<ColorRectComponent>();
        panelRect.color[0] = 20;
        panelRect.color[1] = 28;
        panelRect.color[2] = 40;
        panelRect.color[3] = 200;

        auto title = scene.createChild(panel, "HudTitle");
        auto &titleControl = title.addComponent<ControlComponent>();
        titleControl.anchorLeft = 0.0f;
        titleControl.anchorTop = 0.0f;
        titleControl.anchorRight = 0.0f;
        titleControl.anchorBottom = 0.0f;
        titleControl.offsetLeft = 12.0f;
        titleControl.offsetTop = 10.0f;
        titleControl.offsetRight = 280.0f;
        titleControl.offsetBottom = 32.0f;
        auto &titleLabel = title.addComponent<LabelComponent>();
        titleLabel.text = "HUD: Showcase";
        titleLabel.fontSize = 22;

        auto hint = scene.createChild(panel, "HudHint");
        auto &hintControl = hint.addComponent<ControlComponent>();
        hintControl.anchorLeft = 0.0f;
        hintControl.anchorTop = 0.0f;
        hintControl.anchorRight = 0.0f;
        hintControl.anchorBottom = 0.0f;
        hintControl.offsetLeft = 12.0f;
        hintControl.offsetTop = 38.0f;
        hintControl.offsetRight = 280.0f;
        hintControl.offsetBottom = 62.0f;
        auto &hintLabel = hint.addComponent<LabelComponent>();
        hintLabel.text = "Walk into the coin to change scene";
        hintLabel.fontSize = 16;

        scene.createSystem<PlayerMovement3DSystem>();
        scene.createSystem<ThirdPersonCameraSystem>();
        RegisterColliderSystems(scene);
    };

    scene->setBuilder(buildShowcase);
    nextScene->setBuilder(buildSecond);
    scene->rebuild();
    nextScene->rebuild();

    engine.run();
    return 0;
}
