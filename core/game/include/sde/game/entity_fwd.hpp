/**
 * @copyright 2024-present Brian Cairl
 *
 * @file entity_fwd.hpp
 */
#pragma once

// SDE
#include "sde/game/component_fwd.hpp"
#include "sde/game/registry.hpp"
#include "sde/resource_cache_traits.hpp"

namespace sde::game
{
enum class EntityError;
struct EntityHandle;
struct EntityData;
class EntityCache;
}  // namespace sde::game

namespace sde
{
template <> struct ResourceCacheTraits<game::EntityCache>
{
  using error_type = game::EntityError;
  using handle_type = game::EntityHandle;
  using value_type = game::EntityData;
  using dependencies = ResourceDependencies<game::ComponentCache, game::Registry>;
};
}  // namespace sde