/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <optional>

// SDE
#include "sde/expected.hpp"
#include "sde/graphics/font.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/type_set.hpp"
#include "sde/resource.hpp"
#include "sde/resource_collection.hpp"

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

std::ostream& operator<<(std::ostream& os, AssetError error);

struct Assets : ResourceCollection<
                  ResourceCollectionEntry<"images"_rl, ImageCache>,
                  ResourceCollectionEntry<"fonts"_rl, FontCache>,
                  ResourceCollectionEntry<"shaders"_rl, ShaderCache>,
                  ResourceCollectionEntry<"textures"_rl, TextureCache>,
                  ResourceCollectionEntry<"tile_sets"_rl, TileSetCache>,
                  ResourceCollectionEntry<"type_sets"_rl, TypeSetCache>,
                  ResourceCollectionEntry<"render_targets"_rl, RenderTargetCache>>
{
  expected<void, AssetError> refresh();

  void strip();
};

}  // namespace sde::graphics
