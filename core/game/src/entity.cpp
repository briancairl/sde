// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/component.hpp"
#include "sde/game/entity.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

EntityCache::EntityCache(Registry& registry, ComponentCache& components) :
    registry_{std::addressof(registry)}, components_{std::addressof(components)}
{}

ComponentHandle EntityCache::locate_component_if_registered(std::string_view name) const
{
  return components_->get_handle(std::string{name});
}

expected<void, EntityError> EntityCache::reload(EntityData& entity)
{
  entity.id = registry_->create();
  return {};
}

expected<void, EntityError> EntityCache::unload(const EntityData& entity)
{
  registry_->destroy(entity.id);
  return {};
}

expected<EntityData, EntityError> EntityCache::generate()
{
  EntityData entity;
  if (auto ok_or_error = reload(entity))
  {
    return entity;
  }
  return make_unexpected(EntityError::kCreationFailure);
}

}  // namespace sde::game