// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/game_resources.hpp"
#include "sde/game/native_script.hpp"
#include "sde/game/scene.hpp"
#include "sde/game/scene_graph.hpp"
#include "sde/game/scene_handle.hpp"
#include "sde/game/scene_manifest.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/string.hpp"
#include "sde/unordered_map.hpp"
#include "sde/vector.hpp"

namespace sde::game
{
namespace
{

expected<SceneScriptInstance, SceneGraphErrorCode> instance(
  GameResources& resources,
  const SceneScriptData& script_data,
  const LibraryFlags& script_library_flags = {.required = true})
{
  // Find/load script library
  auto library_or_error =
    resources.find_or_create<LibraryCache>(script_data.path, script_data.path, script_library_flags);
  if (!library_or_error.has_value())
  {
    SDE_LOG_ERROR() << "SceneGraphErrorCode::kInvalidScript (failed to load: " << script_data.path << ')';
    return make_unexpected(SceneGraphErrorCode::kInvalidScript);
  }

  // Find/load script library
  auto script_or_error =
    resources.find_or_create<NativeScriptCache>(library_or_error->handle, library_or_error->handle);
  if (!script_or_error.has_value())
  {
    SDE_LOG_ERROR() << "SceneGraphErrorCode::kInvalidScript (failed to load: " << script_data.path << ')';
    return make_unexpected(SceneGraphErrorCode::kInvalidScript);
  }

  // Instance the script
  auto script_instance = (*script_or_error)->script.instance();

  // Check that previous script version matches current
  if (script_data.version.has_value() and (script_data.version != script_instance.version()))
  {
    SDE_LOG_WARN() << "Script has new version: old: " << SDE_OSNV(*script_data.version)
                   << " --> new: " << SDE_OSNV(script_instance.version());
    return SceneScriptInstance{
      .handle = std::move(script_or_error->handle),
      .instance = std::move(script_instance),
      .instance_data_path = std::nullopt,
      .instance_version_target = std::nullopt};
  }
  else
  {
    return SceneScriptInstance{
      .handle = std::move(script_or_error->handle),
      .instance = std::move(script_instance),
      .instance_data_path = script_data.data,
      .instance_version_target = script_data.version};
  }
}

asset::path createDataFilePath(const SceneScriptInstance& script)
{
  sde::string path = format(
    "%s_%lu_%lu_%p",
    script.instance.name().data(),
    script.handle.id(),
    script.instance.version(),
    std::addressof(script));
  for (auto& c : path)
  {
    if (c == '/' || c == '.')
    {
      c = '_';
    }
  }
  path += ".bin";
  return asset::path{std::move(path)};
}

asset::path getDataFilePath(const SceneScriptInstance& script)
{
  if (script.instance_data_path.has_value())
  {
    return *script.instance_data_path;
  }
  else
  {
    return createDataFilePath(script);
  }
}

}  // namespace

std::ostream& operator<<(std::ostream& os, SceneGraphErrorCode error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kInvalidSceneManifest)
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kInvalidSceneCreation)
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kInvalidSceneRoot)
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kInvalidScript)
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kPreScriptFailure)
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kPostScriptFailure)
  }
  return os;
}

template <typename OnVisitScriptT>
expected<void, SceneGraphError>
SceneGraph::visit(GameResources& resources, const SceneHandle scene_handle, OnVisitScriptT on_visit_script)
{
  auto this_scene = resources(scene_handle);

  SDE_ASSERT_TRUE(this_scene) << scene_handle << " is not a valid scene handle";

  // Call scripts which run before children
  for (const auto& script : this_scene->pre_scripts)
  {
    if (!on_visit_script(script))
    {
      return make_unexpected(
        SceneGraphError{SceneGraphErrorCode::kPreScriptFailure, scene_handle, script.instance.name()});
    }
  }

  // Run child scenes
  for (const auto& child_handle : this_scene->children)
  {
    if (const auto ok_or_error = visit(resources, child_handle, on_visit_script); !ok_or_error.has_value())
    {
      SDE_LOG_ERROR() << ok_or_error.error();
      return make_unexpected(ok_or_error.error());
    }
  }

  // Call scripts which run after children
  for (const auto& script : this_scene->post_scripts)
  {
    if (!on_visit_script(script))
    {
      return make_unexpected(
        SceneGraphError{SceneGraphErrorCode::kPostScriptFailure, scene_handle, script.instance.name()});
    }
  }
  return {};
}

template <typename OnVisitSceneT>
expected<void, SceneGraphError>
SceneGraph::visit_scene(GameResources& resources, const SceneHandle scene_handle, OnVisitSceneT on_visit_scene) const
{
  auto this_scene = resources(scene_handle);

  SDE_ASSERT_TRUE(this_scene) << scene_handle << " is not a valid scene handle";

  on_visit_scene(scene_handle, *this_scene);

  for (const auto& child_handle : this_scene->children)
  {
    if (const auto ok_or_error = visit_scene(resources, child_handle, on_visit_scene); !ok_or_error.has_value())
    {
      SDE_LOG_ERROR() << ok_or_error.error();
      return make_unexpected(ok_or_error.error());
    }
  }
  return {};
}

expected<void, SceneGraphError> SceneGraph::load(GameResources& resources, const asset::path& directory)
{
  return SceneGraph::visit(resources, root_, [&](const SceneScriptInstance& script) -> bool {
    // Skip loading if there is no data path
    if (!script.instance_data_path.has_value())
    {
      SDE_LOG_WARN() << "Not previous data for: " << script.instance;
      return true;
    }

    const auto data_file_path = directory / (*script.instance_data_path);

    // Load script data for this instance
    if (auto ifs_or_error = serial::file_istream::create(data_file_path); ifs_or_error.has_value())
    {
      SDE_LOG_INFO() << "Loading data from: " << data_file_path;
      IArchive iar{*ifs_or_error};
      return script.instance.load(iar);
    }
    else
    {
      SDE_LOG_ERROR() << "Failed to open data file: " << SDE_OSNV(data_file_path) << " (" << ifs_or_error.error()
                      << ')';
    }
    return false;
  });
}

expected<void, SceneGraphError> SceneGraph::save(GameResources& resources, const asset::path& directory)
{
  return SceneGraph::visit(resources, root_, [&directory](const SceneScriptInstance& script) -> bool {
    const auto data_file_path = directory / getDataFilePath(script);

    // Load script data for this instance
    if (auto ofs_or_error = serial::file_ostream::create(data_file_path); ofs_or_error.has_value())
    {
      SDE_LOG_INFO() << "Saving to open data file: " << SDE_OSNV(data_file_path);
      OArchive oar{*ofs_or_error};
      return script.instance.save(oar);
    }
    else
    {
      SDE_LOG_ERROR() << "Failed to open data file: " << SDE_OSNV(data_file_path) << " (" << ofs_or_error.error()
                      << ')';
    }
    return false;
  });
}

expected<SceneManifest, SceneGraphError> SceneGraph::manifest(GameResources& resources) const
{
  const auto to_scene_name = [&](const SceneHandle& scene_handle) {
    const auto scene = resources(scene_handle);
    SDE_ASSERT_TRUE(scene);
    return static_cast<const sde::string&>(scene->name);
  };

  const auto to_scene_script_data = [&](const SceneScriptInstance& instance) -> SceneScriptData {
    const auto script = resources(instance.handle);
    SDE_ASSERT_TRUE(script);
    const auto library = resources(script->library);
    SDE_ASSERT_TRUE(library);
    return {.path = library->path, .data = getDataFilePath(instance), .version = instance.instance.version()};
  };

  // Creata a scene
  SceneManifest manifest;
  manifest.setRoot(to_scene_name(root_));
  auto ok_or_error =
    SceneGraph::visit_scene(resources, root_, [&]([[maybe_unused]] const SceneHandle handle, const SceneData& scene) {
      SceneManifestEntry manifest_entry;
      manifest_entry.pre_scripts.reserve(scene.pre_scripts.size());
      std::transform(
        std::begin(scene.pre_scripts),
        std::end(scene.pre_scripts),
        std::back_inserter(manifest_entry.pre_scripts),
        to_scene_script_data);
      manifest_entry.post_scripts.reserve(scene.post_scripts.size());
      std::transform(
        std::begin(scene.post_scripts),
        std::end(scene.post_scripts),
        std::back_inserter(manifest_entry.post_scripts),
        to_scene_script_data);
      manifest_entry.children.reserve(scene.children.size());
      std::transform(
        std::begin(scene.children),
        std::end(scene.children),
        std::back_inserter(manifest_entry.children),
        to_scene_name);
      manifest.setScene(scene.name, std::move(manifest_entry));
    });

  if (ok_or_error.has_value())
  {
    return {std::move(manifest)};
  }
  return make_unexpected(ok_or_error.error());
}

expected<void, SceneGraphError> SceneGraph::initialize(GameResources& resources, const AppProperties& properties)
{
  return SceneGraph::visit(resources, root_, [&](const SceneScriptInstance& script) -> bool {
    if (const auto ok_or_error = script.instance.initialize(resources, properties); ok_or_error.has_value())
    {
      return true;
    }

    {
      SDE_LOG_ERROR() << "Failed to initialize: " << script.instance;
      return false;
    }
  });
}

expected<void, SceneGraphError> SceneGraph::tick(GameResources& resources, const AppProperties& properties)
{
  return SceneGraph::visit(resources, root_, [&](const SceneScriptInstance& script) -> bool {
    if (const auto ok_or_error = script.instance.call(resources, properties); ok_or_error.has_value())
    {
      return true;
    }

    {
      SDE_LOG_ERROR() << "Failed to update: " << script.instance;
      return false;
    }
  });
}

expected<SceneGraph, SceneGraphErrorCode> SceneGraph::create(GameResources& resources, const SceneManifest& manifest)
{
  SceneGraph graph;

  sde::unordered_map<sde::string, SceneHandle> name_to_handle;
  for (const auto& [scene_name, scene_data] : manifest.scenes())
  {
    sde::vector<SceneScriptInstance> pre_scripts;
    for (const auto& script_data : scene_data.pre_scripts)
    {
      if (auto script_instance_or_error = instance(resources, script_data); script_instance_or_error.has_value())
      {
        pre_scripts.push_back(std::move(script_instance_or_error).value());
      }
      else
      {
        return make_unexpected(script_instance_or_error.error());
      }
    }

    sde::vector<SceneScriptInstance> post_scripts;
    for (const auto& script_data : scene_data.post_scripts)
    {
      if (auto script_instance_or_error = instance(resources, script_data); script_instance_or_error.has_value())
      {
        post_scripts.push_back(std::move(script_instance_or_error).value());
      }
      else
      {
        return make_unexpected(script_instance_or_error.error());
      }
    }

    auto scene_or_error = resources.create<SceneCache>(scene_name, std::move(pre_scripts), std::move(post_scripts));
    if (!scene_or_error.has_value())
    {
      SDE_LOG_ERROR() << "SceneGraphErrorCode::kInvalidSceneCreation (failed to create scene: " << scene_name << ')';
      return make_unexpected(SceneGraphErrorCode::kInvalidSceneCreation);
    }

    [[maybe_unused]] const auto [_, added] = name_to_handle.emplace(scene_name, std::move(scene_or_error->handle));
    SDE_ASSERT_TRUE(added);
  }

  for (const auto& [scene_name, scene_data] : manifest.scenes())
  {
    const auto scene_itr = name_to_handle.find(scene_name);
    SDE_ASSERT_NE(scene_itr, std::end(name_to_handle));
    resources.update_if_exists(scene_itr->second, [&name_to_handle, &children = scene_data.children](auto& scene) {
      for (const auto& child_name : children)
      {
        const auto child_script_itr = name_to_handle.find(child_name);
        SDE_ASSERT_NE(child_script_itr, std::end(name_to_handle));
        scene.children.push_back(child_script_itr->second);
      }
    });
  }

  {
    const auto scene_itr = name_to_handle.find(manifest.root());
    SDE_ASSERT_NE(scene_itr, std::end(name_to_handle));
    graph.root_ = scene_itr->second;
  }

  if (graph.root_.isNull())
  {
    return make_unexpected(SceneGraphErrorCode::kInvalidSceneRoot);
  }
  return {std::move(graph)};
}

}  // namespace sde::game
