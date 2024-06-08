/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/graphics/font.hpp"
#include "sde/graphics/glyph_set.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"

namespace sde::graphics
{

struct Assets
{
  /// Font cache
  FontCache fonts;

  /// Glyph-set cache
  GlyphSetCache glyph_sets;

  /// Shader asset cache
  ShaderCache shaders;

  /// Texture asset cache
  TextureCache textures;

  /// Tile set asset cache
  TileSetCache tile_sets;
};

}  // namespace sde::graphics
