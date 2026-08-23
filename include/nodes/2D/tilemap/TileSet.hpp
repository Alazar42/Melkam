#pragma once

#include "core/Memory.hpp"
#include "helper/color/Color.hpp"
#include "helper/Rect2.hpp"
#include "helper/vectors/Vector2.hpp"
#include "renderers/Texture2D.hpp"
#include <memory>
#include <string>
#include <unordered_map>

struct TileData {
  Vector2 atlasCoords{0.0f, 0.0f};
  bool solid = false;
  Color tint = Color::WHITE;
  int zIndex = 0;
};

// TileSet Resource (inspired by Godot TileSet) defining tile textures, atlas slicing, and collision properties.
class TileSet {
public:
  Ref<Texture2D> texture = nullptr;
  Vector2 tileSize{32.0f, 32.0f};
  Vector2 separation{0.0f, 0.0f};
  Vector2 margin{0.0f, 0.0f};

  TileSet() = default;

  explicit TileSet(Ref<Texture2D> tex, const Vector2 &size = {32.0f, 32.0f})
      : texture(std::move(tex)), tileSize(size) {}

  void setTileData(int tileId, const TileData &data) {
    m_tiles[tileId] = data;
  }

  const TileData *getTileData(int tileId) const {
    auto it = m_tiles.find(tileId);
    if (it != m_tiles.end()) return &it->second;
    return nullptr;
  }

  Rect2 getTileSourceRect(const Vector2 &atlasCoords) const {
    float x = margin.x + atlasCoords.x * (tileSize.x + separation.x);
    float y = margin.y + atlasCoords.y * (tileSize.y + separation.y);
    return Rect2(x, y, tileSize.x, tileSize.y);
  }

  Rect2 getTileSourceRect(int tileId) const {
    const TileData *data = getTileData(tileId);
    if (data) {
      return getTileSourceRect(data->atlasCoords);
    }
    return Rect2(0.0f, 0.0f, tileSize.x, tileSize.y);
  }

  bool isTileSolid(int tileId) const {
    const TileData *data = getTileData(tileId);
    return data ? data->solid : false;
  }

private:
  std::unordered_map<int, TileData> m_tiles;
};
