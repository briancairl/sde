// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/component.hpp"
#include "sde/game/entity.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

std::ostream& operator<<(std::ostream& os, EntityError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(EntityError::kElementAlreadyExists)
    SDE_OS_ENUM_CASE(EntityError::kComponentAlreadyAttached)
    SDE_OS_ENUM_CASE(EntityError::kComponentNotRegistered)
    SDE_OS_ENUM_CASE(EntityError::kInvalidHandle)
    SDE_OS_ENUM_CASE(EntityError::kCreationFailure)
  }
  return os;
}

ComponentHandle EntityCache::locate_component_if_registered(dependencies deps, std::string_view name) const
{
  return deps.get<ComponentCache>().get_handle(sde::string{name});
}

expected<void, EntityError> EntityCache::reload(dependencies deps, EntityData& entity)
{
  entity.id = deps.get<Registry>().create();
  return {};
}

expected<void, EntityError> EntityCache::unload(dependencies deps, const EntityData& entity)
{
  deps.get<Registry>().destroy(entity.id);
  return {};
}

expected<EntityData, EntityError> EntityCache::generate(dependencies deps)
{
  EntityData entity;
  if (auto ok_or_error = reload(deps, entity))
  {
    return entity;
  }
  return make_unexpected(EntityError::kCreationFailure);
}

}  // namespace sde::game