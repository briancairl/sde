/**
 * @copyright 2024-present Brian Cairl
 *
 * @file tile_set_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::graphics
{

struct TileSetHandle : ResourceHandle<TileSetHandle>
{
  TileSetHandle() = default;
  explicit TileSetHandle(id_type id) : ResourceHandle<TileSetHandle>{id} {}
};

}  // namespace sde::graphics

namespace sde
{
template <> struct Hasher<graphics::TileSetHandle> : ResourceHandleHash
{};
}  // namespace sde
