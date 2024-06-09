// C++ Standard Library
#include <algorithm>
#include <ostream>

// SDE
#include "sde/geometry_utils.hpp"
#include "sde/graphics/assets.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/tile_map.hpp"

namespace sde::graphics
{
TileMap::TileMap(TileMap&& other) : tile_indices_{nullptr} { this->swap(other); }

TileMap& TileMap::operator=(TileMap&& other)
{
  this->swap(other);
  return *this;
}

void TileMap::swap(TileMap& other)
{
  std::swap(shape_, other.shape_);
  std::swap(origin_, other.origin_);
  std::swap(tile_size_, other.tile_size_);
  std::swap(tile_set_handle_, other.tile_set_handle_);
  std::swap(tile_indices_, other.tile_indices_);
}

TileMap::~TileMap()
{
  if (tile_indices_ == nullptr)
  {
    return;
  }
  delete[] tile_indices_;
}

TileMap TileMap::create(const TileSetHandle& tile_set, const Vec2i& shape, const Vec2f& origin, const Vec2f& tile_size)
{
  TileMap tm;
  tm.shape_ = shape;
  tm.origin_ = origin;
  tm.tile_size_ = tile_size;
  tm.tile_set_handle_ = tile_set;
  tm.tile_indices_ = new std::size_t[static_cast<std::size_t>(tm.shape_.prod())];
  return tm;
}

void TileMap::draw(RenderPass& rp, const Vec4f& tint) const
{
  const Bounds2f aabb_clipped{
    rp.getViewportInWorldBounds() &
    Bounds2f{origin_, origin_ + Vec2f{shape_.cast<float>().array() * tile_size_.array()}}};
  if (aabb_clipped.volume() == 0)
  {
    return;
  }

  const auto* tile_set = rp.assets().tile_sets(tile_set_handle_);
  if (tile_set == nullptr)
  {
    return;
  }

  const auto texture_unit_opt = rp.assign(tile_set->tile_atlas);
  if (!texture_unit_opt.has_value())
  {
    return;
  }

  static std::vector<TexturedQuad> s__textured_quad_buffer;

  const Vec2i min_indices = ((aabb_clipped.min() - origin_).array() / tile_size_.array()).floor().cast<int>();
  const Vec2i max_indices = ((aabb_clipped.max() - origin_).array() / tile_size_.array()).ceil().cast<int>();

  for (int y = min_indices.y(); y < max_indices.y(); ++y)
  {
    for (int x = min_indices.x(); x < max_indices.x(); ++x)
    {
      const std::size_t tile_index = (*this)[Vec2i{x, y}];

      const Vec2f rect_min{origin_ + Vec2f{x * tile_size_.x(), y * tile_size_.y()}};
      const Vec2f rect_max{rect_min + tile_size_};

      s__textured_quad_buffer.push_back(
        {.rect = Bounds2f{rect_min, rect_max},
         .rect_texture = tile_set->tile_bounds[tile_index],
         .color = tint,
         .texture_unit = (*texture_unit_opt)});
    }
  }

  rp.submit(make_const_view(s__textured_quad_buffer));
  s__textured_quad_buffer.clear();
}

std::ostream& operator<<(std::ostream& os, const TileMap& tile_map)
{
  // os << "position: " << tile_map.position.transpose() << '\n';
  // os << "tile-size: " << tile_map.tile_size.transpose() << '\n';
  // os << "tiles:\n" << tile_map.tiles;
  return os;
}

}  // namespace sde::graphics
