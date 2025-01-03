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

bool EntityCache::when_removed(dependencies deps, EntityHandle handle, const EntityData* data)
{
  if (auto& registry = deps.get<Registry>(); registry.valid(data->id))
  {
    registry.destroy(data->id);
    return true;
  }
  return false;
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

struct EntityLoadWrapper
{
  EntityHandle handle;
  EntityCache::ElementMap* map;
  Registry* registry;
  const ComponentCache* components;
};

expected<void, EntityError> EntityCache::load(dependencies deps, IArchiveAssociative& archive)
{
  using namespace sde::serial;

  auto& registry = deps.get<Registry>();
  const auto& components = deps.get<ComponentCache>();

  std::size_t element_to_load_count = 0;
  archive >> named{"size", element_to_load_count};
  SDE_LOG_INFO() << "Loading components for " << element_to_load_count << " entities";
  for (std::size_t n = 0; n < element_to_load_count; ++n)
  {
    EntityLoadWrapper io{
      EntityHandle::null(),
      std::addressof(handle_to_value_cache_),
      std::addressof(registry),
      std::addressof(components)};
    archive >> serial::named{format("entity-entry[%lu]", n), io};
  }
  return {};
}

struct EntitySaveWrapper
{
  EntityHandle handle;
  const EntityData* entity;
  const Registry* registry;
  const ComponentCache* components;
};

expected<void, EntityError> EntityCache::save(dependencies deps, OArchiveAssociative& archive) const
{
  static_assert(sizeof(EntitySaveWrapper) == sizeof(EntityLoadWrapper));

  using namespace sde::serial;

  const auto& registry = deps.get<Registry>();
  const auto& components = deps.get<ComponentCache>();

  archive << named{"size", handle_to_value_cache_.size()};
  SDE_LOG_INFO() << "Saving components for " << handle_to_value_cache_.size() << " entities";
  std::size_t n = 0;
  for (auto& [handle, entity] : handle_to_value_cache_)
  {
    EntitySaveWrapper io{handle, std::addressof(entity.value), std::addressof(registry), std::addressof(components)};
    archive << serial::named{format("entity-entry[%lu]", n++), io};
  }
  return {};
}

}  // namespace sde::game

namespace sde::serial
{

template <typename Archive> struct load<Archive, sde::game::EntityLoadWrapper>
{
  expected<void, iarchive_error> operator()(Archive& ar, sde::game::EntityLoadWrapper eio) const
  {
    // Elements might not be in the same order as they were saved, so we must
    // load and insert at the absolute handle to this element
    ar >> named{"handle", eio.handle};

    // Access entity
    const auto entity_itr = eio.map->find(eio.handle);
    if (entity_itr == std::end(*eio.map))
    {
      SDE_LOG_ERROR() << eio.handle << "Invalid entity handle: " << eio.handle;
      return make_unexpected(iarchive_error::kLoadFailure);
    }
    auto& entity = entity_itr->second.value;

    // Load its components in order they were saved
    for (const auto& component_handle : entity.components)
    {
      if (const auto c = (*eio.components)(component_handle); c)
      {
        c->io.load(ar, entity.id, *eio.registry);
        SDE_LOG_INFO() << eio.handle << ": loaded component: " << c->name;
      }
      else
      {
        SDE_LOG_ERROR() << eio.handle << ": failed to find component: " << component_handle;
        return make_unexpected(iarchive_error::kLoadFailure);
      }
    }
    return {};
  }
};

template <typename Archive> struct save<Archive, sde::game::EntitySaveWrapper>
{
  expected<void, oarchive_error> operator()(Archive& ar, const sde::game::EntitySaveWrapper eio) const
  {
    // Elements might not be in the same order as they were saved, so we must
    // save the absolute handle to this element
    ar << named{"handle", eio.handle};
    for (const auto& component_handle : eio.entity->components)
    {
      // Save components in the order they were attached
      if (const auto c = (*eio.components)(component_handle); c)
      {
        c->io.save(ar, eio.entity->id, *eio.registry);
        SDE_LOG_INFO() << eio.handle << ": saved component: " << c->name;
      }
      else
      {
        SDE_LOG_ERROR() << eio.handle << ": failed to find component: " << component_handle;
        return make_unexpected(oarchive_error::kSaveFailure);
      }
    }
    return {};
  }
};

}  // namespace sde::serial