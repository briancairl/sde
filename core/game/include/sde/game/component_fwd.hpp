/**
 * @copyright 2024-present Brian Cairl
 *
 * @file component_fwd.hpp
 */
#pragma once

// SDE
#include "sde/game/library_fwd.hpp"
#include "sde/resource_cache_traits.hpp"

namespace sde::game
{
enum class ComponentError;
struct ComponentHandle;
struct ComponentData;
class ComponentIO;
class ComponentCache;
}  // namespace sde::game

namespace sde
{
template <> struct ResourceCacheTraits<game::ComponentCache>
{
  using error_type = game::ComponentError;
  using handle_type = game::ComponentHandle;
  using value_type = game::ComponentData;
  using dependencies = ResourceDependencies<game::LibraryCache>;
};
}  // namespace sde