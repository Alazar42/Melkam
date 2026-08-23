#pragma once

#include "helper/vectors/Vector2.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

struct CurvePoint2D {
  Vector2 position{0.0f, 0.0f};
  Vector2 in{0.0f, 0.0f};  // In-tangent handle offset
  Vector2 out{0.0f, 0.0f}; // Out-tangent handle offset
};

// 2D Parametric Cubic Bezier Curve Resource (inspired by Godot Curve2D)
class Curve2D {
public:
  float bakeInterval = 5.0f;

  void addPoint(const Vector2 &pos, const Vector2 &inHandle = {0.0f, 0.0f}, const Vector2 &outHandle = {0.0f, 0.0f}) {
    m_points.push_back({pos, inHandle, outHandle});
    m_needsBake = true;
  }

  void setPointPosition(int index, const Vector2 &pos) {
    if (index >= 0 && index < static_cast<int>(m_points.size())) {
      m_points[index].position = pos;
      m_needsBake = true;
    }
  }

  Vector2 getPointPosition(int index) const {
    if (index >= 0 && index < static_cast<int>(m_points.size())) {
      return m_points[index].position;
    }
    return {0.0f, 0.0f};
  }

  void clearPoints() {
    m_points.clear();
    m_bakedPoints.clear();
    m_bakedLengths.clear();
    m_totalLength = 0.0f;
    m_needsBake = false;
  }

  int getPointCount() const {
    return static_cast<int>(m_points.size());
  }

  float getBakedLength() {
    if (m_needsBake) bake();
    return m_totalLength;
  }

  Vector2 sampleBaked(float offset, bool loop = true) {
    if (m_needsBake) bake();
    if (m_bakedPoints.empty()) return {0.0f, 0.0f};
    if (m_bakedPoints.size() == 1) return m_bakedPoints[0];

    if (m_totalLength <= 0.0001f) return m_bakedPoints[0];

    if (loop) {
      offset = std::fmod(offset, m_totalLength);
      if (offset < 0.0f) offset += m_totalLength;
    } else {
      offset = std::clamp(offset, 0.0f, m_totalLength);
    }

    // Binary search in baked lengths
    auto it = std::lower_bound(m_bakedLengths.begin(), m_bakedLengths.end(), offset);
    int idx = static_cast<int>(std::distance(m_bakedLengths.begin(), it));
    if (idx <= 0) return m_bakedPoints[0];
    if (idx >= static_cast<int>(m_bakedPoints.size())) return m_bakedPoints.back();

    float l0 = m_bakedLengths[idx - 1];
    float l1 = m_bakedLengths[idx];
    float segLen = l1 - l0;
    float t = (segLen > 0.0001f) ? (offset - l0) / segLen : 0.0f;

    return m_bakedPoints[idx - 1].lerp(m_bakedPoints[idx], t);
  }

  Vector2 sampleBakedWithRotation(float offset, float &outRotationRadians, bool loop = true) {
    Vector2 p = sampleBaked(offset, loop);
    Vector2 pNext = sampleBaked(offset + 1.0f, loop);
    Vector2 diff = pNext - p;
    if (!diff.is_zero_approx()) {
      outRotationRadians = std::atan2(diff.y, diff.x);
    }
    return p;
  }

  const std::vector<Vector2> &getBakedPoints() {
    if (m_needsBake) bake();
    return m_bakedPoints;
  }

  void bake() {
    m_needsBake = false;
    m_bakedPoints.clear();
    m_bakedLengths.clear();
    m_totalLength = 0.0f;

    if (m_points.empty()) return;
    if (m_points.size() == 1) {
      m_bakedPoints.push_back(m_points[0].position);
      m_bakedLengths.push_back(0.0f);
      return;
    }

    m_bakedPoints.push_back(m_points[0].position);
    m_bakedLengths.push_back(0.0f);

    for (size_t i = 0; i + 1 < m_points.size(); ++i) {
      Vector2 p0 = m_points[i].position;
      Vector2 p1 = p0 + m_points[i].out;
      Vector2 p3 = m_points[i + 1].position;
      Vector2 p2 = p3 + m_points[i + 1].in;

      // Estimate segments for smooth subdivision
      float chord = (p3 - p0).length();
      int numSegments = std::max(4, static_cast<int>(chord / std::max(1.0f, bakeInterval)));

      for (int s = 1; s <= numSegments; ++s) {
        float t = static_cast<float>(s) / static_cast<float>(numSegments);
        Vector2 pt = cubicBezier(p0, p1, p2, p3, t);
        float dist = (pt - m_bakedPoints.back()).length();
        m_totalLength += dist;
        m_bakedPoints.push_back(pt);
        m_bakedLengths.push_back(m_totalLength);
      }
    }
  }

private:
  static Vector2 cubicBezier(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, const Vector2 &p3, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    Vector2 p = p0 * uuu;
    p += p1 * (3.0f * uu * t);
    p += p2 * (3.0f * u * tt);
    p += p3 * ttt;
    return p;
  }

  std::vector<CurvePoint2D> m_points;
  std::vector<Vector2> m_bakedPoints;
  std::vector<float> m_bakedLengths;
  float m_totalLength = 0.0f;
  bool m_needsBake = true;
};
