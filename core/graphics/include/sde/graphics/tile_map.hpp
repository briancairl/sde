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
  Vec2f position = Vec2f::Zero();
  Vec2f tile_size = Vec2f::Zero();
  Mat<std::size_t, 16> tiles;
  TileSet* tile_set = nullptr;
};

std::ostream& operator<<(std::ostream& os, const TileMap& tile_map);

}  // namespace sde::graphics
