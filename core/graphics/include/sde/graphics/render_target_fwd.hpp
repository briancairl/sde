/**
 * @copyright 2024-present Brian Cairl
 *
 * @file render_target_fwd.hpp
 */
#pragma once

// SDE
#include "sde/resource_cache_traits.hpp"

namespace sde::graphics
{
enum class RenderTargetError;
struct RenderTarget;
struct RenderTargetHandle;
class RenderTargetCache;
class TextureCache;
}  // namespace sde::graphics

namespace sde
{
template <> struct ResourceCacheTraits<graphics::RenderTargetCache>
{
  using error_type = graphics::RenderTargetError;
  using handle_type = graphics::RenderTargetHandle;
  using value_type = graphics::RenderTarget;
  using dependencies = ResourceDependencies<graphics::TextureCache>;
};

template <> struct ResourceHandleToCache<graphics::RenderTargetHandle>
{
  using type = graphics::RenderTargetCache;
};
}  // namespace sde