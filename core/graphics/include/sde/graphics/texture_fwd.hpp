/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture_fwd.hpp
 */
#pragma once

// SDE
#include "sde/graphics/image_fwd.hpp"
#include "sde/resource_cache_traits.hpp"

namespace sde::graphics
{
enum class TextureError;
struct TextureHandle;
struct Texture;
struct TextureUnits;
struct TextureOptions;
struct TextureShape;
class TextureCache;
}  // namespace sde::graphics

namespace sde
{
template <> struct ResourceCacheTraits<graphics::TextureCache>
{
  using error_type = graphics::TextureError;
  using handle_type = graphics::TextureHandle;
  using value_type = graphics::Texture;
  using dependencies = ResourceDependencies<graphics::ImageCache>;
};
}  // namespace sde