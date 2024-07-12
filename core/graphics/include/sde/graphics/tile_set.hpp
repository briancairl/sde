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
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

enum TileSetError
{
  kInvalidHandle,
  kAssetNotFound,
  kElementAlreadyExists,
  kInvalidAtlasTexture,
  kInvalidTileSize,
  kInvalidSlicingBounds,
};

std::ostream& operator<<(std::ostream& os, TileSetError tile_set_error);

struct TileSet : Resource<TileSet>
{
  TextureHandle tile_atlas;
  std::vector<Bounds2f> tile_bounds;

  auto fields_list() { return std::make_tuple((Field{"tile_atlas", tile_atlas}), (Field{"tile_bounds", tile_bounds})); }

  const Bounds2f& operator[](const std::size_t index) const { return tile_bounds[index]; };
};

std::ostream& operator<<(std::ostream& os, const TileSet& tile_set_info);

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

struct TileSetSliceUniform : Resource<TileSetSliceUniform>
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

  auto fields_list()
  {
    return std::make_tuple(
      (Field{"tile_size_px", tile_size_px}),
      (Field{"tile_orientation_x", tile_orientation_x}),
      (Field{"tile_orientation_y", tile_orientation_y}),
      (Field{"direction", direction}),
      (Field{"start_offset", start_offset}),
      (Field{"stop_after", stop_after}),
      (Field{"bounds_px", bounds_px}),
      (Field{"offset_px", offset_px}),
      (Field{"skip_px", skip_px}));
  }
};

}  // namespace sde::graphics

namespace sde
{

template <> struct Hasher<graphics::TileSetHandle> : ResourceHandleHash
{};
template <> struct Hasher<graphics::TileSetSliceUniform> : ResourceHasher
{};
template <> struct Hasher<graphics::TileSet> : ResourceHasher
{};

template <> struct ResourceCacheTypes<graphics::TileSetCache>
{
  using error_type = graphics::TileSetError;
  using handle_type = graphics::TileSetHandle;
  using value_type = graphics::TileSet;
};

}  // namespace sde

namespace sde::graphics
{

class TileSetCache : public ResourceCache<TileSetCache>
{
  friend cache_base;

public:
  explicit TileSetCache(TextureCache& textures);

private:
  TextureCache* textures_;

  expected<TileSet, TileSetError> generate(const TextureHandle& texture, const TileSetSliceUniform& slice);

  expected<TileSet, TileSetError> generate(const TextureHandle& texture, std::vector<Bounds2f>&& tile_bounds);
};

}  // namespace sde::graphics
