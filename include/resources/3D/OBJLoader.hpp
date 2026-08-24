#pragma once

#include "components/Components3D.hpp"
#include "nodes/3D/AABB.hpp"
#include "nodes/3D/MeshInstance3D.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// High-performance Wavefront OBJ 3D Model Parser for MelkamEngine
class OBJLoader {
public:
  // Resolves file path across standard project directories
  static std::string resolvePath(const std::string &path) {
    std::string cleanPath = path;
    if (cleanPath.rfind("res://", 0) == 0) {
      cleanPath = cleanPath.substr(6);
    }
    if (std::filesystem::exists(cleanPath)) return cleanPath;
    if (std::filesystem::exists("../" + cleanPath)) return "../" + cleanPath;
    if (std::filesystem::exists("../../" + cleanPath)) return "../../" + cleanPath;
    if (std::filesystem::exists("../../../" + cleanPath)) return "../../../" + cleanPath;
    return cleanPath;
  }

  // Loads a Wavefront .obj file and returns a complete Mesh3D representation
  static Mesh3D load(const std::string &filePath) {
    Mesh3D mesh;
    std::string resolved = resolvePath(filePath);

    std::ifstream file(resolved);
    if (!file.is_open()) {
      std::cerr << "[OBJLoader Error] Could not open OBJ file: " << filePath << " (Resolved: " << resolved << ")" << std::endl;
      return mesh;
    }

    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texCoords;

    struct VertexIndex {
      int posIdx = -1;
      int uvIdx = -1;
      int normIdx = -1;

      bool operator==(const VertexIndex &other) const {
        return posIdx == other.posIdx && uvIdx == other.uvIdx && normIdx == other.normIdx;
      }
    };

    struct VertexIndexHash {
      size_t operator()(const VertexIndex &v) const {
        return (std::hash<int>()(v.posIdx) ^ (std::hash<int>()(v.uvIdx) << 1)) ^ (std::hash<int>()(v.normIdx) << 2);
      }
    };

    std::unordered_map<VertexIndex, uint32_t, VertexIndexHash> uniqueVertices;
    std::string line;

    while (std::getline(file, line)) {
      if (line.empty() || line[0] == '#') continue;

      std::istringstream ss(line);
      std::string prefix;
      ss >> prefix;

      if (prefix == "v") {
        float x, y, z;
        ss >> x >> y >> z;
        positions.emplace_back(x, y, z);
      } else if (prefix == "vn") {
        float nx, ny, nz;
        ss >> nx >> ny >> nz;
        normals.emplace_back(nx, ny, nz);
      } else if (prefix == "vt") {
        float u, v;
        ss >> u >> v;
        texCoords.emplace_back(u, 1.0f - v); // Invert V for standard OpenGL/Godot texture UVs
      } else if (prefix == "f") {
        std::vector<VertexIndex> faceVertices;
        std::string vertToken;

        while (ss >> vertToken) {
          VertexIndex vi;
          size_t firstSlash = vertToken.find('/');
          if (firstSlash == std::string::npos) {
            vi.posIdx = std::stoi(vertToken) - 1;
          } else {
            vi.posIdx = std::stoi(vertToken.substr(0, firstSlash)) - 1;
            size_t secondSlash = vertToken.find('/', firstSlash + 1);
            if (secondSlash == std::string::npos) {
              std::string uvStr = vertToken.substr(firstSlash + 1);
              if (!uvStr.empty()) vi.uvIdx = std::stoi(uvStr) - 1;
            } else {
              std::string uvStr = vertToken.substr(firstSlash + 1, secondSlash - firstSlash - 1);
              if (!uvStr.empty()) vi.uvIdx = std::stoi(uvStr) - 1;
              std::string normStr = vertToken.substr(secondSlash + 1);
              if (!normStr.empty()) vi.normIdx = std::stoi(normStr) - 1;
            }
          }
          faceVertices.push_back(vi);
        }

        // Triangulate face (Fan triangulation: 0, i, i+1)
        for (size_t i = 1; i + 1 < faceVertices.size(); ++i) {
          VertexIndex triangle[3] = {faceVertices[0], faceVertices[i], faceVertices[i + 1]};

          for (int j = 0; j < 3; ++j) {
            const VertexIndex &vi = triangle[j];
            auto it = uniqueVertices.find(vi);
            if (it != uniqueVertices.end()) {
              mesh.indices.push_back(it->second);
            } else {
              uint32_t newIndex = static_cast<uint32_t>(mesh.vertices.size());
              Vector3 pos = (vi.posIdx >= 0 && vi.posIdx < static_cast<int>(positions.size())) ? positions[vi.posIdx] : Vector3(0, 0, 0);
              Vector3 norm = (vi.normIdx >= 0 && vi.normIdx < static_cast<int>(normals.size())) ? normals[vi.normIdx].normalized() : Vector3(0, 1, 0);
              Vector2 uv = (vi.uvIdx >= 0 && vi.uvIdx < static_cast<int>(texCoords.size())) ? texCoords[vi.uvIdx] : Vector2(0, 0);

              mesh.vertices.emplace_back(pos, norm, uv, Color::WHITE);
              mesh.indices.push_back(newIndex);
              uniqueVertices[vi] = newIndex;
            }
          }
        }
      }
    }

    // Generate normals if missing
    if (normals.empty() && !mesh.indices.empty()) {
      for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        Vertex3D &v0 = mesh.vertices[mesh.indices[i]];
        Vertex3D &v1 = mesh.vertices[mesh.indices[i + 1]];
        Vertex3D &v2 = mesh.vertices[mesh.indices[i + 2]];
        Vector3 fn = (v1.position - v0.position).cross(v2.position - v0.position).normalized();
        v0.normal = (v0.normal + fn).normalized();
        v1.normal = (v1.normal + fn).normalized();
        v2.normal = (v2.normal + fn).normalized();
      }
    }

    // Compute bounding AABB
    if (!mesh.vertices.empty()) {
      Vector3 minP = mesh.vertices[0].position;
      Vector3 maxP = mesh.vertices[0].position;
      for (const auto &v : mesh.vertices) {
        minP.x = std::min(minP.x, v.position.x);
        minP.y = std::min(minP.y, v.position.y);
        minP.z = std::min(minP.z, v.position.z);
        maxP.x = std::max(maxP.x, v.position.x);
        maxP.y = std::max(maxP.y, v.position.y);
        maxP.z = std::max(maxP.z, v.position.z);
      }
      mesh.aabb = AABB(minP, maxP - minP);
    }

    std::cout << "[OBJLoader] Successfully loaded '" << filePath << "' (" 
              << mesh.vertices.size() << " vertices, " << mesh.indices.size() / 3 << " triangles)" << std::endl;

    return mesh;
  }
};
