/**
 * @copyright 2024-present Brian Cairl
 *
 * @file entity.hpp
 */
#pragma once

// C++ Standard Library
#include <string>
#include <vector>

// EnTT
#include <entt/entity/entity.hpp>
#include <entt/fwd.hpp>

// SDE
#include "sde/game/entity_fwd.hpp"
#include "sde/game/entity_handle.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/type_name.hpp"

namespace sde::game
{

enum class EntityError
{
  kElementAlreadyExists,
  kComponentAlreadyAttached,
  kInvalidHandle,
  kCreationFailure,
};

struct Component : Resource<Component>
{
  std::string type;
  auto field_list() { return FieldList(Field{"type", type}); }
};

struct Entity : Resource<Entity>
{
  entt::entity id;
  std::vector<Component> components;
  auto field_list() { return FieldList(_Stub{"id", id}, Field{"components", components}); }
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

  /**
   * @brief Creates a new entity and adds components
   */
  template <typename AttachComponentsT> decltype(auto) make_entity(AttachComponentsT attach_components)
  {
    auto value_or_error = this->create();
    if (value_or_error.has_value())
    {
      auto entity_itr = handle_to_value_cache_.find(value_or_error->handle);
      attach_components(*this, entity_itr->second.value);
    }
    return value_or_error;
  }

  /**
   * @brief Attach a component to an existing entity
   */
  template <typename ComponentT, typename... CTorArgs>
  expected<ComponentT*, EntityError> attach(EntityHandle handle, CTorArgs&&... args)
  {
    auto entity_itr = handle_to_value_cache_.find(handle);
    if (entity_itr == handle_to_value_cache_.end())
    {
      return make_unexpected(EntityError::kInvalidHandle);
    }
    return attach(entity_itr->second, std::forward<CTorArgs>(args)...);
  }

  /**
   * @brief Attach a component to an existing entity
   */
  template <typename ComponentT, typename... CTorArgs>
  expected<ComponentT*, EntityError> attach(Entity& entity, CTorArgs&&... args)
  {
    if (registry_->template all_of<ComponentT>(entity.id))
    {
      return make_unexpected(EntityError::kComponentAlreadyAttached);
    }

    auto& c = registry_->template emplace<ComponentT>(entity.id, std::forward<CTorArgs>(args)...);
    entity.components.push_back(Component{.type = std::string{type_name<ComponentT>()}});
    return std::addressof(c);
  }

  /**
   * @brief Removes an entity by its native ID
   */
  void remove(entt::entity e)
  {
    for (const auto& [handle, value] : handle_to_value_cache_)
    {
      if (value->id == e)
      {
        handle_to_value_cache_.erase(handle);
        registry_->destroy(e);
        return;
      }
    }
  }

private:
  entt::registry* registry_;

  expected<void, EntityError> reload(Entity& entity);
  expected<void, EntityError> unload(const Entity& entity);
  expected<Entity, EntityError> generate();
};

}  // namespace sde::game
