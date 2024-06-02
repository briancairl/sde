// C++ Standard Library
#include <algorithm>
#include <ostream>

// SDE
#include "sde/geometry_utils.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"

namespace sde::graphics
{
namespace
{}  // namespace

std::ostream& operator<<(std::ostream& os, TileSetError error)
{
  switch (error)
  {
  case TileSetError::kInvalidAtlasTexture:
    return os << "InvalidAtlasTexture";
  case TileSetError::kInvalidTileSize:
    return os << "InvalidTileSize";
  case TileSetError::kInvalidSlicingBounds:
    return os << "InvalidSlicingBounds";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const TileSet& tile_set)
{
  os << "tiles:\n";
  for (std::size_t tile = 0; tile < tile_set.size(); ++tile)
  {
    os << "  [" << tile << "] : " << tile_set[tile].min().transpose() << ", " << tile_set[tile].max().transpose()
       << '\n';
  }
  return os;
}

TileSet::TileSet(TextureHandle atlas_texture, std::vector<Bounds2f> tile_bounds) :
    atlas_texture_{atlas_texture}, tile_bounds_{std::move(tile_bounds)}
{}


expected<TileSet, TileSetError> TileSet::slice(
  const TextureHandle& texture,
  const TextureCache& texture_cache,
  const Vec2i tile_size,
  const Bounds2i& tile_slice_bounds)
{
  const auto* texture_info = texture_cache.get(texture);
  if (texture_info == nullptr)
  {
    return unexpected<TileSetError>{TileSetError::kInvalidAtlasTexture};
  }
  return TileSet::slice(texture, *texture_info, tile_size, tile_slice_bounds);
}

expected<TileSet, TileSetError> TileSet::slice(
  const TextureHandle& texture,
  const TextureInfo& texture_info,
  const Vec2i tile_size,
  const Bounds2i& tile_slice_bounds)
{
  if (texture.isNull())
  {
    return unexpected<TileSetError>{TileSetError::kInvalidAtlasTexture};
  }

  const Bounds2i texture_bounds{Vec2i::Zero(), texture_info.shape.value};
  const Bounds2i sliced_region = isEmpty(tile_slice_bounds) ? texture_bounds : (tile_slice_bounds & texture_bounds);

  const Vec2i tile_count = toExtents(sliced_region).array() / tile_size.array();

  if (tile_count.prod() == 0)
  {
    return unexpected<TileSetError>{TileSetError::kInvalidTileSize};
  }

  const Vec2f axis_rates{1.0F / texture_info.shape.value.array().cast<float>()};

  std::vector<Bounds2f> tile_bounds;
  tile_bounds.reserve(static_cast<std::size_t>(tile_count.prod()));
  for (int x_min = sliced_region.min().x(); x_min < sliced_region.max().x(); x_min += tile_size.x())
  {
    const int x_max = x_min + tile_size.x();
    for (int y_min = sliced_region.min().y(); y_min < sliced_region.max().y(); y_min += tile_size.y())
    {
      const int y_max = y_min + tile_size.y();
      tile_bounds.emplace_back(
        Vec2f{static_cast<float>(x_min), static_cast<float>(y_min)}.array() * axis_rates.array(),
        Vec2f{static_cast<float>(x_max), static_cast<float>(y_max)}.array() * axis_rates.array());
    }
  }

  return TileSet{texture, std::move(tile_bounds)};
}

}  // namespace sde::graphics
