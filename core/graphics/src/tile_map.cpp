// C++ Standard Library
#include <algorithm>
#include <ostream>

// SDE
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_map.hpp"

namespace sde::graphics
{

std::ostream& operator<<(std::ostream& os, const TileMap& tile_map)
{
  os << "position: " << tile_map.position.transpose() << '\n';
  os << "tile-size: " << tile_map.tile_size.transpose() << '\n';
  os << "tiles:\n" << tile_map.tiles;
  return os;
}

}  // namespace sde::graphics
