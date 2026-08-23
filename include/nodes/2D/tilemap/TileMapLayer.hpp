#pragma once

#include "core/Memory.hpp"
#include "helper/color/Color.hpp"
#include "helper/Rect2.hpp"
#include "helper/vectors/Vector2.hpp"
#include "nodes/2D/Camera2D.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/physics/CollisionShape2D.hpp"
#include "nodes/2D/physics/StaticBody2D.hpp"
#include "nodes/2D/tilemap/TileSet.hpp"
#include "renderers/Renderer2D.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <utility>
#include <vector>

struct TileCell {
  int tileId = 0;
  Vector2 atlasCoords{0.0f, 0.0f};
  bool solid = false;
  bool useAtlas = false;
  Color tint = Color::WHITE;
};

// 2D TileMap Layer Node (inspired by Godot 4 TileMapLayer / TileMap)
class TileMapLayer : public Node2D {
public:
  Ref<TileSet> tileSet = nullptr;
  bool collisionEnabled = true;
  uint32_t collisionLayer = 1;
  uint32_t collisionMask = 1;

  TileMapLayer() : Node2D("TileMapLayer") {}

  explicit TileMapLayer(Ref<TileSet> set) : Node2D("TileMapLayer"), tileSet(std::move(set)) {}

  void setCell(int x, int y, int tileId) {
    TileCell cell;
    cell.tileId = tileId;
    cell.useAtlas = false;
    if (tileSet) {
      cell.solid = tileSet->isTileSolid(tileId);
    }
    m_cells[{x, y}] = cell;
    m_collidersDirty = true;
  }

  void setCell(int x, int y, const Vector2 &atlasCoords, bool solid = false, const Color &tint = Color::WHITE) {
    TileCell cell;
    cell.atlasCoords = atlasCoords;
    cell.solid = solid;
    cell.useAtlas = true;
    cell.tint = tint;
    m_cells[{x, y}] = cell;
    m_collidersDirty = true;
  }

  void eraseCell(int x, int y) {
    m_cells.erase({x, y});
    m_collidersDirty = true;
  }

  bool hasCell(int x, int y) const {
    return m_cells.find({x, y}) != m_cells.end();
  }

  const TileCell *getCell(int x, int y) const {
    auto it = m_cells.find({x, y});
    if (it != m_cells.end()) return &it->second;
    return nullptr;
  }

  size_t getCellCount() const {
    return m_cells.size();
  }

  void clear() {
    m_cells.clear();
    m_collidersDirty = true;
  }

  Vector2 mapToLocal(int x, int y) const {
    Vector2 sz = tileSet ? tileSet->tileSize : Vector2(32.0f, 32.0f);
    return Vector2((x + 0.5f) * sz.x, (y + 0.5f) * sz.y);
  }

  std::pair<int, int> localToMap(const Vector2 &localPos) const {
    Vector2 sz = tileSet ? tileSet->tileSize : Vector2(32.0f, 32.0f);
    int x = static_cast<int>(std::floor(localPos.x / sz.x));
    int y = static_cast<int>(std::floor(localPos.y / sz.y));
    return {x, y};
  }

  void onReady() override {
    updateColliders();
  }

  void onProcess(float delta) override {
    (void)delta;
    if (m_collidersDirty) {
      updateColliders();
    }
  }

  // Generates optimized greedy horizontal static colliders for all solid tiles (Godot 4 Parity)
  void updateColliders() {
    m_collidersDirty = false;
    if (!collisionEnabled) {
      for (auto &body : m_colliderBodies) {
        removeChild(body);
      }
      m_colliderBodies.clear();
      return;
    }

    // Remove old collider bodies
    for (auto &body : m_colliderBodies) {
      removeChild(body);
    }
    m_colliderBodies.clear();

    if (m_cells.empty()) return;

    Vector2 sz = tileSet ? tileSet->tileSize : Vector2(32.0f, 32.0f);

    // Group solid cells by row Y
    std::map<int, std::vector<int>> solidRows;
    for (const auto &[coord, cell] : m_cells) {
      if (cell.solid) {
        solidRows[coord.second].push_back(coord.first);
      }
    }

    // Merge contiguous horizontal tiles into spans per row
    for (auto &[y, xCols] : solidRows) {
      std::sort(xCols.begin(), xCols.end());

      size_t i = 0;
      while (i < xCols.size()) {
        int xStart = xCols[i];
        int xEnd = xStart;

        while (i + 1 < xCols.size() && xCols[i + 1] == xEnd + 1) {
          xEnd = xCols[i + 1];
          ++i;
        }

        int count = (xEnd - xStart + 1);
        float spanWidth = count * sz.x;
        float spanHeight = sz.y;
        Vector2 spanCenter = Vector2((xStart + count * 0.5f) * sz.x, (y + 0.5f) * sz.y);

        auto body = addChild<StaticBody2D>("TileMapSpan_" + std::to_string(y) + "_" + std::to_string(xStart));
        body->setPosition(spanCenter);
        body->collisionLayer = collisionLayer;
        body->collisionMask = collisionMask;
        body->addChild<CollisionShape2D>(Vector2(spanWidth, spanHeight));
        m_colliderBodies.push_back(body);

        ++i;
      }
    }
  }

  void onDraw() override {
    if (!tileSet || !tileSet->texture || m_cells.empty()) return;

    Transform2D globalTransform = getGlobalTransform();
    Vector2 sz = tileSet->tileSize;
    const Texture2D &tex = *tileSet->texture;

    for (const auto &[coord, cell] : m_cells) {
      Vector2 localPos = Vector2(coord.first * sz.x, coord.second * sz.y);
      Vector2 worldPos = globalTransform.transformPoint(localPos);

      Rect2 srcRect = cell.useAtlas ? tileSet->getTileSourceRect(cell.atlasCoords)
                                    : tileSet->getTileSourceRect(cell.tileId);

      Renderer2D::drawTextureRegion(tex, srcRect, worldPos, sz, cell.tint, globalTransform.rotation);
    }
  }

private:
  std::map<std::pair<int, int>, TileCell> m_cells;
  std::vector<std::shared_ptr<StaticBody2D>> m_colliderBodies;
  bool m_collidersDirty = false;
};

using TileMap = TileMapLayer;
