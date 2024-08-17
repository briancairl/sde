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

/**
 * @brief Ordered sequence of scripts
 */
struct Scripts : public Resource<Scripts>
{
public:
  expected<void, SceneError> add(std::string name, ScriptRuntimeUPtr script);

  void remove(const std::string& name);

private:
  std::vector<ScriptData> scripts_;
};

struct ComponentData : public Resource<ComponentData>
{
  std::function<void(IArchive&, entt::entity, entt::registry&)> load;
  std::function<void(OArchive&, entt::entity, const entt::registry&)> save;
};

struct Components : public Resource<Components>
{
public:
  expected<void, SceneError> add(std::string name, ComponentData component);

  template <typename ComponentT> expected<void, SceneError> add()
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

enum class SceneError {
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

  bool tick(AppState& state, const AppProperties& properties);

private:
  auto field_list() { return FieldList(_Stub{"pre_scripts", pre_scripts_}, _Stub{"post_scripts", post_scripts_}); }
  Scripts pre_scripts_;
  Scripts post_scripts_;
};

class SceneNode : public Resource<SceneNode>
{
  Scene scene;
  std::vector<std::string> children;
};

class SceneGraph : public Resource<SceneGraph>
{
public:
  bool tick(AppState& state, const AppProperties& properties, const std::string& start = "root");

private:
  auto field_list()
  {
    return FieldList(_Stub{"assets", assets}, _Stub{"components", components_}, _Stub{"graph", graph_});
  }

  Assets assets_;
  Components components_;
  std::unordered_map<std::string, SceneNode> graph_;
};

}  // namespace sde::game
