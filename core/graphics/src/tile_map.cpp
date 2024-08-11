// C++ Standard Library
#include <algorithm>
#include <ostream>

// SDE
#include "sde/geometry_utils.hpp"
#include "sde/graphics/assets.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/tile_map.hpp"

namespace sde::graphics
{

TileMap::TileMap(const TileMapOptions& options) { this->setup(options); }

TileMap::TileMap(TileMap&& other) : tile_indices_{} { this->swap(other); }

TileMap& TileMap::operator=(TileMap&& other)
{
  this->swap(other);
  return *this;
}

void TileMap::swap(TileMap& other)
{
  std::swap(options_, other.options_);
  std::swap(tile_indices_, other.tile_indices_);
}

void TileMap::setup(const TileMapOptions& options)
{
  options_ = options;
  if (const std::size_t new_tile_count = static_cast<std::size_t>(options_.shape.prod()); new_tile_count > 0UL)
  {
    tile_indices_.resize(new_tile_count);
  }
}

void TileMap::draw(RenderPass& rp, const Vec2f& origin) const
{
  const Bounds2f aabb_clipped{rp.getViewportInWorldBounds() & Bounds2f{origin, origin + options_.mapSize()}};
  if (aabb_clipped.volume() == 0)
  {
    return;
  }

  const auto tile_set = rp.assets().tile_sets(options_.tile_set);
  if (!tile_set)
  {
    return;
  }

  const auto texture_unit_opt = rp.assign(tile_set->tile_atlas);
  if (!texture_unit_opt.has_value())
  {
    return;
  }

  const Vec2i min_indices = ((aabb_clipped.min() - origin).array() / options_.tile_size.array()).floor().cast<int>();
  const Vec2i max_indices = ((aabb_clipped.max() - origin).array() / options_.tile_size.array()).ceil().cast<int>();

  for (int y = min_indices.y(); y < max_indices.y(); ++y)
  {
    for (int x = min_indices.x(); x < max_indices.x(); ++x)
    {
      const Vec2i tile_coords{x, y};
      const TileIndex tile_index = (*this)[tile_coords];

      const Vec2f rect_min{origin.array() + tile_coords.array().cast<float>() * options_.tile_size.array()};
      const Vec2f rect_max{rect_min + options_.tile_size};

      rp->textured_quads.push_back(
        {.rect = Rect2f{rect_max, rect_min},
         .rect_texture = tile_set->tile_bounds[tile_index],
         .color = options_.tint_color,
         .texture_unit = (*texture_unit_opt)});
    }
  }
}

}  // namespace sde::graphics
