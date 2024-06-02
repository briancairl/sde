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

namespace sde::graphics
{

enum TileSetError
{
  kInvalidAtlasTexture,
  kInvalidTileSize,
  kInvalidSlicingBounds,
};

std::ostream& operator<<(std::ostream& os, TileSetError error);

class TileSet
{
public:
  /**
   * @brief Creates a tile set by uniformly slicing a texture
   */
  static expected<TileSet, TileSetError> slice(
    const TextureHandle& texture,
    const TextureInfo& texture_info,
    const Vec2i tile_size,
    const Bounds2i& tile_slice_bounds = Bounds2i{});

  /**
   * @brief Creates a tile set by uniformly slicing a texture
   */
  static expected<TileSet, TileSetError> slice(
    const TextureHandle& texture,
    const TextureCache& texture_cache,
    const Vec2i tile_size,
    const Bounds2i& tile_slice_bounds = Bounds2i{});

  /**
   * @brief Returns handle to atlas texture for this tile set
   */
  const TextureHandle atlas() const { return atlas_texture_; }

  /**
   * @brief Returns texture-space bounds for a given tile
   */
  const Bounds2f& get(const std::size_t tile) const { return tile_bounds_[tile]; }

  /**
   * @brief Returns texture-space bounds for a given tile
   */
  const Bounds2f& operator[](const std::size_t tile) const { return tile_bounds_[tile]; }

  /**
   * @brief Returns the number of tiles
   */
  std::size_t size() const { return tile_bounds_.size(); }

  TileSet(TextureHandle atlas_texture, std::vector<Bounds2f> tile_bounds);

private:
  TextureHandle atlas_texture_;
  std::vector<Bounds2f> tile_bounds_;
};

std::ostream& operator<<(std::ostream& os, const TileSet& tile_set);

}  // namespace sde::graphics
