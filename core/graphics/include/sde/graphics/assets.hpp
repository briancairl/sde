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
#include "sde/resource.hpp"

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

struct Assets : Resource<Assets>
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

  void strip();

  auto field_list()
  {
    return FieldList(
      Field{"images", images},
      Field{"fonts", fonts},
      Field{"shaders", shaders},
      Field{"textures", textures},
      Field{"tile_sets", tile_sets},
      Field{"type_sets", type_sets},
      Field{"render_targets", render_targets});
  }
};

}  // namespace sde::graphics
