#pragma once

#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Node2D.hpp"
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

enum class Mesh2DType {
  Rectangle,
  Circle,
  Triangle,
  Polygon
};

// 2D Mesh and Geometric Instance Node (inspired by Godot's MeshInstance2D / Polygon2D).
class MeshInstance2D : public Node2D {
public:
  Mesh2DType meshType = Mesh2DType::Rectangle;
  Color color = Color::WHITE;
  bool filled = true;
  std::shared_ptr<Texture2D> texture = nullptr;

  // Geometry dimensions
  Vector2 size{50.0f, 50.0f}; // For rectangles
  float radius = 25.0f;       // For circles
  int segments = 32;          // For circles
  Vector2 p1{-25.0f, 25.0f};  // For triangles
  Vector2 p2{25.0f, 25.0f};
  Vector2 p3{0.0f, -25.0f};
  std::vector<Vector2> vertices; // For polygons

  MeshInstance2D() : Node2D("MeshInstance2D") {}

  MeshInstance2D(const MeshInstance2D &other)
      : Node2D(other), meshType(other.meshType), color(other.color),
        filled(other.filled), texture(other.texture), size(other.size),
        radius(other.radius), segments(other.segments), p1(other.p1),
        p2(other.p2), p3(other.p3), vertices(other.vertices) {}

  MeshInstance2D &operator=(const MeshInstance2D &other) {
    if (this != &other) {
      Node2D::operator=(other);
      meshType = other.meshType;
      color = other.color;
      filled = other.filled;
      texture = other.texture;
      size = other.size;
      radius = other.radius;
      segments = other.segments;
      p1 = other.p1;
      p2 = other.p2;
      p3 = other.p3;
      vertices = other.vertices;
    }
    return *this;
  }

  // Direct rectangle constructor
  MeshInstance2D(const Vector2 &rectSize, const Color &rectColor = Color::WHITE,
                 bool isFilled = true)
      : Node2D("MeshInstance2D"), meshType(Mesh2DType::Rectangle),
        color(rectColor), filled(isFilled), size(rectSize) {}

  // Direct circle constructor
  MeshInstance2D(float circleRadius, const Color &circleColor = Color::WHITE,
                 bool isFilled = true, int circleSegments = 32)
      : Node2D("MeshInstance2D"), meshType(Mesh2DType::Circle),
        color(circleColor), filled(isFilled), radius(circleRadius),
        segments(circleSegments) {}

  // Factory: creates a Rectangle 2D mesh
  static MeshInstance2D createRectangle(const Vector2 &size,
                                        const Color &color = Color::WHITE,
                                        bool filled = true) {
    MeshInstance2D mesh(size, color, filled);
    return mesh;
  }

  // Factory: creates a Circle 2D mesh
  static MeshInstance2D createCircle(float radius,
                                     const Color &color = Color::WHITE,
                                     bool filled = true, int segments = 32) {
    MeshInstance2D mesh(radius, color, filled, segments);
    return mesh;
  }

  // Factory: creates a Triangle 2D mesh
  static MeshInstance2D createTriangle(const Vector2 &p1, const Vector2 &p2,
                                       const Vector2 &p3,
                                       const Color &color = Color::WHITE,
                                       bool filled = true) {
    MeshInstance2D mesh;
    mesh.meshType = Mesh2DType::Triangle;
    mesh.p1 = p1;
    mesh.p2 = p2;
    mesh.p3 = p3;
    mesh.color = color;
    mesh.filled = filled;
    return mesh;
  }

  // Factory: creates a Polygon 2D mesh
  static MeshInstance2D createPolygon(const std::vector<Vector2> &verts,
                                      const Color &color = Color::WHITE,
                                      bool filled = true) {
    MeshInstance2D mesh;
    mesh.meshType = Mesh2DType::Polygon;
    mesh.vertices = verts;
    mesh.color = color;
    mesh.filled = filled;
    return mesh;
  }

  // Direct render helper with explicit transform
  void draw(const Vector2 &position, float rotation = 0.0f,
            const Vector2 &scale = {1.0f, 1.0f}) const {
    switch (meshType) {
    case Mesh2DType::Rectangle: {
      Vector2 scaledSize = Vector2(size.x * scale.x, size.y * scale.y);
      if (rotation == 0.0f) {
        Renderer2D::drawRect(position - scaledSize * 0.5f, scaledSize, color, filled);
      } else {
        Renderer2D::drawRectRotated(position - scaledSize * 0.5f, scaledSize,
                                    rotation, color, filled);
      }
      break;
    }

    case Mesh2DType::Circle: {
      Renderer2D::drawCircle(position, radius * std::max(scale.x, scale.y), color,
                             filled, segments);
      break;
    }

    case Mesh2DType::Triangle: {
      Vector2 tp1 = position + p1.rotated(rotation);
      Vector2 tp2 = position + p2.rotated(rotation);
      Vector2 tp3 = position + p3.rotated(rotation);
      Renderer2D::drawTriangle(tp1, tp2, tp3, color, filled);
      break;
    }

    case Mesh2DType::Polygon: {
      if (vertices.size() >= 2) {
        std::vector<Vector2> transformed;
        transformed.reserve(vertices.size());
        for (const auto &v : vertices) {
          transformed.push_back(position + v.rotated(rotation));
        }
        Renderer2D::drawLines(transformed, color, true);
      }
      break;
    }
    }
  }

  // Renders this mesh automatically when part of the active scene tree.
  void onDraw() override {
    if (!visible) return;
    Transform2D global = getGlobalTransform();
    draw(global.position, global.rotation, global.scale);
  }
};

// Backwards-compatibility alias
using Shape2D = MeshInstance2D;
