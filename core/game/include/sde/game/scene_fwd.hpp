/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene_fwd.hpp
 */
#pragma once

// SDE
#include "sde/resource_cache_traits.hpp"

namespace sde::game
{
enum class SceneError;
struct SceneData;
struct SceneHandle;
class Scene;
class SceneCache;
}  // namespace sde::game

namespace sde
{
template <> struct ResourceCacheTraits<game::SceneCache>
{
  using error_type = game::SceneError;
  using handle_type = game::SceneHandle;
  using value_type = game::SceneData;
  using dependencies = no_dependencies;
};

template <> struct ResourceHandleToCache<game::SceneHandle>
{
  using type = game::SceneCache;
};
}  // namespace sde