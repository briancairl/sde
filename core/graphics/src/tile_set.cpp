// C++ Standard Library
#include <algorithm>
#include <ostream>

// SDE
#include "sde/graphics/tile_set.hpp"

namespace sde::graphics
{
namespace
{}  // namespace

std::ostream& operator<<(std::ostream& os, TileSetError error)
{
  switch (error)
  {
  case TileSetError::kInvalidAtlasTexture:
    return os << "InvalidAtlasTexture";
  case TileSetError::kInvalidTileSize:
    return os << "InvalidTileSize";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const TileSet& tile_set)
{
  os << "{ ";
  for (std::size_t tile = 0; tile < tile_set.size(); ++tile)
  {
    os << '[' << tile << "] : " << tile_set[tile] << (((tile + 1) == tile_set.size()) ? ' ' : ',');
  }
  os << '}';
  return os;
}

}  // namespace sde::graphics
