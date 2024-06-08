/**
 * @copyright 2024-present Brian Cairl
 *
 * @file tile_map.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/geometry_types.hpp"
#include "sde/graphics/renderer_fwd.hpp"
#include "sde/graphics/tile_set_handle.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

class TileMap
{
public:
  ~TileMap();

  TileMap(TileMap&&);
  TileMap& operator=(TileMap&&);

  void draw(RenderPass& rp, const Vec4f& tint = Vec4f::Ones()) const;

  const Vec2i shape() const { return shape_; }

  View<const std::size_t> data() const
  {
    return View<const std::size_t>{tile_indices_, static_cast<std::size_t>(shape_.prod())};
  }

  std::size_t operator[](const Vec2i indices) const { return tile_indices_[indices.y() * shape_.y() + indices.x()]; }

  std::size_t& operator[](const Vec2i indices) { return tile_indices_[indices.y() * shape_.y() + indices.x()]; }

  void swap(TileMap& other);

  static TileMap create(const TileSetHandle& tile_set, const Vec2i& shape, const Vec2f& origin, const Vec2f& tile_size);

private:
  TileMap() = default;
  TileMap(const TileMap&) = delete;
  TileMap& operator=(const TileMap&) = delete;

  Vec2i shape_;
  Vec2f origin_;
  Vec2f tile_size_;
  TileSetHandle tile_set_handle_;
  std::size_t* tile_indices_ = nullptr;
};

std::ostream& operator<<(std::ostream& os, const TileMap& tile_map);

}  // namespace sde::graphics
