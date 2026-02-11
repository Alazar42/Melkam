#include <Melkam/core/Engine.hpp>
#include <Melkam/core/Logger.hpp>
#include <Melkam/scene/Components.hpp>
#include <Melkam/scene/Scene.hpp>
#include <Melkam/scene/System.hpp>
#include <Melkam/physics/Collider.hpp>
#include <Melkam/platform/Input.hpp>
#include <Melkam/ui/Ui.hpp>

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

    auto menuScene = engine.createScene("MainMenu");
    auto uiScene = engine.createScene("UiShowcase");
    auto menuClickToken = std::make_shared<bool>(false);
    auto backToken = std::make_shared<bool>(false);

    auto buildMenu = [&](Scene &scene)
    {
        *menuClickToken = false;

        auto layer = scene.createEntity("MenuLayer");
        layer.addComponent<CanvasLayerComponent>().layer = 0;

        auto root = scene.createChild(layer, "MenuRoot");
        auto &rootControl = root.addComponent<ControlComponent>();
        rootControl.anchorLeft = 0.0f;
        rootControl.anchorTop = 0.0f;
        rootControl.anchorRight = 1.0f;
        rootControl.anchorBottom = 1.0f;
        rootControl.offsetLeft = 0.0f;
        rootControl.offsetTop = 0.0f;
        rootControl.offsetRight = 0.0f;
        rootControl.offsetBottom = 0.0f;

        auto background = scene.createChild(root, "Background");
        auto &bgControl = background.addComponent<ControlComponent>();
        bgControl.anchorLeft = 0.0f;
        bgControl.anchorTop = 0.0f;
        bgControl.anchorRight = 1.0f;
        bgControl.anchorBottom = 1.0f;
        bgControl.offsetLeft = 0.0f;
        bgControl.offsetTop = 0.0f;
        bgControl.offsetRight = 0.0f;
        bgControl.offsetBottom = 0.0f;
        auto &bgRect = background.addComponent<ColorRectComponent>();
        bgRect.color[0] = 12;
        bgRect.color[1] = 16;
        bgRect.color[2] = 24;
        bgRect.color[3] = 255;

        auto title = scene.createChild(root, "Title");
        auto &titleControl = title.addComponent<ControlComponent>();
        titleControl.anchorLeft = 0.5f;
        titleControl.anchorTop = 0.25f;
        titleControl.anchorRight = 0.5f;
        titleControl.anchorBottom = 0.25f;
        titleControl.offsetLeft = -200.0f;
        titleControl.offsetTop = -40.0f;
        titleControl.offsetRight = 200.0f;
        titleControl.offsetBottom = 10.0f;
        auto &titleLabel = title.addComponent<LabelComponent>();
        titleLabel.text = "MELKAM UI";
        titleLabel.fontSize = 36;

        auto subtitle = scene.createChild(root, "Subtitle");
        auto &subtitleControl = subtitle.addComponent<ControlComponent>();
        subtitleControl.anchorLeft = 0.5f;
        subtitleControl.anchorTop = 0.35f;
        subtitleControl.anchorRight = 0.5f;
        subtitleControl.anchorBottom = 0.35f;
        subtitleControl.offsetLeft = -220.0f;
        subtitleControl.offsetTop = -10.0f;
        subtitleControl.offsetRight = 220.0f;
        subtitleControl.offsetBottom = 20.0f;
        auto &subtitleLabel = subtitle.addComponent<LabelComponent>();
        subtitleLabel.text = "Godot-style Control + CanvasLayer";
        subtitleLabel.fontSize = 18;

        auto startButton = scene.createChild(root, "StartButton");
        auto &startControl = startButton.addComponent<ControlComponent>();
        startControl.anchorLeft = 0.5f;
        startControl.anchorTop = 0.55f;
        startControl.anchorRight = 0.5f;
        startControl.anchorBottom = 0.55f;
        startControl.offsetLeft = -170.0f;
        startControl.offsetTop = -24.0f;
        startControl.offsetRight = 170.0f;
        startControl.offsetBottom = 24.0f;
        auto &startBtn = startButton.addComponent<ButtonComponent>();
        startBtn.text = "Open UI Showcase";
        startBtn.fontSize = 20;

        ConnectButtonPressed(startButton, [&, uiScene](Entity button)
                             {
                                 (void)button;
                                 if (*menuClickToken)
                                 {
                                     return;
                                 }

                                 *menuClickToken = true;
                                 uiScene->rebuild();
                                 engine.requestSceneChange(uiScene);
                             });

        RegisterUiSystems(scene);
    };

    auto buildUi = [&](Scene &scene)
    {
        *backToken = false;

        auto layer = scene.createEntity("UiLayer");
        layer.addComponent<CanvasLayerComponent>().layer = 0;

        auto root = scene.createChild(layer, "UiRoot");
        auto &rootControl = root.addComponent<ControlComponent>();
        rootControl.anchorLeft = 0.0f;
        rootControl.anchorTop = 0.0f;
        rootControl.anchorRight = 1.0f;
        rootControl.anchorBottom = 1.0f;
        rootControl.offsetLeft = 0.0f;
        rootControl.offsetTop = 0.0f;
        rootControl.offsetRight = 0.0f;
        rootControl.offsetBottom = 0.0f;

        auto background = scene.createChild(root, "Background");
        auto &bgControl = background.addComponent<ControlComponent>();
        bgControl.anchorLeft = 0.0f;
        bgControl.anchorTop = 0.0f;
        bgControl.anchorRight = 1.0f;
        bgControl.anchorBottom = 1.0f;
        bgControl.offsetLeft = 0.0f;
        bgControl.offsetTop = 0.0f;
        bgControl.offsetRight = 0.0f;
        bgControl.offsetBottom = 0.0f;
        auto &bgRect = background.addComponent<ColorRectComponent>();
        bgRect.color[0] = 18;
        bgRect.color[1] = 24;
        bgRect.color[2] = 36;
        bgRect.color[3] = 255;

        auto title = scene.createChild(root, "UiTitle");
        auto &titleControl = title.addComponent<ControlComponent>();
        titleControl.anchorLeft = 0.0f;
        titleControl.anchorTop = 0.0f;
        titleControl.anchorRight = 0.0f;
        titleControl.anchorBottom = 0.0f;
        titleControl.offsetLeft = 24.0f;
        titleControl.offsetTop = 24.0f;
        titleControl.offsetRight = 420.0f;
        titleControl.offsetBottom = 60.0f;
        auto &titleLabel = title.addComponent<LabelComponent>();
        titleLabel.text = "UI Component Showcase";
        titleLabel.fontSize = 26;

        auto panel = scene.createChild(root, "Panel");
        auto &panelControl = panel.addComponent<ControlComponent>();
        panelControl.anchorLeft = 0.0f;
        panelControl.anchorTop = 0.0f;
        panelControl.anchorRight = 0.0f;
        panelControl.anchorBottom = 0.0f;
        panelControl.offsetLeft = 24.0f;
        panelControl.offsetTop = 90.0f;
        panelControl.offsetRight = 380.0f;
        panelControl.offsetBottom = 320.0f;
        auto &panelRect = panel.addComponent<ColorRectComponent>();
        panelRect.color[0] = 28;
        panelRect.color[1] = 34;
        panelRect.color[2] = 46;
        panelRect.color[3] = 220;

        auto label = scene.createChild(panel, "LabelSample");
        auto &labelControl = label.addComponent<ControlComponent>();
        labelControl.anchorLeft = 0.0f;
        labelControl.anchorTop = 0.0f;
        labelControl.anchorRight = 0.0f;
        labelControl.anchorBottom = 0.0f;
        labelControl.offsetLeft = 16.0f;
        labelControl.offsetTop = 14.0f;
        labelControl.offsetRight = 330.0f;
        labelControl.offsetBottom = 40.0f;
        auto &labelComp = label.addComponent<LabelComponent>();
        labelComp.text = "Label: anchored + offset";
        labelComp.fontSize = 18;

        auto colorRect = scene.createChild(panel, "ColorRectSample");
        auto &colorControl = colorRect.addComponent<ControlComponent>();
        colorControl.anchorLeft = 0.0f;
        colorControl.anchorTop = 0.0f;
        colorControl.anchorRight = 0.0f;
        colorControl.anchorBottom = 0.0f;
        colorControl.offsetLeft = 16.0f;
        colorControl.offsetTop = 52.0f;
        colorControl.offsetRight = 150.0f;
        colorControl.offsetBottom = 92.0f;
        auto &colorRectComp = colorRect.addComponent<ColorRectComponent>();
        colorRectComp.color[0] = 80;
        colorRectComp.color[1] = 140;
        colorRectComp.color[2] = 200;
        colorRectComp.color[3] = 230;

        auto textureRect = scene.createChild(panel, "TextureRectSample");
        auto &textureControl = textureRect.addComponent<ControlComponent>();
        textureControl.anchorLeft = 0.0f;
        textureControl.anchorTop = 0.0f;
        textureControl.anchorRight = 0.0f;
        textureControl.anchorBottom = 0.0f;
        textureControl.offsetLeft = 170.0f;
        textureControl.offsetTop = 52.0f;
        textureControl.offsetRight = 330.0f;
        textureControl.offsetBottom = 92.0f;
        auto &textureRectComp = textureRect.addComponent<TextureRectComponent>();
        textureRectComp.texturePath = "";
        textureRectComp.keepAspect = true;

        auto textureLabel = scene.createChild(panel, "TextureLabel");
        auto &textureLabelControl = textureLabel.addComponent<ControlComponent>();
        textureLabelControl.anchorLeft = 0.0f;
        textureLabelControl.anchorTop = 0.0f;
        textureLabelControl.anchorRight = 0.0f;
        textureLabelControl.anchorBottom = 0.0f;
        textureLabelControl.offsetLeft = 170.0f;
        textureLabelControl.offsetTop = 96.0f;
        textureLabelControl.offsetRight = 330.0f;
        textureLabelControl.offsetBottom = 120.0f;
        auto &textureLabelComp = textureLabel.addComponent<LabelComponent>();
        textureLabelComp.text = "TextureRect (empty)";
        textureLabelComp.fontSize = 14;

        auto textEdit = scene.createChild(panel, "TextEditSample");
        auto &textControl = textEdit.addComponent<ControlComponent>();
        textControl.anchorLeft = 0.0f;
        textControl.anchorTop = 0.0f;
        textControl.anchorRight = 0.0f;
        textControl.anchorBottom = 0.0f;
        textControl.offsetLeft = 16.0f;
        textControl.offsetTop = 132.0f;
        textControl.offsetRight = 330.0f;
        textControl.offsetBottom = 170.0f;
        auto &textEditComp = textEdit.addComponent<TextEditComponent>();
        textEditComp.placeholder = "TextEdit (input next)";
        textEditComp.fontSize = 18;

        auto backButton = scene.createChild(root, "BackButton");
        auto &backControl = backButton.addComponent<ControlComponent>();
        backControl.anchorLeft = 1.0f;
        backControl.anchorTop = 0.0f;
        backControl.anchorRight = 1.0f;
        backControl.anchorBottom = 0.0f;
        backControl.offsetLeft = -150.0f;
        backControl.offsetTop = 20.0f;
        backControl.offsetRight = -20.0f;
        backControl.offsetBottom = 52.0f;
        auto &backBtn = backButton.addComponent<ButtonComponent>();
        backBtn.text = "Back";
        backBtn.fontSize = 18;

        ConnectButtonPressed(backButton, [&, menuScene](Entity button)
                             {
                                 (void)button;
                                 if (*backToken)
                                 {
                                     return;
                                 }

                                 *backToken = true;
                                 menuScene->rebuild();
                                 engine.requestSceneChange(menuScene);
                             });

        RegisterUiSystems(scene);
    };

    menuScene->setBuilder(buildMenu);
    uiScene->setBuilder(buildUi);
    menuScene->rebuild();
    uiScene->rebuild();

    engine.run();
    return 0;
}
