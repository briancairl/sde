/**
 * @copyright 2024-present Brian Cairl
 *
 * @file tile_set_fwd.hpp
 */
#pragma once

// SDE
#include "sde/graphics/texture_fwd.hpp"
#include "sde/resource_cache_traits.hpp"

namespace sde::graphics
{
enum class TileSetError;
struct TileSet;
struct TileSetHandle;
class TileSetCache;
}  // namespace sde::graphics

namespace sde
{
template <> struct ResourceCacheTraits<graphics::TileSetCache>
{
  using error_type = graphics::TileSetError;
  using handle_type = graphics::TileSetHandle;
  using value_type = graphics::TileSet;
  using dependencies = ResourceDependencies<graphics::TextureCache>;
};

template <> struct ResourceHandleToCache<graphics::TileSetHandle>
{
  using type = graphics::TileSetCache;
};
}  // namespace sde