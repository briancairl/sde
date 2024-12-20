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
#include "sde/game/component.hpp"
#include "sde/game/component_decl.hpp"
#include "sde/game/component_handle.hpp"
#include "sde/game/entity_fwd.hpp"
#include "sde/game/entity_handle.hpp"
#include "sde/game/registry.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
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

struct EntityComponent : Resource<EntityComponent>
{
  std::size_t version = 0;
  ComponentHandle handle = ComponentHandle::null();

  EntityComponent(std::size_t _version, ComponentHandle _handle) : version{_version}, handle{_handle} {}

  auto field_list() { return FieldList(Field{"version", version}, Field{"handle", handle}); }
};

struct EntityData : Resource<EntityData>
{
  EntityID id;
  sde::vector<EntityComponent> components;
  auto field_list() { return FieldList(_Stub{"id", id}, Field{"components", components}); }
};

class EntityCreator
{
public:
  EntityCreator(ComponentCache& components, Registry& reg, EntityData& entity) :
      components_{std::addressof(components)}, reg_{std::addressof(reg)}, entity_{std::addressof(entity)}
  {}

  /**
   * @brief Attach a component to an existing entity
   */
  template <typename ComponentT, typename... CTorArgs> expected<ComponentT*, EntityError> attach(CTorArgs&&... args)
  {
    if (reg_->template all_of<ComponentT>(entity_->id))
    {
      return make_unexpected(EntityError::kComponentAlreadyAttached);
    }
    else if (auto component = components_->to_handle(sde::string{ComponentName<ComponentT>::value}))
    {
      entity_->components.emplace_back(
        ComputeHash(ComponentName<ComponentT>::value, std::forward<CTorArgs>(args)...), component);
    }
    else
    {
      return make_unexpected(EntityError::kComponentNotRegistered);
    }

    using ReturnT = decltype(reg_->template emplace<ComponentT>(entity_->id, std::forward<CTorArgs>(args)...));
    if constexpr (std::is_void_v<ReturnT>)
    {
      reg_->template emplace<ComponentT>(entity_->id, std::forward<CTorArgs>(args)...);
      return nullptr;
    }
    else
    {
      auto& c = reg_->template emplace<ComponentT>(entity_->id, std::forward<CTorArgs>(args)...);
      return std::addressof(c);
    }
  }

public:
  ComponentCache* components_;
  Registry* reg_;
  EntityData* entity_;
};

class EntityLoader
{
public:
  EntityLoader(ComponentCache& components, Registry& reg, EntityData& entity) :
      components_{std::addressof(components)}, reg_{std::addressof(reg)}, entity_{std::addressof(entity)}
  {}

  /**
   * @brief Attach a component to an existing entity
   */
  template <typename ComponentT, typename... CTorArgs> expected<ComponentT*, EntityError> attach(CTorArgs&&... args)
  {
    if (reg_->template all_of<ComponentT>(entity_->id))
    {
      return make_unexpected(EntityError::kComponentAlreadyAttached);
    }
    else if (auto component = components_->to_handle(sde::string{ComponentName<ComponentT>::value}))
    {
      entity_->components.emplace_back(
        ComputeHash(ComponentName<ComponentT>::value, std::forward<CTorArgs>(args)...), component);
    }
    else
    {
      return make_unexpected(EntityError::kComponentNotRegistered);
    }

    using ReturnT = decltype(reg_->template emplace<ComponentT>(entity_->id, std::forward<CTorArgs>(args)...));
    if constexpr (std::is_void_v<ReturnT>)
    {
      reg_->template emplace<ComponentT>(entity_->id, std::forward<CTorArgs>(args)...);
      return nullptr;
    }
    else
    {
      auto& c = reg_->template emplace<ComponentT>(entity_->id, std::forward<CTorArgs>(args)...);
      return std::addressof(c);
    }
  }

public:
  ComponentCache* components_;
  Registry* reg_;
  EntityData* entity_;
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
    auto value_or_error = this->create(deps);
    if (value_or_error.has_value())
    {
      auto entity_itr = handle_to_value_cache_.find(value_or_error->handle);
      EntityCreator entity_creation{deps.get<ComponentCache>(), deps.get<Registry>(), entity_itr->second.value};
      attach_components(entity_creation);
    }
    return value_or_error;
  }

private:
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