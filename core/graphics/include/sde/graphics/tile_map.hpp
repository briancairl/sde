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
#include "sde/geometry_utils.hpp"

namespace sde::graphics
{

struct TileMap
{
  static constexpr std::size_t kDim = 8;
  static constexpr std::size_t kTileCount = kDim * kDim;

  Vec2f position = Vec2f::Zero();
  Vec2f tile_size = Vec2f::Zero();
  Vec4f color = Vec4f::Ones();
  Mat<std::size_t, kDim> tiles;
  std::size_t texture_unit;

  Bounds2f bounds() const
  {
    const Vec2f p_min = position;
    const Vec2f p_max = (position + Vec2f{tile_size.x() * tiles.cols(), -tile_size.y() * tiles.rows()});
    return toBounds(p_min, p_max);
  }
};

inline Vec2f getNextRightPosition(const TileMap& tm)
{
  return tm.position + Vec2f{tm.tile_size.x() * TileMap::kDim, 0.0F};
}

std::ostream& operator<<(std::ostream& os, const TileMap& tile_map);

}  // namespace sde::graphics
