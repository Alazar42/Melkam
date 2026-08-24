#pragma once

#include "nodes/3D/physics/CollisionObject3D.hpp"
#include "nodes/3D/physics/PhysicsServer3D.hpp"
#include <memory>

// Dynamic 3D Physics Rigid Body (inspired by Godot RigidBody3D)
class RigidBody3D : public CollisionObject3D {
public:
  float mass = 1.0f;
  float gravityScale = 1.0f;
  float friction = 0.5f;
  float bounce = 0.0f;
  bool freeze = false;

  RigidBody3D() : CollisionObject3D("RigidBody3D") {}
  ~RigidBody3D() override {
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

  void onPhysicsProcess(float delta) override {
    (void)delta;
    if (m_rigidBody && !freeze) {
      btTransform trans;
      m_rigidBody->getMotionState()->getWorldTransform(trans);
      btVector3 origin = trans.getOrigin();
      btQuaternion rot = trans.getRotation();

      setPosition(Vector3(origin.x(), origin.y(), origin.z()));
      setRotation(Quaternion(rot.x(), rot.y(), rot.z(), rot.w()).to_euler());
    }
  }

  void setMass(float m) {
    mass = m;
    if (m_rigidBody && m_bulletShape) {
      btVector3 inertia(0, 0, 0);
      if (mass > 0.0f) {
        m_bulletShape->calculateLocalInertia(mass, inertia);
      }
      m_rigidBody->setMassProps(mass, inertia);
      m_rigidBody->updateInertiaTensor();
    }
  }

  void setLinearVelocity(const Vector3 &vel) {
    if (m_rigidBody) {
      m_rigidBody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
      m_rigidBody->activate(true);
    }
  }

  Vector3 getLinearVelocity() const {
    if (m_rigidBody) {
      btVector3 v = m_rigidBody->getLinearVelocity();
      return Vector3(v.x(), v.y(), v.z());
    }
    return Vector3(0.0f, 0.0f, 0.0f);
  }

  void applyImpulse(const Vector3 &impulse, const Vector3 &relPos = Vector3(0, 0, 0)) {
    if (m_rigidBody) {
      m_rigidBody->activate(true);
      m_rigidBody->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z), btVector3(relPos.x, relPos.y, relPos.z));
    }
  }

  void applyForce(const Vector3 &force, const Vector3 &relPos = Vector3(0, 0, 0)) {
    if (m_rigidBody) {
      m_rigidBody->activate(true);
      m_rigidBody->applyForce(btVector3(force.x, force.y, force.z), btVector3(relPos.x, relPos.y, relPos.z));
    }
  }

  void applyTorque(const Vector3 &torque) {
    if (m_rigidBody) {
      m_rigidBody->activate(true);
      m_rigidBody->applyTorque(btVector3(torque.x, torque.y, torque.z));
    }
  }

  void buildBulletBody() {
    destroyBulletBody();

    Transform3D trans = getGlobalTransform();
    btTransform btTrans;
    btTrans.setIdentity();
    btTrans.setOrigin(btVector3(trans.origin.x, trans.origin.y, trans.origin.z));

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

    btVector3 localInertia(0, 0, 0);
    if (mass > 0.0f && !freeze) {
      m_bulletShape->calculateLocalInertia(mass, localInertia);
    }

    m_motionState = std::make_unique<btDefaultMotionState>(btTrans);
    btRigidBody::btRigidBodyConstructionInfo ci(mass, m_motionState.get(), m_bulletShape.get(), localInertia);
    ci.m_friction = friction;
    ci.m_restitution = bounce;

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
