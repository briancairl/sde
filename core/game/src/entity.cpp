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
    SDE_OS_ENUM_CASE(EntityError::kComponentDumpFailure)
    SDE_OS_ENUM_CASE(EntityError::kComponentLoadFailure)
    SDE_OS_ENUM_CASE(EntityError::kInvalidHandle)
    SDE_OS_ENUM_CASE(EntityError::kCreationFailure)
  }
  return os;
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

expected<void, EntityError> EntityCache::load(dependencies deps, IArchive& archive)
{
  auto& registry = deps.get<Registry>();
  const auto& components = deps.get<ComponentCache>();
  for (auto& [handle, data] : handle_to_value_cache_)
  {
    for (const auto& component_handle : data->components)
    {
      if (const auto c = components(component_handle); c)
      {
        c->io.load(archive, data->id, registry);
        SDE_LOG_INFO() << handle << ": loaded component: " << c->name;
      }
      else
      {
        SDE_LOG_ERROR() << handle << ": failed to find component: " << component_handle;
        return make_unexpected(EntityError::kComponentLoadFailure);
      }
    }
  }
  return {};
}

expected<void, EntityError> EntityCache::dump(dependencies deps, OArchive& archive) const
{
  const auto& registry = deps.get<Registry>();
  const auto& components = deps.get<ComponentCache>();
  for (auto& [handle, data] : handle_to_value_cache_)
  {
    for (const auto& component_handle : data->components)
    {
      if (const auto c = components(component_handle); c)
      {
        c->io.save(archive, data->id, registry);
        SDE_LOG_INFO() << handle << ": saved component: " << c->name;
      }
      else
      {
        SDE_LOG_ERROR() << handle << ": failed to find component: " << component_handle;
        return make_unexpected(EntityError::kComponentDumpFailure);
      }
    }
  }
  return {};
}

}  // namespace sde::game