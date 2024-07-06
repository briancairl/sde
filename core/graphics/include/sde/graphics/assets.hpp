/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// SDE
#include "sde/expected.hpp"
#include "sde/graphics/font.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/type_set.hpp"

namespace sde::graphics
{

enum class AssetError
{
  kFailedImageLoading,
  kFailedFontLoading,
  kFailedShaderLoading,
  kFailedTextureLoading,
  kFailedTileSetLoading,
  kFailedTypeSetLoading,
  kFailedRenderTargetLoading,
};

struct Assets
{
  /// Image cache
  ImageCache images;

  /// Font cache
  FontCache fonts;

  /// Shader asset cache
  ShaderCache shaders;

  /// Texture asset cache
  TextureCache textures;

  /// Tile set asset cache
  TileSetCache tile_sets;

  /// Glyph-set cache
  TypeSetCache type_sets;

  /// Render target asset cache
  RenderTargetCache render_targets;

  Assets();

  expected<void, AssetError> refresh();
};

}  // namespace sde::graphics
