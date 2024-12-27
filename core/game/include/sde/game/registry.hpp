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
class Registry : public entt::basic_registry<EntityID, allocator<EntityID>>
{
public:
  Registry() = default;

  Registry(Registry&& other) = default;
  Registry& operator=(Registry&& other) = default;

  Registry(const Registry& other) = delete;
  Registry& operator=(const Registry& other) = delete;

  void clear([[maybe_unused]] no_dependencies _) { base::clear(); }

private:
  using base = entt::basic_registry<EntityID, allocator<EntityID>>;
  using base::clear;
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