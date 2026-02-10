#include <Melkam/physics/Collider.hpp>

#include <Melkam/scene/Components.hpp>
#include <Melkam/scene/Entity.hpp>
#include <Melkam/scene/Scene.hpp>
#include <Melkam/scene/System.hpp>

#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace Melkam
{
    namespace
    {
        struct Aabb2D
        {
            float minX;
            float minY;
            float maxX;
            float maxY;
        };

        struct Aabb3D
        {
            float minX;
            float minY;
            float minZ;
            float maxX;
            float maxY;
            float maxZ;
        };

        struct SlideSettings
        {
            float epsilon = 0.001f;
            int maxSlides = 4;
            float floorDot = 0.7f;
        };

        SlideSettings s_settings;

        bool intersects(const Aabb2D &a, const Aabb2D &b)
        {
            return a.minX < b.maxX && a.maxX > b.minX && a.minY < b.maxY && a.maxY > b.minY;
        }

        bool intersects(const Aabb3D &a, const Aabb3D &b)
        {
            return a.minX < b.maxX && a.maxX > b.minX && a.minY < b.maxY && a.maxY > b.minY && a.minZ < b.maxZ && a.maxZ > b.minZ;
        }

        bool getAabb2D(const Entity &entity, const TransformComponent &transform, Aabb2D &out)
        {
            if (const auto *box = entity.tryGetComponent<BoxShape2DComponent>())
            {
                const float halfX = box->size[0] * 0.5f;
                const float halfY = box->size[1] * 0.5f;
                out.minX = transform.position.x - halfX;
                out.maxX = transform.position.x + halfX;
                out.minY = transform.position.y - halfY;
                out.maxY = transform.position.y + halfY;
                return true;
            }

            if (const auto *circle = entity.tryGetComponent<CircleShape2DComponent>())
            {
                const float r = circle->radius;
                out.minX = transform.position.x - r;
                out.maxX = transform.position.x + r;
                out.minY = transform.position.y - r;
                out.maxY = transform.position.y + r;
                return true;
            }

            return false;
        }

        bool getAabb3D(const Entity &entity, const TransformComponent &transform, Aabb3D &out)
        {
            if (const auto *box = entity.tryGetComponent<BoxShape3DComponent>())
            {
                const float halfX = box->size[0] * 0.5f;
                const float halfY = box->size[1] * 0.5f;
                const float halfZ = box->size[2] * 0.5f;
                out.minX = transform.position.x - halfX;
                out.maxX = transform.position.x + halfX;
                out.minY = transform.position.y - halfY;
                out.maxY = transform.position.y + halfY;
                out.minZ = transform.position.z - halfZ;
                out.maxZ = transform.position.z + halfZ;
                return true;
            }

            if (const auto *sphere = entity.tryGetComponent<SphereShape3DComponent>())
            {
                const float r = sphere->radius;
                out.minX = transform.position.x - r;
                out.maxX = transform.position.x + r;
                out.minY = transform.position.y - r;
                out.maxY = transform.position.y + r;
                out.minZ = transform.position.z - r;
                out.maxZ = transform.position.z + r;
                return true;
            }

            return false;
        }

        bool shouldCollide(const CollisionLayerComponent *a, const CollisionLayerComponent *b)
        {
            const std::uint32_t aLayer = a ? a->layer : 1u;
            const std::uint32_t aMask = a ? a->mask : 0xFFFFFFFFu;
            const std::uint32_t bLayer = b ? b->layer : 1u;
            const std::uint32_t bMask = b ? b->mask : 0xFFFFFFFFu;
            return (aMask & bLayer) != 0u && (bMask & aLayer) != 0u;
        }

        void clearContactState(ColliderComponent &collider)
        {
            collider.lastNormal[0] = 0.0f;
            collider.lastNormal[1] = 0.0f;
            collider.lastNormal[2] = 0.0f;
            collider.onFloor = false;
            collider.onWall = false;
            collider.onCeiling = false;
        }

        void updateContactState(ColliderComponent &collider, float nx, float ny, float nz, bool is2D)
        {
            collider.lastNormal[0] = nx;
            collider.lastNormal[1] = ny;
            collider.lastNormal[2] = nz;

            if (is2D)
            {
                collider.onFloor = ny <= -s_settings.floorDot;
                collider.onCeiling = ny >= s_settings.floorDot;
                collider.onWall = std::abs(nx) >= s_settings.floorDot;
            }
            else
            {
                collider.onFloor = ny >= s_settings.floorDot;
                collider.onCeiling = ny <= -s_settings.floorDot;
                collider.onWall = std::abs(nx) >= s_settings.floorDot || std::abs(nz) >= s_settings.floorDot;
            }
        }

        bool overlapNormal2D(const Aabb2D &a, const Aabb2D &b, float &outNx, float &outNy)
        {
            if (!intersects(a, b))
            {
                return false;
            }

            const float overlapX1 = b.maxX - a.minX;
            const float overlapX2 = a.maxX - b.minX;
            const float resolveX = (overlapX1 < overlapX2) ? -overlapX1 : overlapX2;

            const float overlapY1 = b.maxY - a.minY;
            const float overlapY2 = a.maxY - b.minY;
            const float resolveY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;

            if (std::abs(resolveX) < std::abs(resolveY))
            {
                outNx = (resolveX < 0.0f) ? -1.0f : 1.0f;
                outNy = 0.0f;
            }
            else
            {
                outNx = 0.0f;
                outNy = (resolveY < 0.0f) ? -1.0f : 1.0f;
            }
            return true;
        }

        bool overlapNormal3D(const Aabb3D &a, const Aabb3D &b, float &outNx, float &outNy, float &outNz)
        {
            if (!intersects(a, b))
            {
                return false;
            }

            const float overlapX1 = b.maxX - a.minX;
            const float overlapX2 = a.maxX - b.minX;
            const float resolveX = (overlapX1 < overlapX2) ? -overlapX1 : overlapX2;

            const float overlapY1 = b.maxY - a.minY;
            const float overlapY2 = a.maxY - b.minY;
            const float resolveY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;

            const float overlapZ1 = b.maxZ - a.minZ;
            const float overlapZ2 = a.maxZ - b.minZ;
            const float resolveZ = (overlapZ1 < overlapZ2) ? -overlapZ1 : overlapZ2;

            float absX = std::abs(resolveX);
            float absY = std::abs(resolveY);
            float absZ = std::abs(resolveZ);

            if (absX <= absY && absX <= absZ)
            {
                outNx = (resolveX < 0.0f) ? -1.0f : 1.0f;
                outNy = 0.0f;
                outNz = 0.0f;
            }
            else if (absY <= absX && absY <= absZ)
            {
                outNx = 0.0f;
                outNy = (resolveY < 0.0f) ? -1.0f : 1.0f;
                outNz = 0.0f;
            }
            else
            {
                outNx = 0.0f;
                outNy = 0.0f;
                outNz = (resolveZ < 0.0f) ? -1.0f : 1.0f;
            }
            return true;
        }

        bool sweepAabb2D(const Aabb2D &mover, const Aabb2D &target, float dx, float dy, float &outTime, float &outNx, float &outNy)
        {
            const float inf = std::numeric_limits<float>::infinity();
            if (dx == 0.0f && dy == 0.0f)
            {
                return false;
            }

            if (dx == 0.0f && (mover.maxX <= target.minX || mover.minX >= target.maxX))
            {
                return false;
            }

            if (dy == 0.0f && (mover.maxY <= target.minY || mover.minY >= target.maxY))
            {
                return false;
            }

            if (overlapNormal2D(mover, target, outNx, outNy))
            {
                outTime = 0.0f;
                return true;
            }

            float xInvEntry;
            float xInvExit;
            float yInvEntry;
            float yInvExit;

            if (dx > 0.0f)
            {
                xInvEntry = target.minX - mover.maxX;
                xInvExit = target.maxX - mover.minX;
            }
            else
            {
                xInvEntry = target.maxX - mover.minX;
                xInvExit = target.minX - mover.maxX;
            }

            if (dy > 0.0f)
            {
                yInvEntry = target.minY - mover.maxY;
                yInvExit = target.maxY - mover.minY;
            }
            else
            {
                yInvEntry = target.maxY - mover.minY;
                yInvExit = target.minY - mover.maxY;
            }

            const float xEntry = (dx == 0.0f) ? -inf : xInvEntry / dx;
            const float xExit = (dx == 0.0f) ? inf : xInvExit / dx;
            const float yEntry = (dy == 0.0f) ? -inf : yInvEntry / dy;
            const float yExit = (dy == 0.0f) ? inf : yInvExit / dy;

            const float entryTime = std::max(xEntry, yEntry);
            const float exitTime = std::min(xExit, yExit);

            if (entryTime > exitTime || entryTime > 1.0f || entryTime < 0.0f)
            {
                return false;
            }

            if (xEntry > yEntry)
            {
                outNx = (dx > 0.0f) ? -1.0f : 1.0f;
                outNy = 0.0f;
            }
            else
            {
                outNx = 0.0f;
                outNy = (dy > 0.0f) ? -1.0f : 1.0f;
            }

            outTime = entryTime;
            return true;
        }

        bool sweepAabb3D(const Aabb3D &mover, const Aabb3D &target, float dx, float dy, float dz, float &outTime, float &outNx, float &outNy, float &outNz)
        {
            const float inf = std::numeric_limits<float>::infinity();
            if (dx == 0.0f && dy == 0.0f && dz == 0.0f)
            {
                return false;
            }

            if (dx == 0.0f && (mover.maxX <= target.minX || mover.minX >= target.maxX))
            {
                return false;
            }

            if (dy == 0.0f && (mover.maxY <= target.minY || mover.minY >= target.maxY))
            {
                return false;
            }

            if (dz == 0.0f && (mover.maxZ <= target.minZ || mover.minZ >= target.maxZ))
            {
                return false;
            }

            if (overlapNormal3D(mover, target, outNx, outNy, outNz))
            {
                outTime = 0.0f;
                return true;
            }

            float xInvEntry;
            float xInvExit;
            float yInvEntry;
            float yInvExit;
            float zInvEntry;
            float zInvExit;

            if (dx > 0.0f)
            {
                xInvEntry = target.minX - mover.maxX;
                xInvExit = target.maxX - mover.minX;
            }
            else
            {
                xInvEntry = target.maxX - mover.minX;
                xInvExit = target.minX - mover.maxX;
            }

            if (dy > 0.0f)
            {
                yInvEntry = target.minY - mover.maxY;
                yInvExit = target.maxY - mover.minY;
            }
            else
            {
                yInvEntry = target.maxY - mover.minY;
                yInvExit = target.minY - mover.maxY;
            }

            if (dz > 0.0f)
            {
                zInvEntry = target.minZ - mover.maxZ;
                zInvExit = target.maxZ - mover.minZ;
            }
            else
            {
                zInvEntry = target.maxZ - mover.minZ;
                zInvExit = target.minZ - mover.maxZ;
            }

            const float xEntry = (dx == 0.0f) ? -inf : xInvEntry / dx;
            const float xExit = (dx == 0.0f) ? inf : xInvExit / dx;
            const float yEntry = (dy == 0.0f) ? -inf : yInvEntry / dy;
            const float yExit = (dy == 0.0f) ? inf : yInvExit / dy;
            const float zEntry = (dz == 0.0f) ? -inf : zInvEntry / dz;
            const float zExit = (dz == 0.0f) ? inf : zInvExit / dz;

            const float entryTime = std::max(xEntry, std::max(yEntry, zEntry));
            const float exitTime = std::min(xExit, std::min(yExit, zExit));

            if (entryTime > exitTime || entryTime > 1.0f || entryTime < 0.0f)
            {
                return false;
            }

            if (xEntry >= yEntry && xEntry >= zEntry)
            {
                outNx = (dx > 0.0f) ? -1.0f : 1.0f;
                outNy = 0.0f;
                outNz = 0.0f;
            }
            else if (yEntry >= xEntry && yEntry >= zEntry)
            {
                outNx = 0.0f;
                outNy = (dy > 0.0f) ? -1.0f : 1.0f;
                outNz = 0.0f;
            }
            else
            {
                outNx = 0.0f;
                outNy = 0.0f;
                outNz = (dz > 0.0f) ? -1.0f : 1.0f;
            }

            outTime = entryTime;
            return true;
        }

        class Render3DSystem : public System
        {
        public:
            void onUpdate(Scene &scene, float dt) override
            {
                (void)dt;
                static bool initialized = false;
                static Shader shader = {};
                static Model cubeModel = {};
                static Model sphereModel = {};

                if (!initialized)
                {
                    const char *vs =
                        "#version 330\n"
                        "in vec3 vertexPosition;\n"
                        "in vec3 vertexNormal;\n"
                        "in vec4 vertexColor;\n"
                        "in vec2 vertexTexCoord;\n"
                        "uniform mat4 mvp;\n"
                        "uniform mat4 matModel;\n"
                        "out vec3 fragPos;\n"
                        "out vec3 fragNormal;\n"
                        "out vec4 fragColor;\n"
                        "void main() {\n"
                        "    fragPos = vec3(matModel * vec4(vertexPosition, 1.0));\n"
                        "    fragNormal = mat3(transpose(inverse(matModel))) * vertexNormal;\n"
                        "    fragColor = vertexColor;\n"
                        "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
                        "}\n";

                    const char *fs =
                        "#version 330\n"
                        "in vec3 fragPos;\n"
                        "in vec3 fragNormal;\n"
                        "in vec4 fragColor;\n"
                        "out vec4 finalColor;\n"
                        "uniform vec4 colDiffuse;\n"
                        "uniform vec3 lightDir;\n"
                        "uniform vec3 lightColor;\n"
                        "uniform vec3 ambientColor;\n"
                        "uniform vec3 viewPos;\n"
                        "void main() {\n"
                        "    vec3 norm = normalize(fragNormal);\n"
                        "    float diff = max(dot(norm, -lightDir), 0.0);\n"
                        "    vec3 color = ambientColor + lightColor * diff;\n"
                        "    finalColor = vec4(color, 1.0) * fragColor * colDiffuse;\n"
                        "}\n";

                    shader = LoadShaderFromMemory(vs, fs);
                    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
                    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
                    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader, "colDiffuse");
                    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
                    initialized = true;

                    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
                    cubeModel = LoadModelFromMesh(cubeMesh);
                    cubeModel.materials[0].shader = shader;

                    Mesh sphereMesh = GenMeshSphere(1.0f, 24, 24);
                    sphereModel = LoadModelFromMesh(sphereMesh);
                    sphereModel.materials[0].shader = shader;
                }

                Camera3D camera = {};
                camera.position = {0.0f, 6.0f, 12.0f};
                camera.target = {0.0f, 1.0f, 0.0f};
                camera.up = {0.0f, 1.0f, 0.0f};
                camera.fovy = 60.0f;
                camera.projection = CAMERA_PERSPECTIVE;

                for (auto &entity : scene.view<TransformComponent, CameraComponent>())
                {
                    auto *transform = entity.tryGetComponent<TransformComponent>();
                    auto *cameraComponent = entity.tryGetComponent<CameraComponent>();
                    if (!transform || !cameraComponent)
                    {
                        continue;
                    }

                    camera.position = {transform->position.x, transform->position.y, transform->position.z};
                    camera.fovy = cameraComponent->fov;
                    break;
                }

                for (auto &entity : scene.view<TransformComponent, CharacterBody3DComponent>())
                {
                    auto *transform = entity.tryGetComponent<TransformComponent>();
                    if (!transform)
                    {
                        continue;
                    }
                    camera.target = {transform->position.x, transform->position.y, transform->position.z};
                    break;
                }

                const Vector3 lightDir = Vector3Normalize({-0.6f, -1.0f, -0.4f});
                const Vector3 lightColor = {1.0f, 1.0f, 1.0f};
                const Vector3 ambient = {0.2f, 0.2f, 0.2f};

                BeginDrawing();
                ClearBackground({18, 24, 36, 255});
                BeginMode3D(camera);

                SetShaderValue(shader, GetShaderLocation(shader, "lightDir"), &lightDir, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, GetShaderLocation(shader, "lightColor"), &lightColor, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, GetShaderLocation(shader, "ambientColor"), &ambient, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position, SHADER_UNIFORM_VEC3);

                BeginShaderMode(shader);

                for (auto &entity : scene.view<TransformComponent, BoxShape3DComponent>())
                {
                    auto *transform = entity.tryGetComponent<TransformComponent>();
                    auto *shape = entity.tryGetComponent<BoxShape3DComponent>();
                    if (!transform || !shape)
                    {
                        continue;
                    }

                    Color drawColor = RAYWHITE;
                    if (auto *render = entity.tryGetComponent<Render2DComponent>())
                    {
                        drawColor = {render->color[0], render->color[1], render->color[2], render->color[3]};
                    }

                    Vector3 position = {transform->position.x, transform->position.y, transform->position.z};
                    Vector3 scale = {shape->size[0], shape->size[1], shape->size[2]};
                    DrawModelEx(cubeModel, position, {0.0f, 1.0f, 0.0f}, 0.0f, scale, drawColor);
                }

                for (auto &entity : scene.view<TransformComponent, SphereShape3DComponent>())
                {
                    auto *transform = entity.tryGetComponent<TransformComponent>();
                    auto *shape = entity.tryGetComponent<SphereShape3DComponent>();
                    if (!transform || !shape)
                    {
                        continue;
                    }

                    Color drawColor = RAYWHITE;
                    if (auto *render = entity.tryGetComponent<Render2DComponent>())
                    {
                        drawColor = {render->color[0], render->color[1], render->color[2], render->color[3]};
                    }

                    Vector3 position = {transform->position.x, transform->position.y, transform->position.z};
                    Vector3 scale = {shape->radius, shape->radius, shape->radius};
                    DrawModelEx(sphereModel, position, {0.0f, 1.0f, 0.0f}, 0.0f, scale, drawColor);
                }

                EndShaderMode();
                DrawGrid(20, 1.0f);
                EndMode3D();
                EndDrawing();
            }
        };
    }

    void RegisterColliderSystems(Scene &scene)
    {
        scene.createSystem<Render3DSystem>();
    }

    void SetSlideSettings(float epsilon, int maxSlides)
    {
        s_settings.epsilon = std::max(0.00001f, epsilon);
        s_settings.maxSlides = std::max(1, maxSlides);
    }

    bool MoveAndSlide2D(Entity &entity, float dt)
    {
        auto *scene = entity.scene();
        auto *transform = entity.tryGetComponent<TransformComponent>();
        auto *velocity = entity.tryGetComponent<Velocity2DComponent>();
        auto *collider = entity.tryGetComponent<ColliderComponent>();
        if (!scene || !transform || !velocity || !collider || !collider->is2D || dt <= 0.0f)
        {
            return false;
        }

        clearContactState(*collider);

        const auto all = scene->view<TransformComponent, ColliderComponent>();
        bool moved = false;
        float remaining = 1.0f;
        float vx = velocity->velocity[0];
        float vy = velocity->velocity[1];

        for (int iter = 0; iter < s_settings.maxSlides; ++iter)
        {
            const float dx = vx * dt * remaining;
            const float dy = vy * dt * remaining;
            if (std::abs(dx) <= s_settings.epsilon && std::abs(dy) <= s_settings.epsilon)
            {
                break;
            }

            Aabb2D moverBox;
            if (!getAabb2D(entity, *transform, moverBox))
            {
                return false;
            }

            float bestTime = 1.0f;
            float hitNx = 0.0f;
            float hitNy = 0.0f;
            bool hit = false;
            Aabb2D hitBox{};

            for (auto &other : all)
            {
                if (other.id() == entity.id())
                {
                    continue;
                }

                auto *otherCollider = other.tryGetComponent<ColliderComponent>();
                auto *otherTransform = other.tryGetComponent<TransformComponent>();
                if (!otherCollider || !otherTransform || !otherCollider->is2D)
                {
                    continue;
                }

                if (!shouldCollide(entity.tryGetComponent<CollisionLayerComponent>(), other.tryGetComponent<CollisionLayerComponent>()))
                {
                    continue;
                }

                Aabb2D otherBox;
                if (!getAabb2D(other, *otherTransform, otherBox))
                {
                    continue;
                }

                float time = 0.0f;
                float nx = 0.0f;
                float ny = 0.0f;
                if (!sweepAabb2D(moverBox, otherBox, dx, dy, time, nx, ny))
                {
                    continue;
                }

                if (time < bestTime)
                {
                    bestTime = time;
                    hitNx = nx;
                    hitNy = ny;
                    hit = true;
                    hitBox = otherBox;
                }
            }

            if (!hit)
            {
                transform->position.x += dx;
                transform->position.y += dy;
                moved = true;
                break;
            }

            transform->position.x += dx * bestTime;
            transform->position.y += dy * bestTime;

            if (bestTime > 0.0f)
            {
                transform->position.x += hitNx * s_settings.epsilon;
                transform->position.y += hitNy * s_settings.epsilon;
            }
            else
            {
                Aabb2D moverBox;
                if (getAabb2D(entity, *transform, moverBox) && intersects(moverBox, hitBox))
                {
                    const float overlap1 = hitBox.maxX - moverBox.minX;
                    const float overlap2 = moverBox.maxX - hitBox.minX;
                    const float resolveX = (overlap1 < overlap2) ? -overlap1 : overlap2;

                    const float overlapY1 = hitBox.maxY - moverBox.minY;
                    const float overlapY2 = moverBox.maxY - hitBox.minY;
                    const float resolveY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;

                    if (std::abs(resolveX) < std::abs(resolveY))
                    {
                        transform->position.x += resolveX;
                        hitNx = (resolveX < 0.0f) ? -1.0f : 1.0f;
                        hitNy = 0.0f;
                    }
                    else
                    {
                        transform->position.y += resolveY;
                        hitNx = 0.0f;
                        hitNy = (resolveY < 0.0f) ? -1.0f : 1.0f;
                    }
                }
            }
            moved = true;

            updateContactState(*collider, hitNx, hitNy, 0.0f, true);

            const float dot = vx * hitNx + vy * hitNy;
            vx = vx - hitNx * dot;
            vy = vy - hitNy * dot;
            remaining *= (1.0f - bestTime);

            if (remaining <= s_settings.epsilon)
            {
                break;
            }
        }

        velocity->velocity[0] = vx;
        velocity->velocity[1] = vy;
        return moved;
    }

    bool MoveAndSlide3D(Entity &entity, float dt)
    {
        auto *scene = entity.scene();
        auto *transform = entity.tryGetComponent<TransformComponent>();
        auto *velocity = entity.tryGetComponent<Velocity3DComponent>();
        auto *collider = entity.tryGetComponent<ColliderComponent>();
        if (!scene || !transform || !velocity || !collider || collider->is2D || dt <= 0.0f)
        {
            return false;
        }

        clearContactState(*collider);

        const auto all = scene->view<TransformComponent, ColliderComponent>();
        bool moved = false;
        float remaining = 1.0f;
        float vx = velocity->velocity[0];
        float vy = velocity->velocity[1];
        float vz = velocity->velocity[2];

        for (int iter = 0; iter < s_settings.maxSlides; ++iter)
        {
            const float dx = vx * dt * remaining;
            const float dy = vy * dt * remaining;
            const float dz = vz * dt * remaining;
            if (std::abs(dx) <= s_settings.epsilon && std::abs(dy) <= s_settings.epsilon && std::abs(dz) <= s_settings.epsilon)
            {
                break;
            }

            Aabb3D moverBox;
            if (!getAabb3D(entity, *transform, moverBox))
            {
                return false;
            }

            float bestTime = 1.0f;
            float hitNx = 0.0f;
            float hitNy = 0.0f;
            float hitNz = 0.0f;
            bool hit = false;
            Aabb3D hitBox{};

            for (auto &other : all)
            {
                if (other.id() == entity.id())
                {
                    continue;
                }

                auto *otherCollider = other.tryGetComponent<ColliderComponent>();
                auto *otherTransform = other.tryGetComponent<TransformComponent>();
                if (!otherCollider || !otherTransform || otherCollider->is2D)
                {
                    continue;
                }

                if (!shouldCollide(entity.tryGetComponent<CollisionLayerComponent>(), other.tryGetComponent<CollisionLayerComponent>()))
                {
                    continue;
                }

                Aabb3D otherBox;
                if (!getAabb3D(other, *otherTransform, otherBox))
                {
                    continue;
                }

                float time = 0.0f;
                float nx = 0.0f;
                float ny = 0.0f;
                float nz = 0.0f;
                if (!sweepAabb3D(moverBox, otherBox, dx, dy, dz, time, nx, ny, nz))
                {
                    continue;
                }

                if (time < bestTime)
                {
                    bestTime = time;
                    hitNx = nx;
                    hitNy = ny;
                    hitNz = nz;
                    hit = true;
                    hitBox = otherBox;
                }
            }

            if (!hit)
            {
                transform->position.x += dx;
                transform->position.y += dy;
                transform->position.z += dz;
                moved = true;
                break;
            }

            transform->position.x += dx * bestTime;
            transform->position.y += dy * bestTime;
            transform->position.z += dz * bestTime;

            if (bestTime > 0.0f)
            {
                transform->position.x += hitNx * s_settings.epsilon;
                transform->position.y += hitNy * s_settings.epsilon;
                transform->position.z += hitNz * s_settings.epsilon;
            }
            else
            {
                Aabb3D moverBox;
                if (getAabb3D(entity, *transform, moverBox) && intersects(moverBox, hitBox))
                {
                    const float overlapX1 = hitBox.maxX - moverBox.minX;
                    const float overlapX2 = moverBox.maxX - hitBox.minX;
                    const float resolveX = (overlapX1 < overlapX2) ? -overlapX1 : overlapX2;

                    const float overlapY1 = hitBox.maxY - moverBox.minY;
                    const float overlapY2 = moverBox.maxY - hitBox.minY;
                    const float resolveY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;

                    const float overlapZ1 = hitBox.maxZ - moverBox.minZ;
                    const float overlapZ2 = moverBox.maxZ - hitBox.minZ;
                    const float resolveZ = (overlapZ1 < overlapZ2) ? -overlapZ1 : overlapZ2;

                    float absX = std::abs(resolveX);
                    float absY = std::abs(resolveY);
                    float absZ = std::abs(resolveZ);

                    if (absX <= absY && absX <= absZ)
                    {
                        transform->position.x += resolveX;
                        hitNx = (resolveX < 0.0f) ? -1.0f : 1.0f;
                        hitNy = 0.0f;
                        hitNz = 0.0f;
                    }
                    else if (absY <= absX && absY <= absZ)
                    {
                        transform->position.y += resolveY;
                        hitNx = 0.0f;
                        hitNy = (resolveY < 0.0f) ? -1.0f : 1.0f;
                        hitNz = 0.0f;
                    }
                    else
                    {
                        transform->position.z += resolveZ;
                        hitNx = 0.0f;
                        hitNy = 0.0f;
                        hitNz = (resolveZ < 0.0f) ? -1.0f : 1.0f;
                    }
                }
            }
            moved = true;

            updateContactState(*collider, hitNx, hitNy, hitNz, false);

            const float dot = vx * hitNx + vy * hitNy + vz * hitNz;
            vx = vx - hitNx * dot;
            vy = vy - hitNy * dot;
            vz = vz - hitNz * dot;
            remaining *= (1.0f - bestTime);

            if (remaining <= s_settings.epsilon)
            {
                break;
            }
        }

        velocity->velocity[0] = vx;
        velocity->velocity[1] = vy;
        velocity->velocity[2] = vz;
        return moved;
    }

    bool MoveAndCollide2D(Entity &entity, const float motion[2], float dt)
    {
        CollisionInfo info;
        return MoveAndCollide2D(entity, motion, dt, info);
    }

    bool MoveAndCollide3D(Entity &entity, const float motion[3], float dt)
    {
        CollisionInfo info;
        return MoveAndCollide3D(entity, motion, dt, info);
    }

    bool MoveAndCollide2D(Entity &entity, const float motion[2], float dt, CollisionInfo &outInfo)
    {
        outInfo = CollisionInfo{};

        auto *scene = entity.scene();
        auto *transform = entity.tryGetComponent<TransformComponent>();
        auto *collider = entity.tryGetComponent<ColliderComponent>();
        if (!scene || !transform || !collider || !collider->is2D || dt <= 0.0f || !motion)
        {
            return false;
        }

        clearContactState(*collider);

        const float dx = motion[0];
        const float dy = motion[1];
        const auto all = scene->view<TransformComponent, ColliderComponent>();

        Aabb2D moverBox;
        if (!getAabb2D(entity, *transform, moverBox))
        {
            return false;
        }

        float bestTime = 1.0f;
        float hitNx = 0.0f;
        float hitNy = 0.0f;
        EntityId hitEntity = InvalidEntity;
        Aabb2D hitBox{};

        for (auto &other : all)
        {
            if (other.id() == entity.id())
            {
                continue;
            }

            auto *otherCollider = other.tryGetComponent<ColliderComponent>();
            auto *otherTransform = other.tryGetComponent<TransformComponent>();
            if (!otherCollider || !otherTransform || !otherCollider->is2D)
            {
                continue;
            }

            if (!shouldCollide(entity.tryGetComponent<CollisionLayerComponent>(), other.tryGetComponent<CollisionLayerComponent>()))
            {
                continue;
            }

            Aabb2D otherBox;
            if (!getAabb2D(other, *otherTransform, otherBox))
            {
                continue;
            }

            float time = 0.0f;
            float nx = 0.0f;
            float ny = 0.0f;
            if (!sweepAabb2D(moverBox, otherBox, dx, dy, time, nx, ny))
            {
                continue;
            }

            if (time < bestTime)
            {
                bestTime = time;
                hitNx = nx;
                hitNy = ny;
                hitEntity = other.id();
                hitBox = otherBox;
            }
        }

        if (hitEntity == InvalidEntity)
        {
            transform->position.x += dx;
            transform->position.y += dy;
            return false;
        }

        transform->position.x += dx * bestTime;
        transform->position.y += dy * bestTime;

        if (bestTime > 0.0f)
        {
            transform->position.x += hitNx * s_settings.epsilon;
            transform->position.y += hitNy * s_settings.epsilon;
        }
        else
        {
            Aabb2D moverBox2;
            if (getAabb2D(entity, *transform, moverBox2) && intersects(moverBox2, hitBox))
            {
                const float overlap1 = hitBox.maxX - moverBox2.minX;
                const float overlap2 = moverBox2.maxX - hitBox.minX;
                const float resolveX = (overlap1 < overlap2) ? -overlap1 : overlap2;

                const float overlapY1 = hitBox.maxY - moverBox2.minY;
                const float overlapY2 = moverBox2.maxY - hitBox.minY;
                const float resolveY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;

                if (std::abs(resolveX) < std::abs(resolveY))
                {
                    transform->position.x += resolveX;
                    hitNx = (resolveX < 0.0f) ? -1.0f : 1.0f;
                    hitNy = 0.0f;
                }
                else
                {
                    transform->position.y += resolveY;
                    hitNx = 0.0f;
                    hitNy = (resolveY < 0.0f) ? -1.0f : 1.0f;
                }
            }
        }

        updateContactState(*collider, hitNx, hitNy, 0.0f, true);

        outInfo.hit = true;
        outInfo.collider = hitEntity;
        outInfo.normal[0] = collider->lastNormal[0];
        outInfo.normal[1] = collider->lastNormal[1];
        outInfo.normal[2] = collider->lastNormal[2];
        outInfo.travel = std::sqrt((dx * bestTime) * (dx * bestTime) + (dy * bestTime) * (dy * bestTime));
        return true;
    }

    bool MoveAndCollide3D(Entity &entity, const float motion[3], float dt, CollisionInfo &outInfo)
    {
        outInfo = CollisionInfo{};

        auto *scene = entity.scene();
        auto *transform = entity.tryGetComponent<TransformComponent>();
        auto *collider = entity.tryGetComponent<ColliderComponent>();
        if (!scene || !transform || !collider || collider->is2D || dt <= 0.0f || !motion)
        {
            return false;
        }

        clearContactState(*collider);

        const float dx = motion[0];
        const float dy = motion[1];
        const float dz = motion[2];
        const auto all = scene->view<TransformComponent, ColliderComponent>();

        Aabb3D moverBox;
        if (!getAabb3D(entity, *transform, moverBox))
        {
            return false;
        }

        float bestTime = 1.0f;
        float hitNx = 0.0f;
        float hitNy = 0.0f;
        float hitNz = 0.0f;
        EntityId hitEntity = InvalidEntity;
        Aabb3D hitBox{};

        for (auto &other : all)
        {
            if (other.id() == entity.id())
            {
                continue;
            }

            auto *otherCollider = other.tryGetComponent<ColliderComponent>();
            auto *otherTransform = other.tryGetComponent<TransformComponent>();
            if (!otherCollider || !otherTransform || otherCollider->is2D)
            {
                continue;
            }

            if (!shouldCollide(entity.tryGetComponent<CollisionLayerComponent>(), other.tryGetComponent<CollisionLayerComponent>()))
            {
                continue;
            }

            Aabb3D otherBox;
            if (!getAabb3D(other, *otherTransform, otherBox))
            {
                continue;
            }

            float time = 0.0f;
            float nx = 0.0f;
            float ny = 0.0f;
            float nz = 0.0f;
            if (!sweepAabb3D(moverBox, otherBox, dx, dy, dz, time, nx, ny, nz))
            {
                continue;
            }

            if (time < bestTime)
            {
                bestTime = time;
                hitNx = nx;
                hitNy = ny;
                hitNz = nz;
                hitEntity = other.id();
                hitBox = otherBox;
            }
        }

        if (hitEntity == InvalidEntity)
        {
            transform->position.x += dx;
            transform->position.y += dy;
            transform->position.z += dz;
            return false;
        }

        transform->position.x += dx * bestTime;
        transform->position.y += dy * bestTime;
        transform->position.z += dz * bestTime;

        if (bestTime > 0.0f)
        {
            transform->position.x += hitNx * s_settings.epsilon;
            transform->position.y += hitNy * s_settings.epsilon;
            transform->position.z += hitNz * s_settings.epsilon;
        }
        else
        {
            Aabb3D moverBox2;
            if (getAabb3D(entity, *transform, moverBox2) && intersects(moverBox2, hitBox))
            {
                const float overlapX1 = hitBox.maxX - moverBox2.minX;
                const float overlapX2 = moverBox2.maxX - hitBox.minX;
                const float resolveX = (overlapX1 < overlapX2) ? -overlapX1 : overlapX2;

                const float overlapY1 = hitBox.maxY - moverBox2.minY;
                const float overlapY2 = moverBox2.maxY - hitBox.minY;
                const float resolveY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;

                const float overlapZ1 = hitBox.maxZ - moverBox2.minZ;
                const float overlapZ2 = moverBox2.maxZ - hitBox.minZ;
                const float resolveZ = (overlapZ1 < overlapZ2) ? -overlapZ1 : overlapZ2;

                float absX = std::abs(resolveX);
                float absY = std::abs(resolveY);
                float absZ = std::abs(resolveZ);

                if (absX <= absY && absX <= absZ)
                {
                    transform->position.x += resolveX;
                    hitNx = (resolveX < 0.0f) ? -1.0f : 1.0f;
                    hitNy = 0.0f;
                    hitNz = 0.0f;
                }
                else if (absY <= absX && absY <= absZ)
                {
                    transform->position.y += resolveY;
                    hitNx = 0.0f;
                    hitNy = (resolveY < 0.0f) ? -1.0f : 1.0f;
                    hitNz = 0.0f;
                }
                else
                {
                    transform->position.z += resolveZ;
                    hitNx = 0.0f;
                    hitNy = 0.0f;
                    hitNz = (resolveZ < 0.0f) ? -1.0f : 1.0f;
                }
            }
        }

        updateContactState(*collider, hitNx, hitNy, hitNz, false);

        outInfo.hit = true;
        outInfo.collider = hitEntity;
        outInfo.normal[0] = collider->lastNormal[0];
        outInfo.normal[1] = collider->lastNormal[1];
        outInfo.normal[2] = collider->lastNormal[2];
        outInfo.travel = std::sqrt((dx * bestTime) * (dx * bestTime) + (dy * bestTime) * (dy * bestTime) + (dz * bestTime) * (dz * bestTime));
        return true;
    }

    bool IsOnFloor(const Entity &entity)
    {
        auto *collider = const_cast<Entity &>(entity).tryGetComponent<ColliderComponent>();
        return collider ? collider->onFloor : false;
    }

    bool IsOnWall(const Entity &entity)
    {
        auto *collider = const_cast<Entity &>(entity).tryGetComponent<ColliderComponent>();
        return collider ? collider->onWall : false;
    }

    bool IsOnCeiling(const Entity &entity)
    {
        auto *collider = const_cast<Entity &>(entity).tryGetComponent<ColliderComponent>();
        return collider ? collider->onCeiling : false;
    }

    void GetFloorNormal(const Entity &entity, float outNormal[3])
    {
        if (!outNormal)
        {
            return;
        }

        auto *collider = const_cast<Entity &>(entity).tryGetComponent<ColliderComponent>();
        if (!collider)
        {
            outNormal[0] = 0.0f;
            outNormal[1] = 0.0f;
            outNormal[2] = 0.0f;
            return;
        }

        outNormal[0] = collider->lastNormal[0];
        outNormal[1] = collider->lastNormal[1];
        outNormal[2] = collider->lastNormal[2];
    }
}
