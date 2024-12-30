/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture_fwd.hpp
 */
#pragma once

// SDE
#include "sde/resource_cache_traits.hpp"

namespace sde::graphics
{
enum class ShaderError;
struct ShaderHandle;
class ShaderCache;
struct Shader;
}  // namespace sde::graphics

namespace sde
{
template <> struct ResourceCacheTraits<graphics::ShaderCache>
{
  using error_type = graphics::ShaderError;
  using handle_type = graphics::ShaderHandle;
  using value_type = graphics::Shader;
  using dependencies = no_dependencies;
};

template <> struct ResourceHandleToCache<graphics::ShaderHandle>
{
  using type = graphics::ShaderCache;
};
}  // namespace sde