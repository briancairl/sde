// C++ Standard Library
#include <algorithm>
#include <ostream>

// SDE
#include "sde/geometry_utils.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/logging.hpp"

namespace sde::graphics
{
namespace
{}  // namespace

std::ostream& operator<<(std::ostream& os, TileSetError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(TileSetError::kElementAlreadyExists)
    SDE_OS_ENUM_CASE(TileSetError::kInvalidHandle)
    SDE_OS_ENUM_CASE(TileSetError::kAssetNotFound)
    SDE_OS_ENUM_CASE(TileSetError::kInvalidAtlasTexture)
    SDE_OS_ENUM_CASE(TileSetError::kInvalidTileSize)
    SDE_OS_ENUM_CASE(TileSetError::kInvalidSlicingBounds)
  }
  return os;
}

expected<TileSet, TileSetError>
TileSetCache::generate(dependencies deps, const TextureHandle& texture, sde::vector<Rect2f>&& tile_bounds)
{
  if (!deps.get<TextureCache>().exists(texture))
  {
    return make_unexpected(TileSetError::kInvalidAtlasTexture);
  }
  return TileSet{.tile_atlas = texture, .tile_bounds = std::move(tile_bounds)};
}

expected<TileSet, TileSetError>
TileSetCache::generate(dependencies deps, const TextureHandle& texture, const TileSetSliceUniform& slice)
{
  const auto* texture_info = deps.get<TextureCache>().get_if(texture);
  if (texture_info == nullptr)
  {
    SDE_LOG_ERROR() << "InvalidAtlasTexture: " << SDE_OSNV(texture);
    return make_unexpected(TileSetError::kInvalidAtlasTexture);
  }

  const Bounds2i texture_bounds{Vec2i::Zero(), texture_info->shape.value};
  const Bounds2i sliced_region = slice.bounds_px.isEmpty() ? texture_bounds : (slice.bounds_px & texture_bounds);

  const std::size_t tile_count_max = (toExtents(sliced_region).array() / slice.tile_size_px.array()).prod();

  if (tile_count_max == 0)
  {
    SDE_LOG_DEBUG_FMT(
      "InvalidTileSize: sliced_region=(%d,%d,%d,%d)",
      sliced_region.min().x(),
      sliced_region.min().y(),
      sliced_region.max().x(),
      sliced_region.max().y());
    return make_unexpected(TileSetError::kInvalidTileSize);
  }

  const Vec2f axis_rates{1.0F / texture_info->shape.value.array().cast<float>()};

  const auto toTileBounds = [&slice, &axis_rates](int x_lb, int y_lb, int x_ub, int y_ub) -> Rect2f {
    if (
      (slice.tile_orientation_x == TileOrientation::kFlipped) and
      (slice.tile_orientation_y == TileOrientation::kFlipped))
    {
      return Rect2f{
        Vec2f{static_cast<float>(x_ub), static_cast<float>(y_ub)}.array() * axis_rates.array(),
        Vec2f{static_cast<float>(x_lb), static_cast<float>(y_lb)}.array() * axis_rates.array()};
    }
    else if (slice.tile_orientation_x == TileOrientation::kFlipped)
    {
      return Rect2f{
        Vec2f{static_cast<float>(x_ub), static_cast<float>(y_lb)}.array() * axis_rates.array(),
        Vec2f{static_cast<float>(x_lb), static_cast<float>(y_ub)}.array() * axis_rates.array()};
    }
    else if (slice.tile_orientation_y == TileOrientation::kFlipped)
    {
      return Rect2f{
        Vec2f{static_cast<float>(x_lb), static_cast<float>(y_ub)}.array() * axis_rates.array(),
        Vec2f{static_cast<float>(x_ub), static_cast<float>(y_lb)}.array() * axis_rates.array()};
    }
    else
    {
      return Rect2f{
        Vec2f{static_cast<float>(x_lb), static_cast<float>(y_lb)}.array() * axis_rates.array(),
        Vec2f{static_cast<float>(x_ub), static_cast<float>(y_ub)}.array() * axis_rates.array()};
    }
  };

  const int x_min = sliced_region.min().x() + slice.offset_px.x();
  const int y_min = sliced_region.min().y() + slice.offset_px.y();
  const int x_step = slice.tile_size_px.x() + slice.skip_px.x();
  const int y_step = slice.tile_size_px.y() + slice.skip_px.y();

  std::size_t skip_countdown = slice.start_offset;

  sde::vector<Rect2f> tile_bounds;
  tile_bounds.reserve(tile_count_max);
  if (slice.direction == TileSliceDirection::kColWise)
  {
    for (int x_lb = x_min; x_lb < sliced_region.max().x(); x_lb += x_step)
    {
      const int x_ub = x_lb + slice.tile_size_px.x();
      for (int y_lb = y_min; y_lb < sliced_region.max().y(); y_lb += y_step)
      {
        const int y_ub = y_lb + slice.tile_size_px.y();
        if (skip_countdown == 0UL)
        {
          tile_bounds.emplace_back(toTileBounds(x_lb, y_lb, x_ub, y_ub));
        }
        else
        {
          --skip_countdown;
          continue;
        }

        if ((slice.stop_after > 0UL) and (tile_bounds.size() == slice.stop_after))
        {
          return TileSet{.tile_atlas = texture, .tile_bounds = std::move(tile_bounds)};
        }
      }
    }
  }
  else
  {
    for (int y_lb = y_min; y_lb < sliced_region.max().y(); y_lb += y_step)
    {
      const int y_ub = y_lb + slice.tile_size_px.y();
      for (int x_lb = x_min; x_lb < sliced_region.max().x(); x_lb += x_step)
      {
        const int x_ub = x_lb + slice.tile_size_px.x();
        if (skip_countdown == 0UL)
        {
          tile_bounds.emplace_back(toTileBounds(x_lb, y_lb, x_ub, y_ub));
        }
        else
        {
          --skip_countdown;
          continue;
        }

        if ((slice.stop_after > 0UL) and (tile_bounds.size() == slice.stop_after))
        {
          return TileSet{.tile_atlas = texture, .tile_bounds = std::move(tile_bounds)};
        }
      }
    }
  }

  return TileSet{.tile_atlas = texture, .tile_bounds = std::move(tile_bounds)};
}

}  // namespace sde::graphics
