/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sprite_io.hpp
 */
#pragma once

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/graphics/tile_set_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct serialize<Archive, graphics::TileMapOptions>
{
  void operator()(Archive& ar, graphics::TileMapOptions& options) const
  {
    ar& named{"tint_color", options.tint_color};
    ar& named{"shape", options.shape};
    ar& named{"tile_size", options.tile_size};
    ar& named{"tile_set", options.tile_set};
  }
};

template <typename Archive> struct save<Archive, graphics::TileMap>
{
  void operator()(Archive& ar, const graphics::TileMap& tile_map) const
  {
    ar << named{"options", tile_map.options()};
    ar << named{"data", make_packet(tile_map.data().data(), tile_map.data().size())};
  }
};

template <typename Archive> struct load<Archive, graphics::TileMap>
{
  void operator()(Archive& ar, graphics::TileMap& tile_map) const
  {
    graphics::TileMapOptions options;
    ar >> named{"options", options};
    tile_map.setup(options);
    ar >> named{"data", make_packet(tile_map.data().data(), tile_map.data().size())};
  }
};

}  // namespace sde::serial
