/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture.hpp
 */
#pragma once

// C++ Standard Library
#include <vector>

// SDE
#include "sde/expected.hpp"

// SDE
#include "sde/graphics/shapdes.hpp"
#include "sde/graphics/texture_fwd.hpp"

namespace sde::graphics
{

enum TileSetError
{

};

class TileSet
{
public:
  static expected<TileSet, TileSetError>
  slice(const TextureInfo& texture, const Vec2i tile_size, const Vec2i offset = Vec2i::Zero());

private:
  explicit TileSet(std::vector<Rect> tex_coords);
  std::vector<Rect> tex_coords_;
};

}  // namespace sde::graphics
