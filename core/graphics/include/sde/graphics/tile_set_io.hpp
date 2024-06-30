/**
 * @copyright 2024-present Brian Cairl
 *
 * @file tileset_io.hpp
 */
#pragma once

// SDE
#include "sde/graphics/tile_set.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive>
struct save<Archive, graphics::TileSetHandle> : save<Archive, typename graphics::TileSetHandle::rh_type>
{};

template <typename Archive>
struct load<Archive, graphics::TileSetHandle> : load<Archive, typename graphics::TileSetHandle::rh_type>
{};

}  // namespace sde::serial
