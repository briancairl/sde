// C++ Standard Library
#include <exception>
#include <fstream>

// JSON
#include <nlohmann/json.hpp>

// SDE
#include "sde/game/game_manifest.hpp"
#include "sde/game/game_resources.hpp"
#include "sde/logging.hpp"
#include "sde/time.hpp"

namespace sde::game
{
namespace
{

[[nodiscard]] bool load_components_from_manifest(GameResources& resources, const nlohmann::json& components_json)
{
  for (const auto& [component_key_json, component_lib_path_json] : components_json.items())
  {
    const sde::string component_name{component_key_json};
    const asset::path component_path{component_lib_path_json};
    if (auto component_or_error =
          resources.find_or_create<ComponentCache>(component_name, component_name, component_path);
        !component_or_error.has_value())
    {
      SDE_LOG_ERROR() << "Failed to load component: " << SDE_OSNV(component_name) << " from "
                      << SDE_OSNV(component_path) << " with error: " << component_or_error.error();
      return false;
    }
    else if (component_or_error->status == ResourceStatus::kCreated)
    {
      SDE_LOG_INFO() << "Component loaded: " << SDE_OSNV(component_name) << " from " << SDE_OSNV(component_path);
    }
    else
    {
      SDE_LOG_INFO() << "Component found: " << SDE_OSNV(component_name) << " from " << SDE_OSNV(component_path);
    }
  }
  return true;
}

[[nodiscard]] bool load_scripts_from_manifest(GameResources& resources, const nlohmann::json& scripts_json)
{
  for (const auto& [script_key_json, script_lib_path_json] : scripts_json.items())
  {
    const sde::string script_name{script_key_json};
    const asset::path script_path{script_lib_path_json};
    if (auto ok_or_error = resources.find_or_create<NativeScriptCache>(script_name, script_name, script_path);
        !ok_or_error.has_value())
    {
      SDE_LOG_ERROR() << "Failed to load script: " << SDE_OSNV(script_name) << " from " << SDE_OSNV(script_path)
                      << " with error: " << ok_or_error.error();
      return false;
    }
    else if (ok_or_error->status == ResourceStatus::kCreated)
    {
      SDE_LOG_INFO() << "Script loaded: " << SDE_OSNV(script_name) << " from " << SDE_OSNV(script_path);
    }
    else
    {
      SDE_LOG_INFO() << "Script found: " << SDE_OSNV(script_name) << " from " << SDE_OSNV(script_path);
    }
  }
  return true;
}

[[nodiscard]] bool load_scenes_from_manifest(GameResources& resources, const nlohmann::json& scenes_json)
{
  for (const auto& [scene_key_json, scene_nodes_json] : scenes_json.items())
  {
    const sde::string scene_name{scene_key_json};
    if (auto scene_or_error = resources.find_or_create<SceneCache>(scene_name, scene_name); !scene_or_error.has_value())
    {
      SDE_LOG_ERROR() << "Scene: " << SDE_OSNV(scene_name)
                      << " : creation failed with error: " << scene_or_error.error();
      return false;
    }
    else if (scene_or_error->status == ResourceStatus::kCreated)
    {
      SDE_LOG_INFO() << "Scene: " << SDE_OSNV(scene_name) << " added";
    }
    else
    {
      SDE_LOG_INFO() << "Scene: " << SDE_OSNV(scene_name) << " found";
    }
  }

  for (const auto& [scene_key_json, scene_nodes_json] : scenes_json.items())
  {
    const sde::string scene_name{scene_key_json};

    // Remove previous nodes
    resources.get<SceneCache>().update_if_exists(scene_name, [](auto& scene) { scene.nodes.clear(); });

    // Update topology from manifest
    for (const auto& node_json : scene_nodes_json)
    {
      if (node_json.is_string())
      {
        const sde::string child_scene_name{node_json};

        if (const auto child_scene_handle = resources.get<SceneCache>().to_handle(child_scene_name); child_scene_handle)
        {
          resources.get<SceneCache>().update_if_exists(
            scene_name, [child_scene_handle](auto& scene) { scene.nodes.push_back({.child = child_scene_handle}); });
        }
        else
        {
          SDE_LOG_ERROR() << "Scene: " << scene_name << " : invalid child scene " << SDE_OSNV(child_scene_name);
          return false;
        }
      }
      else if (node_json.is_object())
      {
        const sde::string script_type{node_json["script"]};

        const auto script = resources.get<NativeScriptCache>().find(script_type);

        if (!script)
        {
          SDE_LOG_ERROR() << "Scene: " << scene_name << " : failed to retrieve script: " << SDE_OSNV(script_type);
          return false;
        }

        const sde::string script_name{node_json["name"].is_null() ? node_json["script"] : node_json["name"]};

        if (auto instance_or_error =
              resources.find_or_create<NativeScriptInstanceCache>(script_name, script_name, script.handle);
            instance_or_error.has_value())
        {
          resources.get<SceneCache>().update_if_exists(
            scene_name, [script_handle = instance_or_error->handle](auto& scene) {
              scene.nodes.push_back({.script = script_handle});
            });
        }
        else
        {
          SDE_LOG_ERROR() << "Scene: " << scene_name << " : failed to instance script: " << SDE_OSNV(script_name)
                          << " from " << SDE_OSNV(script_type) << " with error: " << instance_or_error.error();
          return false;
        }
      }
      else
      {
        SDE_FAIL() << "Invalid scene entry";
      }
    }
  }
  return true;
}

nlohmann::json getNode(const nlohmann::json& node, const std::string& name)
{
  auto sub = node[name];
  SDE_ASSERT_FALSE(sub.is_null()) << SDE_OSNV(name) << " is not a valid key";
  return sub;
}

}  // namespace

GameConfiguration GameConfiguration::load(const asset::path& config_path)
{
  SDE_ASSERT_TRUE(asset::exists(config_path)) << SDE_OSNV(config_path) << " does not exist";
  SDE_ASSERT_TRUE(asset::is_regular_file(config_path)) << SDE_OSNV(config_path) << " must be a regular file (JSON)";

  // Load scene manifest (JSON)
  nlohmann::json config_json;
  SDE_ASSERT_NO_EXCEPT(
    {
      std::ifstream ifs{config_path};
      ifs >> config_json;
    },
    std::exception);

  GameConfiguration config;
  config.config_path = config_path;
  config.rate = Rate::fromHertz(static_cast<float>(getNode(config_json, "rate")));
  config.working_directory = asset::path{getNode(config_json, "working_directory")};
  config.script_directory = asset::path{getNode(config_json, "script_directory")};
  config.assets_data_path = asset::path{getNode(config_json, "assets_data_path")};
  config.window_icon_path = asset::path{getNode(config_json, "window_icon_path")};
  config.manifest_path = asset::path{getNode(config_json, "manifest_path")};

  return config;
}

SceneHandle load_manifest(GameResources& resources, const GameConfiguration& config)
{
  SDE_ASSERT_TRUE(asset::exists(config.manifest_path)) << SDE_OSNV(config.manifest_path) << " does not exist";
  SDE_ASSERT_TRUE(asset::is_regular_file(config.manifest_path))
    << SDE_OSNV(config.manifest_path) << " must be a regular file (JSON)";

  // Load scene manifest (JSON)
  nlohmann::json manifest_json;
  SDE_ASSERT_NO_EXCEPT(
    {
      std::ifstream ifs{config.manifest_path};
      ifs >> manifest_json;
    },
    std::exception);

  // Load components from manifest
  SDE_ASSERT_TRUE(load_components_from_manifest(resources, getNode(manifest_json, "components")))
    << "Failed to load components";
  SDE_ASSERT_TRUE(load_scripts_from_manifest(resources, getNode(manifest_json, "scripts"))) << "Failed to load scripts";
  SDE_ASSERT_TRUE(load_scenes_from_manifest(resources, getNode(manifest_json, "scenes"))) << "Failed to load scenes";

  SceneHandle root_scene = resources.get<SceneCache>().to_handle(getNode(manifest_json, "root"));
  SDE_ASSERT_TRUE(root_scene) << "Root scene is invalid";

  return root_scene;
}

}  // namespace sde::game
