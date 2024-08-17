/**
 * @copyright 2024-present Brian Cairl
 *
 * @file components.hpp
 */
#pragma once

// C++ Standard Library
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

// SDE
#include "sde/app_fwd.hpp"
#include "sde/format.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/script_runtime_fwd.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/type_name.hpp"

namespace sde::game
{

enum class ComponentError
{
  kComponentAlreadyAdded,
};

struct ComponentData : Resource<ComponentData>
{
  std::string label;
  std::function<void(IArchive&, entt::entity, entt::registry&)> load;
  std::function<void(OArchive&, entt::entity, const entt::registry&)> save;
};

}  // namespace sde::game

namespace sde
{

template <> struct ResourceCacheTypes<audio::SoundCache>
{
  using error_type = audio::SoundError;
  using handle_type = audio::SoundHandle;
  using value_type = audio::Sound;
};

}  // namespace sde

namespace sde::game
{

struct Components : public Resource<Components>
{
public:
  expected<void, ComponentError> add(std::string name, ComponentData component);

  template <typename ComponentT> expected<void, ComponentError> add()
  {
    ComponentData data{
      .load =
        [](IArchive& ar, entt::entity e, entt::registry& registry) {
          if constexpr (std::is_void_v<decltype(registry.template emplace<ComponentT>(e))>)
          {
            registry.template emplace<ComponentT>(e);
          }
          else
          {
            ar >> registry.template emplace<ComponentT>(e);
          }
        },
      .save =
        [](OArchive& ar, entt::entity e, const entt::registry& registry) {
          if constexpr (!std::is_void_v<decltype(registry.template get<ComponentT>(e))>)
          {
            ar << registry.template get<ComponentT>(e);
          }
        }};
    return this->add(std::string{type_name<ComponentT>()}, std::move(data));
  }

  void remove(const std::string& name);

private:
  std::unordered_map<std::string, ComponentData> components_;
}

}  // namespace sde::game
