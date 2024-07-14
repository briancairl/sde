// C++ Standard Library
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/entity.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

EntityCache::EntityCache(entt::registry& registry) : registry_{std::addressof(registry)} {}

expected<void, EntityError> EntityCache::reload(Entity& entity)
{
  entity.id = registry_->create();
  return {};
}

expected<void, EntityError> EntityCache::unload(const Entity& entity)
{
  registry_->destroy(entity.id);
  return {};
}

expected<Entity, EntityError> EntityCache::generate()
{
  Entity entity;
  if (auto ok_or_error = reload(entity))
  {
    return entity;
  }
  return make_unexpected(EntityError::kCreationFailure);
}

}  // namespace sde::game