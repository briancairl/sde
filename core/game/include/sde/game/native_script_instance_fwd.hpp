/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_instance_fwd.hpp
 */
#pragma once

// SDE
#include "sde/game/native_script_fwd.hpp"
#include "sde/resource_cache_traits.hpp"

namespace sde::game
{
enum class NativeScriptInstanceError;
struct NativeScriptInstanceHandle;
class NativeScriptInstance;
class NativeScriptInstanceCache;
}  // namespace sde::game

namespace sde
{
template <> struct ResourceCacheTraits<game::NativeScriptInstanceCache>
{
  using error_type = game::NativeScriptInstanceError;
  using handle_type = game::NativeScriptInstanceHandle;
  using value_type = game::NativeScriptInstanceData;
  using dependencies = ResourceDependencies<game::NativeScriptCache>;
};

template <> struct ResourceHandleToCache<game::NativeScriptInstanceHandle>
{
  using type = game::NativeScriptInstanceCache;
};
}  // namespace sde