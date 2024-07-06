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
#include "sde/graphics/render_buffer_fwd.hpp"
#include "sde/graphics/renderer_fwd.hpp"
#include "sde/graphics/tile_map_fwd.hpp"
#include "sde/graphics/tile_set_handle.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

struct TileMapOptions
{
  Vec4f tint_color = Vec4f::Ones();
  Vec2i shape = Vec2i::Zero();
  Vec2f tile_size = Vec2f::Zero();
  TileSetHandle tile_set = TileSetHandle::null();
};

std::ostream& operator<<(std::ostream& os, const TileMapOptions& tile_map_options);

bool operator==(const TileMapOptions& lhs, const TileMapOptions& rhs);

class TileMap
{
public:
  ~TileMap();

  explicit TileMap(const TileMapOptions& options);

  TileMap() = default;
  TileMap(TileMap&&);
  TileMap& operator=(TileMap&&);

  void draw(RenderPass& rp, const Vec2f& origin) const;

  const TileMapOptions& options() const { return options_; }

  const Vec2i shape() const { return options_.shape; }

  View<TileIndex> data() { return View<TileIndex>{tile_indices_, static_cast<std::size_t>(options_.shape.prod())}; }

  View<const TileIndex> data() const
  {
    return View<const TileIndex>{tile_indices_, static_cast<std::size_t>(options_.shape.prod())};
  }

  TileIndex operator[](const Vec2i indices) const
  {
    return tile_indices_[indices.y() * options_.shape.y() + indices.x()];
  }

  TileIndex& operator[](const Vec2i indices) { return tile_indices_[indices.y() * options_.shape.y() + indices.x()]; }

  void swap(TileMap& other);

  void setup(const TileMapOptions& options);

  void setTileSize(const Vec2f& tile_size) { options_.tile_size = tile_size; }

  void setTileSet(TileSetHandle tile_set) { options_.tile_set = tile_set; }

private:
  TileMap(const TileMap&) = delete;
  TileMap& operator=(const TileMap&) = delete;

  void release();

  TileMapOptions options_;
  TileIndex* tile_indices_ = nullptr;
};

std::ostream& operator<<(std::ostream& os, const TileMap& tile_map);

bool operator==(const TileMap& lhs, const TileMap& rhs);

}  // namespace sde::graphics
