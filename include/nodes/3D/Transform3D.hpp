#pragma once

#include "helper/vectors/Vector3.hpp"
#include "nodes/3D/Basis.hpp"
#include <array>

// Godot-Standard 4x4 Affine Spatial Transformation Matrix
class Transform3D {
public:
  Basis basis;
  Vector3 origin{0.0f, 0.0f, 0.0f};

  constexpr Transform3D() = default;
  constexpr Transform3D(const Basis &basis, const Vector3 &origin)
      : basis(basis), origin(origin) {}

  // Constructs from translation only
  explicit Transform3D(const Vector3 &origin)
      : basis(), origin(origin) {}

  Transform3D inverse() const {
    Basis inv_basis = basis.inverse();
    return Transform3D(inv_basis, inv_basis.xform(-origin));
  }

  Transform3D affine_inverse() const {
    Basis inv_basis = basis.inverse();
    return Transform3D(inv_basis, inv_basis.xform(-origin));
  }

  Transform3D orthonormalized() const {
    return Transform3D(basis.orthonormalized(), origin);
  }

  Transform3D rotated(const Vector3 &axis, float angle) const {
    return Transform3D((Basis(axis, angle) * basis).orthonormalized(), origin);
  }

  Transform3D rotated_local(const Vector3 &axis, float angle) const {
    return Transform3D((basis * Basis(axis, angle)).orthonormalized(), origin);
  }

  Transform3D scaled(const Vector3 &scale) const {
    return Transform3D(basis.scaled(scale), origin * scale);
  }

  Transform3D scaled_local(const Vector3 &scale) const {
    return Transform3D(Basis(
        basis.rows[0] * scale.x,
        basis.rows[1] * scale.y,
        basis.rows[2] * scale.z), origin);
  }

  Transform3D translated(const Vector3 &offset) const {
    return Transform3D(basis, origin + offset);
  }

  Transform3D translated_local(const Vector3 &offset) const {
    return Transform3D(basis, origin + basis.xform(offset));
  }

  Transform3D looking_at(const Vector3 &target, const Vector3 &up = Vector3(0.0f, 1.0f, 0.0f)) const {
    return Transform3D(Basis::lookingAt(target - origin, up), origin);
  }

  Transform3D interpolate_with(const Transform3D &to, float weight) const {
    Quaternion src_q = basis.get_quaternion();
    Quaternion dst_q = to.basis.get_quaternion();
    Quaternion rot_q = src_q.slerp(dst_q, weight);

    Vector3 src_scale = basis.get_scale();
    Vector3 dst_scale = to.basis.get_scale();
    Vector3 scale = src_scale.lerp(dst_scale, weight);

    Basis interpolated_basis = Basis(rot_q).scaled(scale);
    Vector3 interpolated_origin = origin.lerp(to.origin, weight);

    return Transform3D(interpolated_basis, interpolated_origin);
  }

  // Point transformation
  Vector3 xform(const Vector3 &v) const {
    return basis.xform(v) + origin;
  }

  // Inverse point transformation
  Vector3 xform_inv(const Vector3 &v) const {
    return basis.inverse().xform(v - origin);
  }

  Vector3 operator*(const Vector3 &v) const { return xform(v); }

  // Transform multiplication (A * B)
  Transform3D operator*(const Transform3D &other) const {
    return Transform3D(basis * other.basis, xform(other.origin));
  }

  // Converts to column-major float[16] array for Vulkan / GPU Uniform Buffers
  std::array<float, 16> toMatrix4() const {
    return {
        basis.rows[0].x, basis.rows[1].x, basis.rows[2].x, 0.0f,
        basis.rows[0].y, basis.rows[1].y, basis.rows[2].y, 0.0f,
        basis.rows[0].z, basis.rows[1].z, basis.rows[2].z, 0.0f,
        origin.x,        origin.y,        origin.z,        1.0f
    };
  }
};
