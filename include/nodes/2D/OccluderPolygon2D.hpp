#pragma once

#include "helper/vectors/Vector2.hpp"
#include <vector>

enum class OccluderCullMode {
  Disabled,
  ClockWise,
  CounterClockWise
};

// 2D Light Occluder Polygon Resource (inspired by Godot OccluderPolygon2D)
class OccluderPolygon2D {
public:
  std::vector<Vector2> polygon;
  bool closed = true;
  OccluderCullMode cullMode = OccluderCullMode::Disabled;

  OccluderPolygon2D() = default;
  explicit OccluderPolygon2D(std::vector<Vector2> points, bool isClosed = true)
      : polygon(std::move(points)), closed(isClosed) {}

  void setPolygon(std::vector<Vector2> points) {
    polygon = std::move(points);
  }

  void addPoint(const Vector2 &p) {
    polygon.push_back(p);
  }

  void clear() {
    polygon.clear();
  }

  int getPointCount() const {
    return static_cast<int>(polygon.size());
  }
};
