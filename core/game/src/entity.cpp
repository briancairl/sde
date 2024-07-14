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

expected<void, EntityError> EntityCache::reload(entt::entity& entity)
{
  entity = registry_->create();
  return {};
}

expected<void, EntityError> EntityCache::unload(const entt::entity& entity)
{
  registry_->destroy(entity);
  return {};
}

expected<entt::entity, EntityError> EntityCache::generate()
{
  entt::entity e;
  if (auto ok_or_error = reload(e))
  {
    return e;
  }
  return make_unexpected(EntityError::kCreationFailure);
}

}  // namespace sde::game