/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sprite_io.hpp
 */
#pragma once

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/tile_map_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, graphics::TileMapOptions>
{
  void operator()(Archive& ar, const graphics::TileMapOptions& options) const;
};

template <typename Archive> struct load<Archive, graphics::TileMapOptions>
{
  void operator()(Archive& ar, graphics::TileMapOptions& options) const;
};

template <typename Archive> struct save<Archive, graphics::TileMap>
{
  void operator()(Archive& ar, const graphics::TileMap& tile_map) const;
};

template <typename Archive> struct load<Archive, graphics::TileMap>
{
  void operator()(Archive& ar, graphics::TileMap& tile_map) const;
};

}  // namespace sde::serial
