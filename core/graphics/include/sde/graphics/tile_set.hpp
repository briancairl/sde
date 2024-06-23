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
#include "sde/graphics/texture.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/tile_set_fwd.hpp"
#include "sde/graphics/tile_set_handle.hpp"
#include "sde/resource_cache.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

enum TileSetError
{
  kElementAlreadyExists,
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

enum class TileSliceDirection
{
  kColWise,
  kRowWise,
};

enum class TileOrientation
{
  kNormal,
  kFlipped,
};


struct TileSetSliceUniform
{
  Vec2i tile_size_px = {};
  TileOrientation tile_orientation_x = TileOrientation::kNormal;
  TileOrientation tile_orientation_y = TileOrientation::kNormal;
  TileSliceDirection direction = TileSliceDirection::kColWise;
  std::size_t start_offset = 0UL;
  std::size_t stop_after = 0UL;
  Bounds2i bounds_px = {};
  Vec2i offset_px = Vec2i::Zero();
  Vec2i skip_px = Vec2i::Zero();
};

class TileSetCache : public ResourceCache<TileSetCache>
{
  friend class ResourceCache<TileSetCache>;

private:
  expected<TileSetInfo, TileSetError>
  generate(const TextureCache& texture_cache, const TextureHandle& texture, const TileSetSliceUniform& slice);

  expected<TileSetInfo, TileSetError>
  generate(const element_t<TextureCache>& texture, const TileSetSliceUniform& slice);
};

}  // namespace sde::graphics
