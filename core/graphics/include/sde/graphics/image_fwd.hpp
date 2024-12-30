/**
 * @copyright 2024-present Brian Cairl
 *
 * @file image_fwd.hpp
 */
#pragma once

// SDE
#include "sde/resource_cache_traits.hpp"

namespace sde::graphics
{
enum class ImageError;
struct ImageHandle;
struct ImageOptions;
struct ImageShape;
struct ImageRef;
struct Image;
class ImageCache;
}  // namespace sde::graphics

namespace sde
{
template <> struct ResourceCacheTraits<graphics::ImageCache>
{
  using error_type = graphics::ImageError;
  using handle_type = graphics::ImageHandle;
  using value_type = graphics::Image;
  using dependencies = no_dependencies;
};

template <> struct ResourceHandleToCache<graphics::ImageHandle>
{
  using type = graphics::ImageCache;
};
}  // namespace sde