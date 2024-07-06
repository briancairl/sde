// C++ Standard Library
#include <ostream>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/tile_map_io.hpp"
#include "sde/graphics/tile_set_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::TileMapOptions>::operator()(Archive& ar, const graphics::TileMapOptions& options) const
{
  ar << named{"tint_color", options.tint_color};
  ar << named{"shape", options.shape};
  ar << named{"tile_size", options.tile_size};
  ar << named{"tile_set", options.tile_set};
}

template <typename Archive>
void load<Archive, graphics::TileMapOptions>::operator()(Archive& ar, graphics::TileMapOptions& options) const
{
  ar >> named{"tint_color", options.tint_color};
  ar >> named{"shape", options.shape};
  ar >> named{"tile_size", options.tile_size};
  ar >> named{"tile_set", options.tile_set};
}

template <typename Archive>
void save<Archive, graphics::TileMap>::operator()(Archive& ar, const graphics::TileMap& tile_map) const
{
  ar << named{"options", tile_map.options()};
  ar << named{"data", make_packet(tile_map.data().data(), tile_map.data().size())};
}

template <typename Archive>
void load<Archive, graphics::TileMap>::operator()(Archive& ar, graphics::TileMap& tile_map) const
{
  graphics::TileMapOptions options;
  ar >> named{"options", options};
  tile_map.setup(options);
  ar >> named{"data", make_packet(tile_map.data().data(), tile_map.data().size())};
}

template struct save<binary_ofarchive, graphics::TileMap>;
template struct load<binary_ifarchive, graphics::TileMap>;
template struct save<binary_ofarchive, graphics::TileMapOptions>;
template struct load<binary_ifarchive, graphics::TileMapOptions>;

}  // namespace sde::serial
