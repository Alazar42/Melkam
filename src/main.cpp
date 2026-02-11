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
    EngineConfig config;
    config.width = 1280;
    config.height = 720;
    config.title = "Melkam Engine";
    config.resizable = true;
    config.maximized = false;
    config.fullscreen = false;
    config.borderless = false;
    config.vsync = true;
    config.highDpi = true;

    Engine engine(config);

    SetUiThemeMelkam();

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
        titleLabel.color[0] = 236;
        titleLabel.color[1] = 240;
        titleLabel.color[2] = 248;
        titleLabel.color[3] = 255;

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
        subtitleLabel.color[0] = 186;
        subtitleLabel.color[1] = 194;
        subtitleLabel.color[2] = 210;
        subtitleLabel.color[3] = 255;

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
        titleLabel.color[0] = 236;
        titleLabel.color[1] = 240;
        titleLabel.color[2] = 248;
        titleLabel.color[3] = 255;

        auto contentScroll = scene.createChild(root, "ContentScroll");
        auto &contentScrollControl = contentScroll.addComponent<ControlComponent>();
        contentScrollControl.anchorLeft = 0.0f;
        contentScrollControl.anchorTop = 0.0f;
        contentScrollControl.anchorRight = 1.0f;
        contentScrollControl.anchorBottom = 1.0f;
        contentScrollControl.offsetLeft = 24.0f;
        contentScrollControl.offsetTop = 90.0f;
        contentScrollControl.offsetRight = -24.0f;
        contentScrollControl.offsetBottom = -24.0f;
        contentScroll.addComponent<ScrollContainerComponent>();

        auto scrollContent = scene.createChild(contentScroll, "ScrollContent");
        auto &scrollContentControl = scrollContent.addComponent<ControlComponent>();
        scrollContentControl.anchorLeft = 0.0f;
        scrollContentControl.anchorTop = 0.0f;
        scrollContentControl.anchorRight = 1.0f;
        scrollContentControl.anchorBottom = 1.0f;
        scrollContentControl.offsetLeft = 0.0f;
        scrollContentControl.offsetTop = 0.0f;
        scrollContentControl.offsetRight = 0.0f;
        scrollContentControl.offsetBottom = 0.0f;
        scrollContent.addComponent<HBoxContainerComponent>().spacing = 18.0f;

        auto leftColumn = scene.createChild(scrollContent, "LeftColumn");
        auto &leftControl = leftColumn.addComponent<ControlComponent>();
        leftControl.minSize[0] = 300.0f;
        leftControl.sizeFlagsH = static_cast<std::uint32_t>(UiSizeFlags::Expand);
        leftControl.sizeFlagsV = static_cast<std::uint32_t>(UiSizeFlags::Fill);
        leftColumn.addComponent<VBoxContainerComponent>().spacing = 10.0f;

        auto rightColumn = scene.createChild(scrollContent, "RightColumn");
        auto &rightControl = rightColumn.addComponent<ControlComponent>();
        rightControl.minSize[0] = 360.0f;
        rightControl.sizeFlagsH = static_cast<std::uint32_t>(UiSizeFlags::Expand);
        rightControl.sizeFlagsV = static_cast<std::uint32_t>(UiSizeFlags::Fill);
        rightColumn.addComponent<VBoxContainerComponent>().spacing = 8.0f;

        auto panel = scene.createChild(leftColumn, "Panel");
        auto &panelControl = panel.addComponent<ControlComponent>();
        panelControl.minSize[1] = 230.0f;
        panel.addComponent<PanelComponent>();

        auto panelBody = scene.createChild(panel, "PanelBody");
        auto &panelBodyControl = panelBody.addComponent<ControlComponent>();
        panelBodyControl.anchorLeft = 0.0f;
        panelBodyControl.anchorTop = 0.0f;
        panelBodyControl.anchorRight = 1.0f;
        panelBodyControl.anchorBottom = 1.0f;
        panelBodyControl.offsetLeft = 12.0f;
        panelBodyControl.offsetTop = 12.0f;
        panelBodyControl.offsetRight = -12.0f;
        panelBodyControl.offsetBottom = -12.0f;
        panelBody.addComponent<VBoxContainerComponent>().spacing = 6.0f;

        auto label = scene.createChild(panelBody, "LabelSample");
        auto &labelControl = label.addComponent<ControlComponent>();
        labelControl.minSize[1] = 24.0f;
        auto &labelComp = label.addComponent<LabelComponent>();
        labelComp.text = "Label: anchored + offset";
        labelComp.fontSize = 18;
        labelComp.color[0] = 230;
        labelComp.color[1] = 234;
        labelComp.color[2] = 244;
        labelComp.color[3] = 255;

        auto colorRect = scene.createChild(panelBody, "ColorRectSample");
        auto &colorControl = colorRect.addComponent<ControlComponent>();
        colorControl.minSize[1] = 32.0f;
        auto &colorRectComp = colorRect.addComponent<ColorRectComponent>();
        colorRectComp.color[0] = 80;
        colorRectComp.color[1] = 140;
        colorRectComp.color[2] = 200;
        colorRectComp.color[3] = 230;

        auto lightCard = scene.createChild(panelBody, "LightCard");
        auto &lightControl = lightCard.addComponent<ControlComponent>();
        lightControl.minSize[1] = 34.0f;
        auto &lightRect = lightCard.addComponent<ColorRectComponent>();
        lightRect.color[0] = 232;
        lightRect.color[1] = 236;
        lightRect.color[2] = 242;
        lightRect.color[3] = 255;

        auto lightLabel = scene.createChild(lightCard, "LightCardLabel");
        auto &lightLabelControl = lightLabel.addComponent<ControlComponent>();
        lightLabelControl.anchorLeft = 0.0f;
        lightLabelControl.anchorTop = 0.0f;
        lightLabelControl.anchorRight = 1.0f;
        lightLabelControl.anchorBottom = 1.0f;
        lightLabelControl.offsetLeft = 8.0f;
        lightLabelControl.offsetTop = 6.0f;
        lightLabelControl.offsetRight = -8.0f;
        lightLabelControl.offsetBottom = -6.0f;
        auto &lightLabelComp = lightLabel.addComponent<LabelComponent>();
        lightLabelComp.text = "Light card: black text";
        lightLabelComp.fontSize = 14;
        lightLabelComp.color[0] = 16;
        lightLabelComp.color[1] = 20;
        lightLabelComp.color[2] = 26;
        lightLabelComp.color[3] = 255;

        auto textureRect = scene.createChild(panelBody, "TextureRectSample");
        auto &textureControl = textureRect.addComponent<ControlComponent>();
        textureControl.minSize[1] = 40.0f;
        auto &textureRectComp = textureRect.addComponent<TextureRectComponent>();
        textureRectComp.texturePath = "";
        textureRectComp.keepAspect = true;

        auto textureLabel = scene.createChild(panelBody, "TextureLabel");
        auto &textureLabelControl = textureLabel.addComponent<ControlComponent>();
        textureLabelControl.minSize[1] = 18.0f;
        auto &textureLabelComp = textureLabel.addComponent<LabelComponent>();
        textureLabelComp.text = "TextureRect (empty)";
        textureLabelComp.fontSize = 14;
        textureLabelComp.color[0] = 186;
        textureLabelComp.color[1] = 194;
        textureLabelComp.color[2] = 210;
        textureLabelComp.color[3] = 255;

        auto textEdit = scene.createChild(panelBody, "TextEditSample");
        auto &textControl = textEdit.addComponent<ControlComponent>();
        textControl.minSize[1] = 32.0f;
        auto &textEditComp = textEdit.addComponent<TextEditComponent>();
        textEditComp.placeholder = "TextEdit (input next)";
        textEditComp.fontSize = 18;

        auto labelButton = scene.createChild(rightColumn, "LabelButton");
        auto &labelButtonControl = labelButton.addComponent<ControlComponent>();
        labelButtonControl.minSize[1] = 28.0f;
        labelButton.addComponent<LabelButtonComponent>().text = "LabelButton";

        auto toggle = scene.createChild(rightColumn, "Toggle");
        auto &toggleControl = toggle.addComponent<ControlComponent>();
        toggleControl.minSize[1] = 28.0f;
        toggle.addComponent<ToggleComponent>().text = "Toggle";

        auto toggleGroup = scene.createChild(rightColumn, "ToggleGroup");
        auto &toggleGroupControl = toggleGroup.addComponent<ControlComponent>();
        toggleGroupControl.minSize[1] = 28.0f;
        toggleGroup.addComponent<ToggleGroupComponent>().items = "One;Two;Three";

        auto toggleSlider = scene.createChild(rightColumn, "ToggleSlider");
        auto &toggleSliderControl = toggleSlider.addComponent<ControlComponent>();
        toggleSliderControl.minSize[1] = 28.0f;
        toggleSlider.addComponent<ToggleSliderComponent>().text = "Off;On";

        auto checkBox = scene.createChild(rightColumn, "CheckBox");
        auto &checkBoxControl = checkBox.addComponent<ControlComponent>();
        checkBoxControl.minSize[1] = 28.0f;
        checkBox.addComponent<CheckBoxComponent>().text = "CheckBox";

        auto comboBox = scene.createChild(rightColumn, "ComboBox");
        auto &comboBoxControl = comboBox.addComponent<ControlComponent>();
        comboBoxControl.minSize[1] = 28.0f;
        comboBox.addComponent<ComboBoxComponent>().items = "Low;Medium;High";

        auto dropdown = scene.createChild(rightColumn, "DropdownBox");
        auto &dropdownControl = dropdown.addComponent<ControlComponent>();
        dropdownControl.minSize[1] = 28.0f;
        dropdown.addComponent<DropdownBoxComponent>().items = "Red;Green;Blue";

        auto valueBox = scene.createChild(rightColumn, "ValueBox");
        auto &valueBoxControl = valueBox.addComponent<ControlComponent>();
        valueBoxControl.minSize[1] = 28.0f;
        valueBox.addComponent<ValueBoxComponent>().text = "Value";

        auto spinner = scene.createChild(rightColumn, "Spinner");
        auto &spinnerControl = spinner.addComponent<ControlComponent>();
        spinnerControl.minSize[1] = 28.0f;
        spinner.addComponent<SpinnerComponent>().text = "Spin";

        auto slider = scene.createChild(rightColumn, "Slider");
        auto &sliderControl = slider.addComponent<ControlComponent>();
        sliderControl.minSize[1] = 28.0f;
        auto &sliderComp = slider.addComponent<SliderComponent>();
        sliderComp.textLeft = "Min";
        sliderComp.textRight = "Max";
        sliderComp.value = 0.5f;

        auto sliderBar = scene.createChild(rightColumn, "SliderBar");
        auto &sliderBarControl = sliderBar.addComponent<ControlComponent>();
        sliderBarControl.minSize[1] = 24.0f;
        sliderBar.addComponent<SliderBarComponent>().value = 0.3f;

        auto progress = scene.createChild(rightColumn, "ProgressBar");
        auto &progressControl = progress.addComponent<ControlComponent>();
        progressControl.minSize[1] = 24.0f;
        progress.addComponent<ProgressBarComponent>().value = 0.75f;

        auto status = scene.createChild(rightColumn, "StatusBar");
        auto &statusControl = status.addComponent<ControlComponent>();
        statusControl.minSize[1] = 24.0f;
        status.addComponent<StatusBarComponent>().text = "Status: OK";

        auto dummy = scene.createChild(rightColumn, "DummyRec");
        auto &dummyControl = dummy.addComponent<ControlComponent>();
        dummyControl.minSize[1] = 24.0f;
        dummy.addComponent<DummyRecComponent>().text = "Dummy";

        auto grid = scene.createChild(rightColumn, "Grid");
        auto &gridControl = grid.addComponent<ControlComponent>();
        gridControl.minSize[1] = 60.0f;
        grid.addComponent<GridComponent>();

        auto groupBox = scene.createChild(rightColumn, "GroupBox");
        auto &groupBoxControl = groupBox.addComponent<ControlComponent>();
        groupBoxControl.minSize[1] = 36.0f;
        groupBox.addComponent<GroupBoxComponent>().text = "GroupBox";

        auto line = scene.createChild(rightColumn, "Line");
        auto &lineControl = line.addComponent<ControlComponent>();
        lineControl.minSize[1] = 18.0f;
        line.addComponent<LineComponent>().text = "Line";

        auto scrollPanel = scene.createChild(rightColumn, "ScrollPanel");
        auto &scrollControl = scrollPanel.addComponent<ControlComponent>();
        scrollControl.minSize[1] = 120.0f;
        auto &scrollContainer = scrollPanel.addComponent<ScrollContainerComponent>();
        scrollContainer.contentWidth = 260.0f;
        scrollContainer.contentHeight = 360.0f;

        auto scrollPanelContent = scene.createChild(scrollPanel, "ScrollContent");
        auto &scrollPanelContentControl = scrollPanelContent.addComponent<ControlComponent>();
        scrollPanelContentControl.anchorLeft = 0.0f;
        scrollPanelContentControl.anchorTop = 0.0f;
        scrollPanelContentControl.anchorRight = 1.0f;
        scrollPanelContentControl.anchorBottom = 0.0f;
        scrollPanelContentControl.offsetLeft = 8.0f;
        scrollPanelContentControl.offsetTop = 8.0f;
        scrollPanelContentControl.offsetRight = -8.0f;
        scrollPanelContentControl.offsetBottom = 0.0f;
        scrollPanelContent.addComponent<VBoxContainerComponent>().spacing = 6.0f;

        for (int i = 0; i < 10; ++i)
        {
            auto item = scene.createChild(scrollPanelContent, "ScrollItem");
            auto &itemControl = item.addComponent<ControlComponent>();
            itemControl.minSize[1] = 22.0f;
            auto &itemLabel = item.addComponent<LabelComponent>();
            itemLabel.text = "Scrollable item " + std::to_string(i + 1);
            itemLabel.fontSize = 16;
            itemLabel.color[0] = 220;
            itemLabel.color[1] = 226;
            itemLabel.color[2] = 238;
            itemLabel.color[3] = 255;
        }

        auto tabBar = scene.createChild(rightColumn, "TabBar");
        auto &tabControl = tabBar.addComponent<ControlComponent>();
        tabControl.minSize[1] = 24.0f;
        tabBar.addComponent<TabBarComponent>().items = "Tab A;Tab B;Tab C";

        auto listView = scene.createChild(rightColumn, "ListView");
        auto &listControl = listView.addComponent<ControlComponent>();
        listControl.minSize[1] = 70.0f;
        listView.addComponent<ListViewComponent>().items = "Item 1;Item 2;Item 3;Item 4";

        auto colorPicker = scene.createChild(rightColumn, "ColorPicker");
        auto &colorPickerControl = colorPicker.addComponent<ControlComponent>();
        colorPickerControl.minSize[1] = 80.0f;
        colorPicker.addComponent<ColorPickerComponent>();

        auto messageBox = scene.createChild(rightColumn, "MessageBox");
        auto &messageBoxControl = messageBox.addComponent<ControlComponent>();
        messageBoxControl.minSize[1] = 60.0f;
        auto &messageBoxComp = messageBox.addComponent<MessageBoxComponent>();
        messageBoxComp.title = "Message";
        messageBoxComp.message = "Raygui MessageBox";
        messageBoxComp.buttons = "OK;Cancel";

        auto openMessage = scene.createChild(rightColumn, "OpenMessage");
        auto &openMessageControl = openMessage.addComponent<ControlComponent>();
        openMessageControl.minSize[1] = 28.0f;
        openMessage.addComponent<ButtonComponent>().text = "Open Message";

        ConnectButtonPressed(openMessage, [messageBox](Entity button) mutable
                             {
                                 (void)button;
                                 if (auto *box = messageBox.tryGetComponent<MessageBoxComponent>())
                                 {
                                     box->open = true;
                                 }
                             });

        auto textInput = scene.createChild(rightColumn, "TextInputBox");
        auto &textInputControl = textInput.addComponent<ControlComponent>();
        textInputControl.minSize[1] = 60.0f;
        auto &textInputComp = textInput.addComponent<TextInputBoxComponent>();
        textInputComp.title = "Input";
        textInputComp.message = "Enter text";
        textInputComp.buttons = "OK;Cancel";

        auto openInput = scene.createChild(rightColumn, "OpenInput");
        auto &openInputControl = openInput.addComponent<ControlComponent>();
        openInputControl.minSize[1] = 28.0f;
        openInput.addComponent<ButtonComponent>().text = "Open Input";

        ConnectButtonPressed(openInput, [textInput](Entity button) mutable
                             {
                                 (void)button;
                                 if (auto *box = textInput.tryGetComponent<TextInputBoxComponent>())
                                 {
                                     box->open = true;
                                 }
                             });

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
