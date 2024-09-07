// C++ Standard Library
#include <fstream>
#include <ostream>

// JSON
#include <nlohmann/json.hpp>

// SDE
#include "sde/game/assets.hpp"
#include "sde/game/native_script.hpp"
#include "sde/game/scene.hpp"
#include "sde/game/scene_graph.hpp"
#include "sde/game/scene_handle.hpp"
#include "sde/logging.hpp"
#include "sde/string.hpp"
#include "sde/unordered_map.hpp"
#include "sde/vector.hpp"

namespace sde::game
{

expected<void, SceneGraphError>
SceneGraph::tick(const SceneHandle scene_handle, Assets& assets, const AppProperties& properties)
{
  const auto this_scene = assets.scenes(scene_handle);
  SDE_ASSERT_TRUE(this_scene);

  // Call scripts which run before children
  for (const auto& script_handle : this_scene->pre_scripts)
  {
    const auto script = assets.scripts(script_handle);
    SDE_ASSERT_TRUE(script);
    if (const auto ok_or_error = script->script(assets, properties); !ok_or_error.has_value())
    {
      return make_unexpected(SceneGraphError{SceneGraphErrorType::kPreScriptFailure, scene_handle});
    }
  }

  // Run child scenes
  for (const auto& child_handle : this_scene->children)
  {
    if (const auto ok_or_error = SceneGraph::tick(child_handle, assets, properties); !ok_or_error.has_value())
    {
      return make_unexpected(ok_or_error.error());
    }
  }

  // Call scripts which run after children
  for (const auto& script_handle : this_scene->post_scripts)
  {
    const auto script = assets.scripts(script_handle);
    SDE_ASSERT_TRUE(script);
    if (const auto ok_or_error = script->script(assets, properties); !ok_or_error.has_value())
    {
      return make_unexpected(SceneGraphError{SceneGraphErrorType::kPostScriptFailure, scene_handle});
    }
  }

  return {};
}

expected<void, SceneGraphError> SceneGraph::tick(Assets& assets, const AppProperties& properties) const
{
  if (root_.isNull())
  {
    return make_unexpected(SceneGraphError{SceneGraphErrorType::kInvalidRoot, root_});
  }
  return SceneGraph::tick(root_, assets, properties);
}

expected<SceneGraph, SceneGraphLoadError> SceneGraph::load(Assets& assets, const asset::path& path)
{
  std::ifstream ifs{path};
  if (!ifs.is_open())
  {
    return make_unexpected(SceneGraphLoadError::kInvalidJSONPath);
  }

  nlohmann::json scene_manifest;
  ifs >> scene_manifest;

  sde::unordered_map<sde::string, SceneHandle> name_to_handle;
  std::optional<SceneHandle> root_scene_handle{std::nullopt};
  for (const auto& [key, scene_json] : scene_manifest.items())
  {
    const auto pre_scripts = scene_json["pre_scripts"];
    if (pre_scripts.is_null() or !pre_scripts.is_array())
    {
      SDE_LOG_ERROR("SceneGraphLoadError::kInvalidJSONLayout");
      return make_unexpected(SceneGraphLoadError::kInvalidJSONLayout);
    }

    const auto post_scripts = scene_json["post_scripts"];
    if (post_scripts.is_null() or !post_scripts.is_array())
    {
      SDE_LOG_ERROR("SceneGraphLoadError::kInvalidJSONLayout");
      return make_unexpected(SceneGraphLoadError::kInvalidJSONLayout);
    }

    sde::vector<NativeScriptHandle> pre_script_handles;
    for (const auto& path_json_element : pre_scripts)
    {
      const auto path = static_cast<const std::string>(path_json_element);
      const auto script_or_error = assets.scripts.create(asset::path{path}, LibraryFlags{.required = true});
      if (script_or_error.has_value())
      {
        pre_script_handles.push_back(script_or_error->handle);
      }
      else
      {
        SDE_LOG_ERROR_FMT("failed to load: %s", path.c_str());
        return make_unexpected(SceneGraphLoadError::kInvalidScript);
      }
    }

    sde::vector<NativeScriptHandle> post_script_handles;
    for (const auto& path_json_element : post_scripts)
    {
      const auto path = static_cast<const std::string>(path_json_element);
      const auto script_or_error = assets.scripts.create(asset::path{path}, LibraryFlags{.required = true});
      if (script_or_error.has_value())
      {
        post_script_handles.push_back(script_or_error->handle);
      }
      else
      {
        SDE_LOG_ERROR_FMT("failed to load: %s", path.c_str());
        return make_unexpected(SceneGraphLoadError::kInvalidScript);
      }
    }

    const sde::string scene_name{key};
    const SceneType scene_type = (scene_name == "root") ? SceneType::kRoot : SceneType::kChild;
    const auto scene_or_error =
      assets.scenes.create(scene_type, scene_name, std::move(pre_script_handles), std::move(post_script_handles));
    if (scene_or_error.has_value())
    {
      name_to_handle.emplace(scene_name, scene_or_error->handle);
    }
    else
    {
      SDE_LOG_ERROR("failed to create root scene");
      return make_unexpected(SceneGraphLoadError::kInvalidScene);
    }

    if (scene_type != SceneType::kRoot)
    {
      continue;
    }

    if (root_scene_handle.has_value())
    {
      SDE_LOG_ERROR("multiple 'root' scenes");
      return make_unexpected(SceneGraphLoadError::kInvalidJSONLayout);
    }
    else
    {
      root_scene_handle = scene_or_error->handle;
    }
  }

  for (const auto& [key, scene_json] : scene_manifest.items())
  {
    const auto children = scene_json["children"];
    if (children.is_null() or !children.is_array())
    {
      SDE_LOG_ERROR("SceneGraphLoadError::kInvalidJSONLayout");
      return make_unexpected(SceneGraphLoadError::kInvalidJSONLayout);
    }

    const sde::string script_name{key};
    const auto parent_script_itr = name_to_handle.find(script_name);
    SDE_ASSERT(parent_script_itr != name_to_handle.end(), "script does not exist");

    assets.scenes.update_if_exists(parent_script_itr->second, [&name_to_handle, &children](auto& scene) {
      for (const auto& child_name : children)
      {
        SDE_LOG_ERROR_FMT("%s", std::string{child_name}.c_str());
        const auto child_script_itr = name_to_handle.find(child_name);
        SDE_ASSERT(child_script_itr != name_to_handle.end(), "script does not exist");
        scene.children.push_back(child_script_itr->second);
      }
    });
  }

  if (root_scene_handle.has_value())
  {
    SceneGraph scene_graph;
    scene_graph.setRoot(*root_scene_handle);
    return scene_graph;
  }
  return make_unexpected(SceneGraphLoadError::kInvalidRoot);
}

}  // namespace sde::game
