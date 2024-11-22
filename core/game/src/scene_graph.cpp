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
#include "sde/serialization_binary_file.hpp"
#include "sde/string.hpp"
#include "sde/unordered_map.hpp"
#include "sde/vector.hpp"

namespace sde::game
{
namespace
{

expected<std::pair<NativeScriptHandle, NativeScriptInstance>, SceneGraphLoadError>
instance(Assets& assets, const nlohmann::json& script_json)
{
  const auto lib_path = asset::path{static_cast<const std::string>(script_json["path"])};
  const auto script_or_error = assets.scripts.create(lib_path, LibraryFlags{.required = true});
  if (!script_or_error.has_value())
  {
    SDE_LOG_ERROR_FMT("failed to load: %s", lib_path.c_str());
    return make_unexpected(SceneGraphLoadError::kInvalidScript);
  }

  auto script_instance = (*script_or_error)->script.instance();

  const auto data_path_json = script_json["data"];

  if (data_path_json.is_null())
  {
    return std::make_pair(script_or_error->handle, std::move(script_instance));
  }

  const asset::path data_path{static_cast<const std::string>(data_path_json)};

  if (auto ifs_or_error = serial::file_istream::create(data_path); ifs_or_error.has_value())
  {
    IArchive iar{*ifs_or_error};
    script_instance.load(iar);
    return std::make_pair(script_or_error->handle, std::move(script_instance));
  }

  SDE_LOG_ERROR_FMT("failed to load script data: %s", data_path.c_str());
  return make_unexpected(SceneGraphLoadError::kInvalidScriptData);
}

}  // namespace
std::ostream& operator<<(std::ostream& os, SceneGraphErrorType error)
{
  switch (error)
  {
  case SceneGraphErrorType::kInvalidRoot:
    return os << "InvalidRoot";
  case SceneGraphErrorType::kPreScriptFailure:
    return os << "PreScriptFailure";
  case SceneGraphErrorType::kPostScriptFailure:
    return os << "PostScriptFailure";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, SceneGraphError error)
{
  return os << "{ error_type: " << error.error_type << ", handle: " << error.handle << " }";
}

std::ostream& operator<<(std::ostream& os, SceneGraphLoadError error)
{
  switch (error)
  {
  case SceneGraphLoadError::kInvalidJSONPath:
    return os << "InvalidJSONPath";
  case SceneGraphLoadError::kInvalidJSONLayout:
    return os << "InvalidJSONLayout";
  case SceneGraphLoadError::kInvalidScript:
    return os << "InvalidScript";
  case SceneGraphLoadError::kInvalidScriptData:
    return os << "InvalidScriptData";
  case SceneGraphLoadError::kInvalidScene:
    return os << "InvalidScene";
  case SceneGraphLoadError::kInvalidRoot:
    return os << "InvalidRoot";
  }
  return os;
}

expected<void, SceneGraphError>
SceneGraph::initialize(const SceneHandle scene_handle, Assets& assets, const AppProperties& properties)
{
  auto this_scene = assets.scenes(scene_handle);
  SDE_ASSERT_TRUE(this_scene);

  // Call scripts which run before children
  for (const auto& [script_handle, script_instance] : this_scene->pre_scripts)
  {
    SDE_ASSERT_TRUE(script_handle);
    SDE_ASSERT_TRUE(script_instance);
    if (const auto ok_or_error = script_instance.initialize(assets, properties); !ok_or_error.has_value())
    {
      return make_unexpected(SceneGraphError{SceneGraphErrorType::kPreScriptFailure, scene_handle});
    }
  }

  // Run child scenes
  for (const auto& child_handle : this_scene->children)
  {
    if (const auto ok_or_error = SceneGraph::initialize(child_handle, assets, properties); !ok_or_error.has_value())
    {
      return make_unexpected(ok_or_error.error());
    }
  }

  // Call scripts which run after children
  for (const auto& [script_handle, script_instance] : this_scene->post_scripts)
  {
    SDE_ASSERT_TRUE(script_handle);
    SDE_ASSERT_TRUE(script_instance);
    if (const auto ok_or_error = script_instance.initialize(assets, properties); !ok_or_error.has_value())
    {
      return make_unexpected(SceneGraphError{SceneGraphErrorType::kPostScriptFailure, scene_handle});
    }
  }

  return {};
}


expected<void, SceneGraphError>
SceneGraph::tick(const SceneHandle scene_handle, Assets& assets, const AppProperties& properties)
{
  const auto this_scene = assets.scenes(scene_handle);
  SDE_ASSERT_TRUE(this_scene);

  // Call scripts which run before children
  for (const auto& [script_handle, script_instance] : this_scene->pre_scripts)
  {
    SDE_ASSERT_TRUE(script_handle);
    SDE_ASSERT_TRUE(script_instance);
    if (const auto ok_or_error = script_instance.call(assets, properties); !ok_or_error.has_value())
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
  for (const auto& [script_handle, script_instance] : this_scene->post_scripts)
  {
    SDE_ASSERT_TRUE(script_handle);
    SDE_ASSERT_TRUE(script_instance);
    if (const auto ok_or_error = script_instance.call(assets, properties); !ok_or_error.has_value())
    {
      return make_unexpected(SceneGraphError{SceneGraphErrorType::kPostScriptFailure, scene_handle});
    }
  }

  return {};
}

expected<void, SceneGraphError> SceneGraph::initialize(Assets& assets, const AppProperties& properties) const
{
  if (root_.isNull())
  {
    return make_unexpected(SceneGraphError{SceneGraphErrorType::kInvalidRoot, root_});
  }
  return SceneGraph::initialize(root_, assets, properties);
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
    const auto pre_scripts_json = scene_json["pre_scripts"];
    if (pre_scripts_json.is_null() or !pre_scripts_json.is_array())
    {
      SDE_LOG_ERROR("SceneGraphLoadError::kInvalidJSONLayout");
      return make_unexpected(SceneGraphLoadError::kInvalidJSONLayout);
    }

    const auto post_scripts_json = scene_json["post_scripts"];
    if (post_scripts_json.is_null() or !post_scripts_json.is_array())
    {
      SDE_LOG_ERROR("SceneGraphLoadError::kInvalidJSONLayout");
      return make_unexpected(SceneGraphLoadError::kInvalidJSONLayout);
    }

    sde::vector<std::pair<NativeScriptHandle, NativeScriptInstance>> pre_scripts;
    for (const auto& script_json : pre_scripts_json)
    {
      if (auto script_instance_or_error = instance(assets, script_json); script_instance_or_error.has_value())
      {
        pre_scripts.push_back(std::move(script_instance_or_error).value());
      }
      else
      {
        return make_unexpected(script_instance_or_error.error());
      }
    }

    sde::vector<std::pair<NativeScriptHandle, NativeScriptInstance>> post_scripts;
    for (const auto& script_json : post_scripts_json)
    {
      if (auto script_instance_or_error = instance(assets, script_json); script_instance_or_error.has_value())
      {
        post_scripts.push_back(std::move(script_instance_or_error).value());
      }
      else
      {
        return make_unexpected(script_instance_or_error.error());
      }
    }

    const sde::string scene_name{key};
    const SceneType scene_type = (scene_name == "root") ? SceneType::kRoot : SceneType::kChild;
    const auto scene_or_error =
      assets.scenes.create(scene_type, scene_name, std::move(pre_scripts), std::move(post_scripts));
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

expected<void, SceneGraphSaveError> SceneGraph::save(Assets& assets, const asset::path& path) { return {}; }

}  // namespace sde::game
