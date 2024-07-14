/**
 * @copyright 2024-present Brian Cairl
 *
 * @file tile_map.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <vector>

// SDE
#include "sde/geometry.hpp"
#include "sde/graphics/render_buffer_fwd.hpp"
#include "sde/graphics/renderer_fwd.hpp"
#include "sde/graphics/tile_map_fwd.hpp"
#include "sde/graphics/tile_set_handle.hpp"
#include "sde/resource.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

struct TileMapOptions : Resource<TileMapOptions>
{
  Vec4f tint_color = Vec4f::Ones();
  Vec2i shape = Vec2i::Zero();
  Vec2f tile_size = Vec2f::Zero();
  TileSetHandle tile_set = TileSetHandle::null();

  auto field_list()
  {
    return FieldList(
      (Field{"tint_color", tint_color}),
      (Field{"shape", shape}),
      (Field{"tile_size", tile_size}),
      (Field{"tile_set", tile_set}));
  }
};

std::ostream& operator<<(std::ostream& os, const TileMapOptions& tile_map_options);

bool operator==(const TileMapOptions& lhs, const TileMapOptions& rhs);

class TileMap : public Resource<TileMap>
{
  friend fundemental_type;

public:
  explicit TileMap(const TileMapOptions& options);

  TileMap() = default;
  TileMap(TileMap&&);
  TileMap& operator=(TileMap&&);

  void draw(RenderPass& rp, const Vec2f& origin) const;

  const TileMapOptions& options() const { return options_; }

  const Vec2i shape() const { return options_.shape; }

  View<TileIndex> data() { return View<TileIndex>{tile_indices_.data(), tile_indices_.size()}; }

  View<const TileIndex> data() const { return View<const TileIndex>{tile_indices_.data(), tile_indices_.size()}; }

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
  auto field_list() { return std::tuple_cat(options_.field_list(), FieldList(Field{"tile_indices", tile_indices_})); }

  TileMap(const TileMap&) = delete;
  TileMap& operator=(const TileMap&) = delete;

  void release();

  TileMapOptions options_;
  std::vector<TileIndex> tile_indices_;
};

}  // namespace sde::graphics
