/**
 * @copyright 2024-present Brian Cairl
 *
 * @file scene.hpp
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
#include "sde/game/systems_fwd.hpp"
#include "sde/resource.hpp"
#include "sde/type_name.hpp"

namespace sde::game
{

/**
 * @brief Loaded game script data
 */
struct ScriptData : public Resource<ScriptData>
{
  std::string name;
  ScriptRuntimeUPtr script;

  ScriptData(std::string&& _name, ScriptRuntimeUPtr _script);

  auto field_list() { return FieldList(Field{"name", name}, _Stub{"script", script}); }
};

struct ComponentData
{
  std::function<void(IArchive&, entt::entity, entt::registry&)> load;
  std::function<void(OArchive&, entt::entity, const entt::registry&)> save;
};

enum class SceneError
{
  kPathInvalid,
  kPathMissingFiles,
  kPathFailedToCreate,
  kFailedToSave,
  kFailedToLoad,
  kScriptAlreadyAdded,
  kComponentAlreadyAdded,
};

/**
 * @brief All active game data
 */
class Scene : public Resource<Scene>
{
  friend fundemental_type;

public:
  expected<void, SceneError> save(const asset::path& path) const;

  expected<void, SceneError> load(const asset::path& path);

  expected<void, SceneError> addScript(std::string name, ScriptRuntimeUPtr script);

  void removeScript(const std::string& name);

  expected<void, SceneError> addComponent(std::string name, ComponentData component);

  template <typename ComponentT> expected<void, SceneError> addComponent()
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
    return this->addComponent(std::string{type_name<ComponentT>()}, std::move(data));
  }

  void removeComponent(const std::string& name);

  bool tick(AppState& state, const AppProperties& properties);

private:
  Assets assets_;

  std::vector<ScriptData> scripts_;
  std::unordered_map<std::string, ComponentData> components_;

  auto field_list() { return FieldList(Field{"assets", assets_}, _Stub{"scripts", scripts_}); }
};

}  // namespace sde::game
