// C++ Standard Library
#include <algorithm>
#include <ostream>

// SDE
#include "sde/graphics/texture.hpp"
#include "sde/graphics/texture_units.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/tile_set.hpp"

namespace sde::graphics
{

expected<TileMapInfo, TileMapInfoError> TileMapInfo::create(const TileSet& tile_set, const TextureUnits& texture_units)
{
  const auto slot_itr = std::find(std::begin(texture_units), std::end(texture_units), tile_set.atlas());

  if (slot_itr == std::end(texture_units))
  {
    return unexpected<TileMapInfoError>{TileMapInfoError::kAtlasTextureNotLoaded};
  }

  TileMapInfo info;
  info.texture_unit_ = static_cast<std::size_t>(std::distance(slot_itr, std::begin(texture_units)));
  info.tile_set_ = std::addressof(tile_set);
  return info;
}

std::ostream& operator<<(std::ostream& os, TileMapInfoError error)
{
  switch (error)
  {
  case TileMapInfoError::kAtlasTextureNotLoaded:
    return os << "AtlasTextureNotLoaded";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const TileMapInfo& tile_map_info)
{
  return os << "{ texture-unit: " << tile_map_info.getTextureUnit() << " }";
}

std::ostream& operator<<(std::ostream& os, const TileMap& tile_map)
{
  os << "position: " << tile_map.position.transpose() << '\n';
  os << "tile-size: " << tile_map.tile_size.transpose() << '\n';
  os << "tiles:\n" << tile_map.tiles;
  return os;
}

}  // namespace sde::graphics
