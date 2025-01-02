// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/archive.hpp"
#include "sde/game/component.hpp"
#include "sde/game/entity.hpp"
#include "sde/logging.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/resource_io.hpp"
#include "sde/serialization.hpp"

namespace sde::game
{

std::ostream& operator<<(std::ostream& os, EntityError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASES_FOR_RESOURCE_CACHE_ERRORS(EntityError)
    SDE_OS_ENUM_CASE(EntityError::kComponentAlreadyAttached)
    SDE_OS_ENUM_CASE(EntityError::kComponentNotRegistered)
    SDE_OS_ENUM_CASE(EntityError::kComponentDumpFailure)
    SDE_OS_ENUM_CASE(EntityError::kComponentLoadFailure)
    SDE_OS_ENUM_CASE(EntityError::kCreationFailure)
  }
  return os;
}

expected<void, EntityError> EntityCache::reload(dependencies deps, EntityData& entity)
{
  entity.id = deps.get<Registry>().create();
  SDE_LOG_INFO() << "Entity: [" << static_cast<int>(entity.id) << "] has " << entity.components.size() << " components";
  return {};
}

expected<void, EntityError> EntityCache::unload(dependencies deps, const EntityData& entity)
{
  EntityCache::when_removed(deps, EntityHandle::null(), std::addressof(entity));
  return {};
}

void EntityCache::when_removed(dependencies deps, EntityHandle handle, const EntityData* data)
{
  if (auto& registry = deps.get<Registry>(); registry.valid(data->id))
  {
    registry.destroy(data->id);
  }
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
  using namespace sde::serial;

  auto& registry = deps.get<Registry>();
  const auto& components = deps.get<ComponentCache>();

  std::size_t element_to_load_count = 0;
  archive >> named{"size", element_to_load_count};
  SDE_LOG_INFO() << "Loading components for " << element_to_load_count << " entities";
  for (std::size_t n = 0; n < element_to_load_count; ++n)
  {
    // Elements might not be in the same order as they were saved, so we must
    // load and insert at the absolute handle to this element
    EntityHandle handle = {};
    archive >> named{"handle", handle};

    // Access entity
    const auto entity_itr = handle_to_value_cache_.find(handle);
    SDE_ASSERT_TRUE(entity_itr != std::end(handle_to_value_cache_)) << "Invalid entity handle: " << handle;
    auto& entity = entity_itr->second.value;

    // Load its components in order they were saved
    for (const auto& component_handle : entity.components)
    {
      if (const auto c = components(component_handle); c)
      {
        c->io.load(archive, entity.id, registry);
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

expected<void, EntityError> EntityCache::save(dependencies deps, OArchive& archive) const
{
  using namespace sde::serial;

  const auto& registry = deps.get<Registry>();
  const auto& components = deps.get<ComponentCache>();

  archive << named{"size", handle_to_value_cache_.size()};
  SDE_LOG_INFO() << "Saving components for " << handle_to_value_cache_.size() << " entities";
  for (auto& [handle, entity] : handle_to_value_cache_)
  {
    // Elements might not be in the same order as they were saved, so we must
    // save the absolute handle to this element
    archive << named{"handle", handle};
    for (const auto& component_handle : entity->components)
    {
      // Save components in the order they were attached
      if (const auto c = components(component_handle); c)
      {
        c->io.save(archive, entity->id, registry);
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