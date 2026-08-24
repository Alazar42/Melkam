#pragma once

#include "nodes/3D/physics/CollisionObject3D.hpp"
#include "nodes/3D/physics/PhysicsServer3D.hpp"
#include <memory>

// Static Immovable 3D Physics Body (inspired by Godot StaticBody3D)
class StaticBody3D : public CollisionObject3D {
public:
  Vector3 constantLinearVelocity{0.0f, 0.0f, 0.0f};
  Vector3 constantAngularVelocity{0.0f, 0.0f, 0.0f};

  StaticBody3D() : CollisionObject3D("StaticBody3D") {}
  ~StaticBody3D() override {
    destroyBulletBody();
  }

  void onReady() override {
    buildBulletBody();
  }

  void rebuildBulletBody() override {
    buildBulletBody();
  }

  void onDestroy() override {
    destroyBulletBody();
  }

  void buildBulletBody() {
    destroyBulletBody();

    Transform3D trans = getGlobalTransform();
    btTransform btTrans;
    btTrans.setIdentity();
    btTrans.setOrigin(btVector3(trans.origin.x, trans.origin.y, trans.origin.z));

    // Create collision shape from child shapes
    auto childShapes = getShapes();
    if (!childShapes.empty()) {
      auto shape = childShapes[0];
      if (auto box = dynamic_cast<BoxShape3D *>(shape.get())) {
        m_bulletShape = std::make_unique<btBoxShape>(btVector3(box->size.x * 0.5f, box->size.y * 0.5f, box->size.z * 0.5f));
      } else if (auto sphere = dynamic_cast<SphereShape3D *>(shape.get())) {
        m_bulletShape = std::make_unique<btSphereShape>(sphere->radius);
      } else if (auto capsule = dynamic_cast<CapsuleShape3D *>(shape.get())) {
        m_bulletShape = std::make_unique<btCapsuleShape>(capsule->radius, capsule->height);
      } else if (auto cylinder = dynamic_cast<CylinderShape3D *>(shape.get())) {
        m_bulletShape = std::make_unique<btCylinderShape>(btVector3(cylinder->radius, cylinder->height * 0.5f, cylinder->radius));
      }
    }

    if (!m_bulletShape) {
      m_bulletShape = std::make_unique<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));
    }

    m_motionState = std::make_unique<btDefaultMotionState>(btTrans);
    btRigidBody::btRigidBodyConstructionInfo ci(0.0f, m_motionState.get(), m_bulletShape.get(), btVector3(0, 0, 0));
    m_rigidBody = std::make_unique<btRigidBody>(ci);
    m_rigidBody->setUserPointer(this);

    PhysicsServer3D::get().addRigidBody(m_rigidBody.get());
  }

  void destroyBulletBody() {
    if (m_rigidBody) {
      PhysicsServer3D::get().removeRigidBody(m_rigidBody.get());
      m_rigidBody.reset();
    }
    m_motionState.reset();
    m_bulletShape.reset();
  }

private:
  std::unique_ptr<btCollisionShape> m_bulletShape;
  std::unique_ptr<btDefaultMotionState> m_motionState;
  std::unique_ptr<btRigidBody> m_rigidBody;
};
