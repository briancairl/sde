/**
 * @copyright 2024-present Brian Cairl
 *
 * @file tile_set.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <vector>

// SDE
#include "sde/expected.hpp"

// SDE
#include "sde/geometry_types.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/tile_set_handle.hpp"
#include "sde/resource_cache.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

enum TileSetError
{
  kElementAlreadyExists,
  kInvalidAtlasTexture,
  kInvalidTileSize,
  kInvalidSlicingBounds,
};

std::ostream& operator<<(std::ostream& os, TileSetError tile_set_error);

struct TileSetInfo
{
  TextureHandle tile_atlas;
  std::vector<Bounds2f> tile_bounds;

  View<const Bounds2f> getBounds() const { return make_const_view(tile_bounds); }

  const Bounds2f& operator[](const std::size_t index) const { return tile_bounds[index]; };
};

std::ostream& operator<<(std::ostream& os, const TileSetInfo& tile_set_info);

class TileSetCache;

}  // namespace sde::graphics

namespace sde
{

template <> struct ResourceCacheTypes<graphics::TileSetCache>
{
  using error_type = graphics::TileSetError;
  using handle_type = graphics::TileSetHandle;
  using value_type = graphics::TileSetInfo;
};

}  // namespace sde

namespace sde::graphics
{

class TileSetCache : public ResourceCache<TileSetCache>
{
  friend class ResourceCache<TileSetCache>;

public:
  TileSetCache() = default;
  ~TileSetCache() = default;

private:
  expected<TileSetInfo, TileSetError> generate(
    const TextureHandle& texture,
    const TextureInfo& texture_info,
    const Vec2i tile_size,
    const Bounds2i& tile_slice_bounds = Bounds2i{});

  expected<TileSetInfo, TileSetError> generate(
    const TextureHandle& texture,
    const TextureCache& texture_cache,
    const Vec2i tile_size,
    const Bounds2i& tile_slice_bounds = Bounds2i{});
};

}  // namespace sde::graphics
