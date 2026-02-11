#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include <Melkam/math/Math.hpp>

namespace Melkam
{
    using EntityId = std::uint64_t;
    constexpr EntityId InvalidEntity = 0;

    struct NameComponent
    {
        std::string name;
    };

    struct NodeComponent
    {
        EntityId parent = InvalidEntity;
        std::vector<EntityId> children;
    };

    struct TransformComponent
    {
        Vector3f position = {0.0f, 0.0f, 0.0f};
        Vector3f rotation = {0.0f, 0.0f, 0.0f};
        Vector3f scale = {1.0f, 1.0f, 1.0f};
    };

    struct CameraComponent
    {
        float fov = 60.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
    };

    struct MeshComponent
    {
        std::string meshAsset;
        std::string materialAsset;
    };

    struct RigidBodyComponent
    {
        float mass = 1.0f;
        float velocity[3] = {0.0f, 0.0f, 0.0f};
        bool isKinematic = false;
    };

    struct StaticBodyComponent
    {
    };

    struct CharacterBody2DComponent
    {
        float speed = 5.0f;
        float gravity = 900.0f;
        float jumpStrength = 350.0f;
        bool useGravity = false;
    };

    struct CharacterBody3DComponent
    {
        float speed = 5.0f;
        float jumpStrength = 6.0f;
        float gravity = 20.0f;
        bool useGravity = false;
    };

    struct Velocity2DComponent
    {
        float velocity[2] = {0.0f, 0.0f};
    };

    struct Velocity3DComponent
    {
        float velocity[3] = {0.0f, 0.0f, 0.0f};
    };

    struct Input2DComponent
    {
        float direction[2] = {0.0f, 0.0f};
    };

    struct CharacterController2DComponent
    {
        float acceleration = 18.0f;
        float maxSpeed = 220.0f;
        float damping = 12.0f;
    };

    struct Render2DComponent
    {
        unsigned char color[4] = {255, 255, 255, 255};
    };

    struct CollisionLayerComponent
    {
        std::uint32_t layer = 1;
        std::uint32_t mask = 0xFFFFFFFFu;
    };

    struct ColliderComponent
    {
        bool is2D = true;
        bool isTrigger = false;
        float lastNormal[3] = {0.0f, 0.0f, 0.0f};
        bool onFloor = false;
        bool onWall = false;
        bool onCeiling = false;
    };

    struct Area2DComponent
    {
    };

    struct Area3DComponent
    {
    };

    struct StaticBody2DComponent
    {
    };

    struct StaticBody3DComponent
    {
    };

    struct RigidBody2DComponent
    {
        float mass = 1.0f;
    };

    struct RigidBody3DComponent
    {
        float mass = 1.0f;
    };

    struct BoxShape2DComponent
    {
        float size[2] = {1.0f, 1.0f};
    };

    struct CircleShape2DComponent
    {
        float radius = 0.5f;
    };

    struct CapsuleShape2DComponent
    {
        float radius = 0.5f;
        float height = 1.0f;
    };

    struct BoxShape3DComponent
    {
        float size[3] = {1.0f, 1.0f, 1.0f};
    };

    struct SphereShape3DComponent
    {
        float radius = 0.5f;
    };

    struct CapsuleShape3DComponent
    {
        float radius = 0.5f;
        float height = 1.0f;
    };

    struct CylinderShape3DComponent
    {
        float radius = 0.5f;
        float height = 1.0f;
    };

    struct CollisionMeshComponent
    {
        std::string meshAsset;
        bool convex = true;
    };

    enum class UiSizeFlags : std::uint32_t
    {
        None = 0,
        Fill = 1u << 0,
        Expand = 1u << 1
    };

    struct CanvasLayerComponent
    {
        int layer = 0;
    };

    struct ControlComponent
    {
        float anchorLeft = 0.0f;
        float anchorTop = 0.0f;
        float anchorRight = 0.0f;
        float anchorBottom = 0.0f;

        float offsetLeft = 0.0f;
        float offsetTop = 0.0f;
        float offsetRight = 100.0f;
        float offsetBottom = 30.0f;

        float minSize[2] = {0.0f, 0.0f};
        std::uint32_t sizeFlagsH = static_cast<std::uint32_t>(UiSizeFlags::None);
        std::uint32_t sizeFlagsV = static_cast<std::uint32_t>(UiSizeFlags::None);

        float rectX = 0.0f;
        float rectY = 0.0f;
        float rectW = 0.0f;
        float rectH = 0.0f;

        bool visible = true;
    };

    struct VBoxContainerComponent
    {
        float padding = 8.0f;
        float spacing = 6.0f;
    };

    struct HBoxContainerComponent
    {
        float padding = 8.0f;
        float spacing = 10.0f;
    };

    struct ScrollContainerComponent
    {
        float scrollX = 0.0f;
        float scrollY = 0.0f;
        float contentWidth = 0.0f;
        float contentHeight = 0.0f;
        float wheelSpeed = 24.0f;
    };

    struct LabelComponent
    {
        std::string text;
        int fontSize = 20;
        unsigned char color[4] = {255, 255, 255, 255};
    };

    struct ColorRectComponent
    {
        unsigned char color[4] = {40, 40, 40, 200};
    };

    struct TextureRectComponent
    {
        std::string texturePath;
        bool keepAspect = true;
        unsigned char tint[4] = {255, 255, 255, 255};
    };

    struct TextEditComponent
    {
        std::string text;
        std::string placeholder;
        int fontSize = 20;
        int maxLength = 64;
        unsigned char color[4] = {255, 255, 255, 255};
        unsigned char background[4] = {20, 20, 20, 200};
        bool readOnly = false;
    };

    struct UiStyleComponent
    {
        std::string stylePath;
        bool useDefault = false;
    };

    struct LabelButtonComponent
    {
        std::string text;
    };

    struct ToggleComponent
    {
        std::string text;
        bool active = false;
    };

    struct ToggleGroupComponent
    {
        std::string items;
        int active = 0;
    };

    struct ToggleSliderComponent
    {
        std::string text;
        int active = 0;
    };

    struct CheckBoxComponent
    {
        std::string text;
        bool checked = false;
    };

    struct ComboBoxComponent
    {
        std::string items;
        int active = 0;
    };

    struct DropdownBoxComponent
    {
        std::string items;
        int active = 0;
        bool editMode = false;
    };

    struct ValueBoxComponent
    {
        std::string text;
        int value = 0;
        int minValue = 0;
        int maxValue = 100;
        bool editMode = false;
    };

    struct SpinnerComponent
    {
        std::string text;
        int value = 0;
        int minValue = 0;
        int maxValue = 100;
        bool editMode = false;
    };

    struct SliderComponent
    {
        std::string textLeft;
        std::string textRight;
        float value = 0.0f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
    };

    struct SliderBarComponent
    {
        std::string textLeft;
        std::string textRight;
        float value = 0.0f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
    };

    struct ProgressBarComponent
    {
        std::string textLeft;
        std::string textRight;
        float value = 0.0f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
    };

    struct StatusBarComponent
    {
        std::string text;
    };

    struct DummyRecComponent
    {
        std::string text;
    };

    struct GridComponent
    {
        float spacing = 20.0f;
        int subdivs = 5;
        float mouseCell[2] = {0.0f, 0.0f};
    };

    struct WindowBoxComponent
    {
        std::string title;
        bool open = true;
    };

    struct GroupBoxComponent
    {
        std::string text;
    };

    struct LineComponent
    {
        std::string text;
    };

    struct PanelComponent
    {
    };

    struct ScrollPanelComponent
    {
        float contentWidth = 0.0f;
        float contentHeight = 0.0f;
        float scrollX = 0.0f;
        float scrollY = 0.0f;
        float viewX = 0.0f;
        float viewY = 0.0f;
        float viewW = 0.0f;
        float viewH = 0.0f;
    };

    struct TabBarComponent
    {
        std::string items;
        int active = 0;
        int scrollIndex = 0;
    };

    struct ListViewComponent
    {
        std::string items;
        int active = 0;
        int scrollIndex = 0;
    };

    struct ColorPickerComponent
    {
        unsigned char color[4] = {255, 255, 255, 255};
    };

    struct MessageBoxComponent
    {
        std::string title;
        std::string message;
        std::string buttons;
        int result = -1;
        bool open = false;
    };

    struct TextInputBoxComponent
    {
        std::string title;
        std::string message;
        std::string buttons;
        std::string text;
        int maxLength = 64;
        int result = -1;
        bool open = false;
        bool secretView = false;
    };

    struct ButtonComponent
    {
        std::string text;
        int fontSize = 20;
        unsigned char textColor[4] = {255, 255, 255, 255};
        unsigned char normalColor[4] = {50, 90, 140, 220};
        unsigned char hoverColor[4] = {70, 120, 180, 230};
        unsigned char pressedColor[4] = {40, 70, 110, 240};
        bool hovered = false;
        bool pressed = false;
        bool disabled = false;
    };
}
