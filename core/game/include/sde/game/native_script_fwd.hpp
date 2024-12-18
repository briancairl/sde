/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_fwd.hpp
 */
#pragma once

// SDE
#include "sde/game/library_fwd.hpp"
#include "sde/resource_cache_traits.hpp"

namespace sde::game
{
enum class NativeScriptError;
struct NativeScriptFn;
struct NativeScriptHandle;
struct NativeScriptData;
class NativeScriptInstance;
class NativeScript;
class NativeScriptCache;
}  // namespace sde::game

namespace sde
{
template <> struct ResourceCacheTraits<game::NativeScriptCache>
{
  using error_type = game::NativeScriptError;
  using handle_type = game::NativeScriptHandle;
  using value_type = game::NativeScriptData;
  using dependencies = ResourceDependencies<game::LibraryCache>;
};

template <> struct ResourceHandleToCache<game::NativeScriptHandle>
{
  using type = game::NativeScriptCache;
};
}  // namespace sde