#pragma once

#include "helper/vectors/Vector3.hpp"
#include "nodes/3D/Quaternion.hpp"
#include <algorithm>
#include <cmath>

// Godot-Standard 3x3 Orientation and Scale Matrix
class Basis {
public:
  Vector3 rows[3] = {
      Vector3(1.0f, 0.0f, 0.0f),
      Vector3(0.0f, 1.0f, 0.0f),
      Vector3(0.0f, 0.0f, 1.0f)};

  constexpr Basis() = default;

  constexpr Basis(const Vector3 &row0, const Vector3 &row1, const Vector3 &row2) {
    rows[0] = row0;
    rows[1] = row1;
    rows[2] = row2;
  }

  // Constructs Basis from axis and angle
  Basis(const Vector3 &axis, float angle) {
    Quaternion q(axis, angle);
    *this = Basis(q);
  }

  // Constructs Basis from Quaternion
  explicit Basis(const Quaternion &q) {
    float s = 2.0f / (q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);

    float xs = q.x * s,   ys = q.y * s,   zs = q.z * s;
    float wx = q.w * xs,  wy = q.w * ys,  wz = q.w * zs;
    float xx = q.x * xs,  xy = q.x * ys,  xz = q.x * zs;
    float yy = q.y * ys,  yz = q.y * zs,  zz = q.z * zs;

    rows[0] = Vector3(1.0f - (yy + zz), xy - wz, xz + wy);
    rows[1] = Vector3(xy + wz, 1.0f - (xx + zz), yz - wx);
    rows[2] = Vector3(xz - wy, yz + wx, 1.0f - (xx + yy));
  }

  // Constructs Basis from Euler angles (in radians, YXZ order)
  static Basis fromEuler(const Vector3 &euler) {
    float c = std::cos(euler.x);
    float s = std::sin(euler.x);

    Basis xmat(Vector3(1.0f, 0.0f, 0.0f),
               Vector3(0.0f, c, -s),
               Vector3(0.0f, s, c));

    c = std::cos(euler.y);
    s = std::sin(euler.y);

    Basis ymat(Vector3(c, 0.0f, s),
               Vector3(0.0f, 1.0f, 0.0f),
               Vector3(-s, 0.0f, c));

    c = std::cos(euler.z);
    s = std::sin(euler.z);

    Basis zmat(Vector3(c, -s, 0.0f),
               Vector3(s, c, 0.0f),
               Vector3(0.0f, 0.0f, 1.0f));

    return ymat * xmat * zmat;
  }

  // Look-at matrix constructor
  static Basis lookingAt(const Vector3 &target, const Vector3 &up = Vector3(0.0f, 1.0f, 0.0f)) {
    Vector3 v_z = -target.normalized();
    Vector3 v_x = up.cross(v_z).normalized();
    Vector3 v_y = v_z.cross(v_x);

    return Basis(
        Vector3(v_x.x, v_y.x, v_z.x),
        Vector3(v_x.y, v_y.y, v_z.y),
        Vector3(v_x.z, v_y.z, v_z.z));
  }

  Vector3 &operator[](int index) { return rows[index]; }
  const Vector3 &operator[](int index) const { return rows[index]; }

  float determinant() const {
    return rows[0].x * (rows[1].y * rows[2].z - rows[1].z * rows[2].y) -
           rows[0].y * (rows[1].x * rows[2].z - rows[1].z * rows[2].x) +
           rows[0].z * (rows[1].x * rows[2].y - rows[1].y * rows[2].x);
  }

  Basis transposed() const {
    return Basis(
        Vector3(rows[0].x, rows[1].x, rows[2].x),
        Vector3(rows[0].y, rows[1].y, rows[2].y),
        Vector3(rows[0].z, rows[1].z, rows[2].z));
  }

  Basis inverse() const {
    float det = determinant();
    if (std::abs(det) < 0.00001f) return Basis();

    float inv_det = 1.0f / det;
    Basis inv;
    inv.rows[0].x = (rows[1].y * rows[2].z - rows[1].z * rows[2].y) * inv_det;
    inv.rows[0].y = (rows[0].z * rows[2].y - rows[0].y * rows[2].z) * inv_det;
    inv.rows[0].z = (rows[0].y * rows[1].z - rows[0].z * rows[1].y) * inv_det;

    inv.rows[1].x = (rows[1].z * rows[2].x - rows[1].x * rows[2].z) * inv_det;
    inv.rows[1].y = (rows[0].x * rows[2].z - rows[0].z * rows[2].x) * inv_det;
    inv.rows[1].z = (rows[0].z * rows[1].x - rows[0].x * rows[1].z) * inv_det;

    inv.rows[2].x = (rows[1].x * rows[2].y - rows[1].y * rows[2].x) * inv_det;
    inv.rows[2].y = (rows[0].y * rows[2].x - rows[0].x * rows[2].y) * inv_det;
    inv.rows[2].z = (rows[0].x * rows[1].y - rows[0].y * rows[1].x) * inv_det;

    return inv;
  }

  Basis orthonormalized() const {
    Vector3 c0(rows[0].x, rows[1].x, rows[2].x);
    Vector3 c1(rows[0].y, rows[1].y, rows[2].y);
    Vector3 c2(rows[0].z, rows[1].z, rows[2].z);

    c0 = c0.normalized();
    c1 = (c1 - c0 * c0.dot(c1)).normalized();
    c2 = (c2 - c0 * c0.dot(c2) - c1 * c1.dot(c2)).normalized();

    return Basis(
        Vector3(c0.x, c1.x, c2.x),
        Vector3(c0.y, c1.y, c2.y),
        Vector3(c0.z, c1.z, c2.z));
  }

  Basis rotated(const Vector3 &axis, float angle) const {
    return Basis(axis, angle) * (*this);
  }

  Basis scaled(const Vector3 &scale) const {
    return Basis(
        rows[0] * scale.x,
        rows[1] * scale.y,
        rows[2] * scale.z);
  }

  Vector3 get_scale() const {
    float det_sign = (determinant() < 0.0f) ? -1.0f : 1.0f;
    return Vector3(
        Vector3(rows[0].x, rows[1].x, rows[2].x).length() * det_sign,
        Vector3(rows[0].y, rows[1].y, rows[2].y).length(),
        Vector3(rows[0].z, rows[1].z, rows[2].z).length());
  }

  Quaternion get_quaternion() const {
    Basis ortho = orthonormalized();
    float trace = ortho.rows[0].x + ortho.rows[1].y + ortho.rows[2].z;

    if (trace > 0.0f) {
      float s = std::sqrt(trace + 1.0f) * 2.0f;
      return Quaternion(
          (ortho.rows[2].y - ortho.rows[1].z) / s,
          (ortho.rows[0].z - ortho.rows[2].x) / s,
          (ortho.rows[1].x - ortho.rows[0].y) / s,
          0.25f * s);
    } else if (ortho.rows[0].x > ortho.rows[1].y && ortho.rows[0].x > ortho.rows[2].z) {
      float s = std::sqrt(1.0f + ortho.rows[0].x - ortho.rows[1].y - ortho.rows[2].z) * 2.0f;
      return Quaternion(
          0.25f * s,
          (ortho.rows[0].y + ortho.rows[1].x) / s,
          (ortho.rows[0].z + ortho.rows[2].x) / s,
          (ortho.rows[2].y - ortho.rows[1].z) / s);
    } else if (ortho.rows[1].y > ortho.rows[2].z) {
      float s = std::sqrt(1.0f + ortho.rows[1].y - ortho.rows[0].x - ortho.rows[2].z) * 2.0f;
      return Quaternion(
          (ortho.rows[0].y + ortho.rows[1].x) / s,
          0.25f * s,
          (ortho.rows[1].z + ortho.rows[2].y) / s,
          (ortho.rows[0].z - ortho.rows[2].x) / s);
    } else {
      float s = std::sqrt(1.0f + ortho.rows[2].z - ortho.rows[0].x - ortho.rows[1].y) * 2.0f;
      return Quaternion(
          (ortho.rows[0].z + ortho.rows[2].x) / s,
          (ortho.rows[1].z + ortho.rows[2].y) / s,
          0.25f * s,
          (ortho.rows[1].x - ortho.rows[0].y) / s);
    }
  }

  // Vector3 Transformation
  Vector3 xform(const Vector3 &v) const {
    return Vector3(
        rows[0].dot(v),
        rows[1].dot(v),
        rows[2].dot(v));
  }

  Vector3 operator*(const Vector3 &v) const { return xform(v); }

  // Basis Matrix Multiplication
  Basis operator*(const Basis &b) const {
    return Basis(
        Vector3(rows[0].dot(Vector3(b.rows[0].x, b.rows[1].x, b.rows[2].x)),
                rows[0].dot(Vector3(b.rows[0].y, b.rows[1].y, b.rows[2].y)),
                rows[0].dot(Vector3(b.rows[0].z, b.rows[1].z, b.rows[2].z))),
        Vector3(rows[1].dot(Vector3(b.rows[0].x, b.rows[1].x, b.rows[2].x)),
                rows[1].dot(Vector3(b.rows[0].y, b.rows[1].y, b.rows[2].y)),
                rows[1].dot(Vector3(b.rows[0].z, b.rows[1].z, b.rows[2].z))),
        Vector3(rows[2].dot(Vector3(b.rows[0].x, b.rows[1].x, b.rows[2].x)),
                rows[2].dot(Vector3(b.rows[0].y, b.rows[1].y, b.rows[2].y)),
                rows[2].dot(Vector3(b.rows[0].z, b.rows[1].z, b.rows[2].z))));
  }
};
