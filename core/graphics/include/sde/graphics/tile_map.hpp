/**
 * @copyright 2024-present Brian Cairl
 *
 * @file tile_map.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/geometry_types.hpp"

namespace sde::graphics
{
class TileSet;

struct TileMap
{
  static constexpr std::size_t kDim = 8;
  static constexpr std::size_t kTileCount = kDim * kDim;

  Vec2f position = Vec2f::Zero();
  Vec2f tile_size = Vec2f::Zero();
  Vec4f color = Vec4f::Ones();
  Mat<std::size_t, kDim> tiles;
  std::size_t atlas_texture_unit = 0;
  TileSet* tile_set = nullptr;
};

inline Vec2f getNextRightStartPosition(const TileMap& tm)
{
  return tm.position + Vec2f{tm.tile_size.x() * TileMap::kDim, 0.0F};
}

std::ostream& operator<<(std::ostream& os, const TileMap& tile_map);

}  // namespace sde::graphics
