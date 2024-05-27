/**
 * @copyright 2024-present Brian Cairl
 *
 * @file tile_map.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/graphics/texture_fwd.hpp"

namespace sde::graphics
{
class TileSet;

enum class TileMapInfoError
{
  kAtlasTextureNotLoaded
};

std::ostream& operator<<(std::ostream& os, TileMapInfoError error);

struct TileMapInfo
{
public:
  static expected<TileMapInfo, TileMapInfoError> create(const TileSet& tile_set, const TextureUnits& texture_units);

  constexpr std::size_t getTextureUnit() const { return texture_unit_; }

  constexpr const auto& getTileRects() const { return *tile_set_; }

  constexpr bool isValid() const { return tile_set_ != nullptr; }

private:
  /// Assigned texture unit
  std::size_t texture_unit_ = 0;

  /// Associated tile set
  const TileSet* tile_set_ = nullptr;
};

std::ostream& operator<<(std::ostream& os, const TileMapInfo& tile_map_info);

struct TileMap
{
  static constexpr std::size_t kDim = 8;
  static constexpr std::size_t kTileCount = kDim * kDim;

  Vec2f position = Vec2f::Zero();
  Vec2f tile_size = Vec2f::Zero();
  Vec4f color = Vec4f::Ones();
  Mat<std::size_t, kDim> tiles;

  TileMapInfo info;
};

inline Vec2f getNextRightPosition(const TileMap& tm)
{
  return tm.position + Vec2f{tm.tile_size.x() * TileMap::kDim, 0.0F};
}

std::ostream& operator<<(std::ostream& os, const TileMap& tile_map);

}  // namespace sde::graphics
