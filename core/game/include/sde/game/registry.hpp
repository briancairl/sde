/**
 * @copyright 2024-present Brian Cairl
 *
 * @file registry.hpp
 */
#pragma once

// SDE
#include "sde/memory.hpp"
#include "sde/resource_cache.hpp"

/// EnTT
#include <entt/entity/registry.hpp>

namespace sde::game
{

using EntityID = entt::entity;
struct Registry : entt::basic_registry<EntityID, allocator<EntityID>>
{
  Registry() = default;

  Registry(Registry&& other) = default;
  Registry& operator=(Registry&& other) = default;

  Registry(const Registry& other) = delete;
  Registry& operator=(const Registry& other) = delete;
};

}  // namespace sde::game

namespace sde
{
template <> struct ResourceCacheTraits<game::Registry>
{
  using error_type = void;
  using handle_type = game::EntityID;
  using value_type = void;
  using dependencies = no_dependencies;
};

template <> struct ResourceHandleToCache<game::EntityID>
{
  using type = game::Registry;
};

template <> struct IsResourceCache<game::Registry> : std::true_type
{};
}  // namespace sde