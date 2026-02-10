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
}
