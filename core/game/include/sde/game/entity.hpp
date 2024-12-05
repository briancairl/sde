/**
 * @copyright 2024-present Brian Cairl
 *
 * @file entity.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string>

// EnTT
#include <entt/fwd.hpp>

// SDE
#include "sde/game/component_fwd.hpp"
#include "sde/game/component_handle.hpp"
#include "sde/game/entity_fwd.hpp"
#include "sde/game/entity_handle.hpp"
#include "sde/game/registry.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/type_name.hpp"
#include "sde/vector.hpp"

namespace sde::game
{

enum class EntityError
{
  kElementAlreadyExists,
  kComponentAlreadyAttached,
  kComponentNotRegistered,
  kInvalidHandle,
  kCreationFailure,
};

std::ostream& operator<<(std::ostream& os, EntityError error);

struct EntityData : Resource<EntityData>
{
  EntityID id;
  sde::vector<ComponentHandle> components;
  auto field_list() { return FieldList(_Stub{"id", id}, Field{"components", components}); }
};

class EntityCache : public ResourceCache<EntityCache>
{
  friend fundemental_type;

public:
  /**
   * @brief Creates a new entity and adds components
   */
  template <typename AttachComponentsT>
  decltype(auto) make_entity(dependencies deps, AttachComponentsT attach_components)
  {
    auto value_or_error = this->create();
    if (value_or_error.has_value())
    {
      auto entity_itr = handle_to_value_cache_.find(value_or_error->handle);
      attach_components(*this, value_or_error->handle, entity_itr->second.value);
    }
    return value_or_error;
  }

  /**
   * @brief Attach a component to an existing entity
   */
  template <typename ComponentT, typename... CTorArgs>
  expected<ComponentT*, EntityError> attach(dependencies deps, EntityHandle handle, CTorArgs&&... args)
  {
    auto entity_itr = handle_to_value_cache_.find(handle);
    if (entity_itr == handle_to_value_cache_.end())
    {
      return make_unexpected(EntityError::kInvalidHandle);
    }
    return this->template attach<ComponentT>(deps, entity_itr->second.value, std::forward<CTorArgs>(args)...);
  }

  /**
   * @brief Attach a component to an existing entity
   */
  template <typename ComponentT, typename... CTorArgs>
  expected<ComponentT*, EntityError> attach(dependencies deps, EntityData& entity, CTorArgs&&... args)
  {
    auto& registry = deps.template get<Registry>();
    if (registry.template all_of<ComponentT>(entity.id))
    {
      return make_unexpected(EntityError::kComponentAlreadyAttached);
    }

    if (auto component = locate_component_if_registered(type_name<ComponentT>()))
    {
      entity.components.push_back(component);
    }
    else
    {
      return make_unexpected(EntityError::kComponentNotRegistered);
    }

    using ReturnT = decltype(registry.template emplace<ComponentT>(entity.id, std::forward<CTorArgs>(args)...));
    if constexpr (std::is_void_v<ReturnT>)
    {
      registry.template emplace<ComponentT>(entity.id, std::forward<CTorArgs>(args)...);
      return nullptr;
    }
    else
    {
      auto& c = registry.template emplace<ComponentT>(entity.id, std::forward<CTorArgs>(args)...);
      return std::addressof(c);
    }
  }

  /**
   * @brief Removes an entity by its native ID
   */
  void remove(dependencies deps, entt::entity e)
  {
    auto& registry = deps.template get<Registry>();
    for (const auto& [handle, value] : handle_to_value_cache_)
    {
      if (value->id == e)
      {
        handle_to_value_cache_.erase(handle);
        registry.destroy(e);
        return;
      }
    }
  }

private:
  ComponentHandle locate_component_if_registered(dependencies deps, std::string_view name) const;

  expected<void, EntityError> reload(dependencies deps, EntityData& entity);
  expected<void, EntityError> unload(dependencies deps, const EntityData& entity);
  expected<EntityData, EntityError> generate(dependencies deps);
};

}  // namespace sde::game

namespace sde
{
template <> struct Hasher<entt::entity>
{
  Hash operator()(const entt::entity& e) const { return {static_cast<std::size_t>(e)}; }
};
}  // namespace sde