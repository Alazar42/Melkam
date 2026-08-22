#pragma once

#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "physics/2D/Collision2D.hpp"
#include "renderers/Renderer2D.hpp"
#include <box2d/box2d.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

class CollisionObject2D;

// Global 2D Physics Server (wrapping Box2D 3.1.1 with Godot-inspired API).
class PhysicsServer2D {
public:
  // Pixels to Meters scaling ratio (50.0f pixels = 1.0 meter)
  static constexpr float PIXELS_PER_METER = 50.0f;

  // Converts pixel coordinates/dimensions to Box2D meters
  static b2Vec2 toMeters(const Vector2 &pixels) {
    return {pixels.x / PIXELS_PER_METER, pixels.y / PIXELS_PER_METER};
  }

  static float toMeters(float pixels) {
    return pixels / PIXELS_PER_METER;
  }

  // Converts Box2D meters to engine pixels
  static Vector2 toPixels(const b2Vec2 &meters) {
    return {meters.x * PIXELS_PER_METER, meters.y * PIXELS_PER_METER};
  }

  static float toPixels(float meters) {
    return meters * PIXELS_PER_METER;
  }

  // Initializes the physics world with default gravity (downwards 980.0 px/s²)
  static void init(const Vector2 &gravity = {0.0f, 980.0f}) {
    if (!b2World_IsValid(s_worldId)) {
      b2WorldDef worldDef = b2DefaultWorldDef();
      worldDef.gravity = toMeters(gravity);
      s_worldId = b2CreateWorld(&worldDef);
    }
  }

  // Shuts down and cleans up the physics world
  static void shutdown() {
    if (b2World_IsValid(s_worldId)) {
      b2DestroyWorld(s_worldId);
      s_worldId = b2_nullWorldId;
    }
    s_registeredObjects.clear();
  }

  // Steps the Box2D physics simulation and syncs node transforms
  static void step(float fixedDeltaTime, int subSteps = 4) {
    if (!b2World_IsValid(s_worldId)) {
      init();
    }
    b2World_Step(s_worldId, fixedDeltaTime, subSteps);
    syncRegisteredObjects();
  }

  // Declared here, defined after CollisionObject2D
  static void syncRegisteredObjects();

  // Returns the Box2D world handle
  static b2WorldId getWorldId() {
    if (!b2World_IsValid(s_worldId)) {
      init();
    }
    return s_worldId;
  }

  // Sets world gravity in pixels per second squared
  static void setGravity(const Vector2 &gravity) {
    if (b2World_IsValid(s_worldId)) {
      b2World_SetGravity(s_worldId, toMeters(gravity));
    }
  }

  // Returns world gravity in pixels per second squared
  static Vector2 getGravity() {
    if (b2World_IsValid(s_worldId)) {
      b2Vec2 g = b2World_GetGravity(s_worldId);
      return toPixels(g);
    }
    return {0.0f, 980.0f};
  }

  // Registers a CollisionObject2D in the server
  static void registerObject(CollisionObject2D *obj) {
    if (obj) {
      if (std::find(s_registeredObjects.begin(), s_registeredObjects.end(), obj) ==
          s_registeredObjects.end()) {
        s_registeredObjects.push_back(obj);
      }
    }
  }

  // Unregisters a CollisionObject2D from the server
  static void unregisterObject(CollisionObject2D *obj) {
    auto it = std::find(s_registeredObjects.begin(), s_registeredObjects.end(), obj);
    if (it != s_registeredObjects.end()) {
      s_registeredObjects.erase(it);
    }
  }

  // Returns all registered collision objects
  static const std::vector<CollisionObject2D *> &getRegisteredObjects() {
    return s_registeredObjects;
  }

  // Enables or disables debug collision wireframe rendering
  static void setDebugCollisions(bool enabled) {
    s_debugCollisions = enabled;
  }

  // Returns true if debug collision wireframes are enabled
  static bool isDebugCollisions() {
    return s_debugCollisions;
  }

  // Performs a 2D raycast in pixel coordinates
  static RaycastHit2D raycast(const Vector2 &from, const Vector2 &to,
                              uint32_t collisionMask = 0xFFFFFFFF);

  // Draws debug collision wireframes for all registered collision bodies
  static void drawDebug();

private:
  inline static b2WorldId s_worldId = b2_nullWorldId;
  inline static bool s_debugCollisions = false;
  inline static std::vector<CollisionObject2D *> s_registeredObjects;
};
