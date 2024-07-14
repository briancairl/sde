/**
 * @copyright 2024-present Brian Cairl
 *
 * @file entity.hpp
 */
#pragma once

// EnTT
#include <entt/entity/entity.hpp>
#include <entt/fwd.hpp>

// SDE
#include "sde/game/entity_fwd.hpp"
#include "sde/game/entity_handle.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"

namespace sde::game
{

enum class EntityError
{
  kElementAlreadyExists,
  kInvalidHandle,
  kCreationFailure,
};

struct Entity : Resource<Entity>
{
  entt::entity id;

  auto field_list() { return FieldList(Field{"id", id}); }
};

}  // namespace sde::game

namespace sde
{

template <> struct Hasher<entt::entity>
{
  Hash operator()(const entt::entity& e) const { return {static_cast<std::size_t>(e)}; }
};

template <> struct ResourceCacheTypes<game::EntityCache>
{
  using error_type = game::EntityError;
  using handle_type = game::EntityHandle;
  using value_type = game::Entity;
};

}  // namespace sde

namespace sde::game
{

class EntityCache : public ResourceCache<EntityCache>
{
  friend fundemental_type;

public:
  explicit EntityCache(entt::registry& registry);

private:
  entt::registry* registry_;

  expected<void, EntityError> reload(Entity& entity);
  expected<void, EntityError> unload(const Entity& entity);
  expected<Entity, EntityError> generate();
};

}  // namespace sde::game
