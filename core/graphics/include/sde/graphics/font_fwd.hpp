/**
 * @copyright 2024-present Brian Cairl
 *
 * @file text_fwd.hpp
 */
#pragma once

// SDE
#include "sde/resource_cache_traits.hpp"

namespace sde::graphics
{
enum class FontError;
struct Font;
struct FontHandle;
class FontCache;
}  // namespace sde::graphics

namespace sde
{
template <> struct ResourceCacheTraits<graphics::FontCache>
{
  using error_type = graphics::FontError;
  using handle_type = graphics::FontHandle;
  using value_type = graphics::Font;
  using dependencies = no_dependencies;
};

template <> struct ResourceHandleToCache<graphics::FontHandle>
{
  using type = graphics::FontCache;
};
}  // namespace sde