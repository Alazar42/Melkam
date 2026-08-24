#pragma once

#include "helper/vectors/Vector3.hpp"
#include "nodes/3D/Transform3D.hpp"
#include "nodes/3D/physics/Shape3D.hpp"
#include <btBulletDynamicsCommon.h>
#include <memory>
#include <vector>

// Master 3D Physics Server backed by Bullet 3 (inspired by Godot PhysicsServer3D)
class PhysicsServer3D {
public:
  static PhysicsServer3D &get() {
    static PhysicsServer3D s_instance;
    return s_instance;
  }

  void init() {
    if (m_dynamicsWorld) return;

    m_collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
    m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfiguration.get());
    m_broadphase = std::make_unique<btDbvtBroadphase>();
    m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();

    m_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
        m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionConfiguration.get());

    m_dynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));
  }

  void shutdown() {
    m_dynamicsWorld.reset();
    m_solver.reset();
    m_broadphase.reset();
    m_dispatcher.reset();
    m_collisionConfiguration.reset();
  }

  void step(float delta) {
    if (m_dynamicsWorld) {
      m_dynamicsWorld->stepSimulation(delta, 10, 1.0f / 60.0f);
    }
  }

  void setGravity(const Vector3 &gravity) {
    if (m_dynamicsWorld) {
      m_dynamicsWorld->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
    }
  }

  Vector3 getGravity() const {
    if (m_dynamicsWorld) {
      btVector3 g = m_dynamicsWorld->getGravity();
      return Vector3(g.x(), g.y(), g.z());
    }
    return Vector3(0.0f, -9.81f, 0.0f);
  }

  btDiscreteDynamicsWorld *getDynamicsWorld() const { return m_dynamicsWorld.get(); }

  void addRigidBody(btRigidBody *body) {
    if (m_dynamicsWorld && body && !body->isInWorld()) {
      m_dynamicsWorld->addRigidBody(body);
    }
  }

  void removeRigidBody(btRigidBody *body) {
    if (m_dynamicsWorld && body && body->isInWorld()) {
      m_dynamicsWorld->removeRigidBody(body);
    }
  }

  void addCollisionObject(btCollisionObject *obj) {
    if (m_dynamicsWorld && obj && obj->getBroadphaseHandle() == nullptr) {
      m_dynamicsWorld->addCollisionObject(obj);
    }
  }

  void removeCollisionObject(btCollisionObject *obj) {
    if (m_dynamicsWorld && obj && obj->getBroadphaseHandle() != nullptr) {
      m_dynamicsWorld->removeCollisionObject(obj);
    }
  }

  void clear() {
    if (!m_dynamicsWorld) return;
    for (int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i) {
      btCollisionObject *obj = m_dynamicsWorld->getCollisionObjectArray()[i];
      if (btRigidBody *body = btRigidBody::upcast(obj)) {
        m_dynamicsWorld->removeRigidBody(body);
      } else {
        m_dynamicsWorld->removeCollisionObject(obj);
      }
    }
  }

  // Bullet 3 Raycasting
  bool raycast(const Vector3 &from, const Vector3 &to, RayCastHit3D &outHit) {
    if (!m_dynamicsWorld) return false;

    Vector3 diff = to - from;
    float dist = diff.length();
    if (dist < 0.001f) return false;

    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);

    btCollisionWorld::ClosestRayResultCallback callback(btFrom, btTo);
    m_dynamicsWorld->rayTest(btFrom, btTo, callback);

    if (callback.hasHit()) {
      outHit.hit = true;
      outHit.distance = dist * callback.m_closestHitFraction;
      outHit.point = Vector3(callback.m_hitPointWorld.x(), callback.m_hitPointWorld.y(), callback.m_hitPointWorld.z());
      Vector3 hitNorm(callback.m_hitNormalWorld.x(), callback.m_hitNormalWorld.y(), callback.m_hitNormalWorld.z());
      if (hitNorm.length_squared() > 0.0001f) {
        outHit.normal = hitNorm.normalized();
      } else {
        outHit.normal = Vector3(0.0f, 1.0f, 0.0f);
      }
      return true;
    }
    return false;
  }

private:
  PhysicsServer3D() = default;

  std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
  std::unique_ptr<btCollisionDispatcher> m_dispatcher;
  std::unique_ptr<btDbvtBroadphase> m_broadphase;
  std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
  std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;
};
