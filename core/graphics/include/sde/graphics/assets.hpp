/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// SDE
#include "sde/graphics/font.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/type_set.hpp"

namespace sde::graphics
{

struct Assets
{
  /// Font cache
  FontCache fonts;

  /// Shader asset cache
  ShaderCache shaders;

  /// Glyph-set cache
  TypeSetCache type_sets;

  /// Texture asset cache
  TextureCache textures;

  /// Tile set asset cache
  TileSetCache tile_sets;

  /// Render target asset cache
  RenderTargetCache render_targets;
};

}  // namespace sde::graphics
