/**
 * @copyright 2024-present Brian Cairl
 *
 * @file tile_set_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/tile_set_fwd.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct save<Archive, graphics::TileSetHandle>
{
  void operator()(Archive& ar, const graphics::TileSetHandle& handle) const;
};

template <typename Archive> struct load<Archive, graphics::TileSetHandle>
{
  void operator()(Archive& ar, graphics::TileSetHandle& handle) const;
};

template <typename Archive> struct save<Archive, graphics::TileSetCache>
{
  void operator()(Archive& ar, const graphics::TileSetCache& cache) const;
};

template <typename Archive> struct load<Archive, graphics::TileSetCache>
{
  void operator()(Archive& ar, graphics::TileSetCache& cache) const;
};

}  // namespace sde::serial
