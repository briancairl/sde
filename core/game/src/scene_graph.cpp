// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/assets.hpp"
#include "sde/game/native_script.hpp"
#include "sde/game/scene.hpp"
#include "sde/game/scene_graph.hpp"
#include "sde/game/scene_handle.hpp"
#include "sde/game/scene_manifest.hpp"
#include "sde/geometry_io.hpp"
#include "sde/logging.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serial/std/optional.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/string.hpp"
#include "sde/unordered_map.hpp"
#include "sde/vector.hpp"

namespace sde::game
{
namespace
{

expected<SceneScriptInstance, SceneGraphErrorCode> instance(
  Assets& assets,
  const SceneScriptData& script_data,
  const LibraryFlags& script_library_flags = {.required = true})
{
  // Find/load script library
  auto script_or_error = assets.scripts.create(script_data.path, script_library_flags);
  if (!script_or_error.has_value())
  {
    SDE_LOG_ERROR_FMT("SceneGraphErrorCode::kInvalidScript (failed to load: %s)", script_data.path.c_str());
    return make_unexpected(SceneGraphErrorCode::kInvalidScript);
  }

  // Create an instance of this script
  return SceneScriptInstance{
    .handle = std::move(script_or_error->handle),
    .instance = (*script_or_error)->script.instance(),
    .instance_data_path = script_data.data};
}

template <typename StringT> StringT toDataFilePath(StringT path)
{
  static constexpr std::string_view kDataFileExtension{".bin"};
  path.reserve(path.size() + kDataFileExtension.size());
  for (auto& c : path)
  {
    if (c == '/' || c == '.')
    {
      c = '_';
    }
  }
  path += kDataFileExtension;
  return path;
}

asset::path getDataFilePath(const SceneScriptInstance& script)
{
  if (script.instance_data_path.has_value())
  {
    return *script.instance_data_path;
  }
  return asset::path{toDataFilePath(sde::string{script.instance.name()})};
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
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kInvalidScriptData)
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kInvalidScriptDataPath)
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kInvalidScriptAssets)
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kPreScriptFailure)
    SDE_OS_ENUM_CASE(SceneGraphErrorCode::kPostScriptFailure)
  }
  return os;
}

SceneGraph::SceneGraph(sde::unique_ptr<Assets>&& assets) : assets_{std::move(assets)} {}

template <typename OnVisitScriptT>
expected<void, SceneGraphError> SceneGraph::visit(const SceneHandle scene_handle, OnVisitScriptT on_visit_script)
{
  auto this_scene = assets_->scenes(scene_handle);

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
    if (const auto ok_or_error = visit(child_handle, on_visit_script); !ok_or_error.has_value())
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
SceneGraph::visit_scene(const SceneHandle scene_handle, OnVisitSceneT on_visit_scene) const
{
  auto this_scene = assets_->scenes(scene_handle);

  SDE_ASSERT_TRUE(this_scene) << scene_handle << " is not a valid scene handle";

  on_visit_scene(scene_handle, *this_scene);

  for (const auto& child_handle : this_scene->children)
  {
    if (const auto ok_or_error = visit_scene(child_handle, on_visit_scene); !ok_or_error.has_value())
    {
      SDE_LOG_ERROR() << ok_or_error.error();
      return make_unexpected(ok_or_error.error());
    }
  }
  return {};
}

expected<void, SceneGraphError> SceneGraph::load(const asset::path& directory)
{
  return SceneGraph::visit(root_, [&](const SceneScriptInstance& script) -> bool {
    if (!script.instance_data_path.has_value())
    {
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

expected<void, SceneGraphError> SceneGraph::save(const asset::path& directory)
{
  return SceneGraph::visit(root_, [&directory](const SceneScriptInstance& script) -> bool {
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

expected<SceneManifest, SceneGraphError> SceneGraph::manifest() const
{
  const auto to_scene_name = [this](const SceneHandle& scene_handle) {
    const auto scene = assets_->scenes(scene_handle);
    SDE_ASSERT_TRUE(scene);
    return static_cast<const sde::string&>(scene->name);
  };

  const auto to_scene_script_data = [this](const SceneScriptInstance& instance) -> SceneScriptData {
    const auto script = assets_->scripts(instance.handle);
    SDE_ASSERT_TRUE(script);
    const auto library = assets_->libraries(script->library);
    SDE_ASSERT_TRUE(library);
    return {.path = library->path, .data = getDataFilePath(instance)};
  };

  // Creata a scene
  SceneManifest manifest;
  manifest.setPath(this->path());
  manifest.setRoot(to_scene_name(root_));
  auto ok_or_error =
    SceneGraph::visit_scene(root_, [&]([[maybe_unused]] const SceneHandle handle, const SceneData& scene) {
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

expected<void, SceneGraphError> SceneGraph::initialize(const AppProperties& properties)
{
  return SceneGraph::visit(root_, [&](const SceneScriptInstance& script) -> bool {
    if (const auto ok_or_error = script.instance.initialize(*assets_, properties); ok_or_error.has_value())
    {
      return true;
    }

    {
      SDE_LOG_ERROR_FMT("Failed to initialize: %s", script.instance.name());
      return false;
    }
  });
}

expected<void, SceneGraphError> SceneGraph::tick(const AppProperties& properties)
{
  return SceneGraph::visit(root_, [&](const SceneScriptInstance& script) -> bool {
    if (const auto ok_or_error = script.instance.call(*assets_, properties); ok_or_error.has_value())
    {
      return true;
    }

    {
      SDE_LOG_ERROR_FMT("Failed to initialize: %s", script.instance.name());
      return false;
    }
  });
}

expected<SceneGraph, SceneGraphErrorCode>
SceneGraph::create(const SceneManifest& manifest, sde::unique_ptr<Assets>&& assets)
{
  SceneGraph graph{std::move(assets)};

  sde::unordered_map<sde::string, SceneHandle> name_to_handle;
  for (const auto& [scene_name, scene_data] : manifest.scenes())
  {
    sde::vector<SceneScriptInstance> pre_scripts;
    for (const auto& script_data : scene_data.pre_scripts)
    {
      if (auto script_instance_or_error = instance(*graph.assets_, script_data); script_instance_or_error.has_value())
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
      if (auto script_instance_or_error = instance(*graph.assets_, script_data); script_instance_or_error.has_value())
      {
        post_scripts.push_back(std::move(script_instance_or_error).value());
      }
      else
      {
        return make_unexpected(script_instance_or_error.error());
      }
    }

    auto scene_or_error = graph.assets_->scenes.create(scene_name, std::move(pre_scripts), std::move(post_scripts));
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
    graph.assets_->scenes.update_if_exists(
      scene_itr->second, [&name_to_handle, &children = scene_data.children](auto& scene) {
        for (const auto& child_name : children)
        {
          const auto child_script_itr = name_to_handle.find(child_name);
          SDE_ASSERT_NE(child_script_itr, std::end(name_to_handle));
          scene.children.push_back(child_script_itr->second);
        }
      });
  }

  graph.path_ = manifest.path();

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

expected<SceneGraph, SceneGraphErrorCode> SceneGraph::create(const SceneManifest& manifest)
{
  return SceneGraph::create(manifest, sde::make_unique<Assets>());
}

expected<SceneGraph, SceneGraphErrorCode> SceneGraph::create(const asset::path& manifest_path)
{
  auto scene_manifest_or_error = SceneManifest::create(manifest_path);
  if (!scene_manifest_or_error.has_value())
  {
    SDE_LOG_ERROR() << scene_manifest_or_error.error();
    return make_unexpected(SceneGraphErrorCode::kInvalidSceneManifest);
  }

  auto scene_graph_or_error = SceneGraph::create(*scene_manifest_or_error);
  if (!scene_graph_or_error.has_value())
  {
    return make_unexpected(scene_graph_or_error.error());
  }

  const auto assets_path = scene_graph_or_error->assetPath();
  if (auto ifs_or_error = serial::file_istream::create(assets_path); ifs_or_error.has_value())
  {
    IArchive iar{*ifs_or_error};
    iar >> Field{"assets", *scene_graph_or_error->assets_};
    if (auto ok_or_error = scene_graph_or_error->assets_->refresh(); !ok_or_error.has_value())
    {
      SDE_LOG_ERROR() << ok_or_error.error() << " " << SDE_OSNV(assets_path);
      return make_unexpected(SceneGraphErrorCode::kInvalidScriptAssets);
    }
  }
  else if (ifs_or_error.error() == serial::FileStreamError::kFileDoesNotExist)
  {
    SDE_LOG_WARN() << SDE_OSNV(assets_path);
  }
  else
  {
    SDE_LOG_ERROR() << ifs_or_error.error() << " " << SDE_OSNV(assets_path);
    return make_unexpected(SceneGraphErrorCode::kInvalidScriptAssets);
  }

  if (!asset::exists(scene_manifest_or_error->path()) and !asset::create_directories(scene_manifest_or_error->path()))
  {
    SDE_LOG_ERROR() << "Invalid script data directory: " << scene_manifest_or_error->path();
    return make_unexpected(SceneGraphErrorCode::kInvalidScriptDataPath);
  }

  if (auto ok_or_error = scene_graph_or_error->load(scene_manifest_or_error->path()); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(ok_or_error.error().code);
  }

  return {std::move(scene_graph_or_error).value()};
}

expected<void, SceneGraphErrorCode> SceneGraph::dump(SceneGraph& graph, const asset::path& manifest_path)
{
  if (!asset::exists(graph.path()))
  {
    SDE_LOG_ERROR() << "Invalid script data directory: " << graph.path();
    return make_unexpected(SceneGraphErrorCode::kInvalidScriptDataPath);
  }

  const auto assets_path = graph.assetPath();
  if (auto ofs_or_error = serial::file_ostream::create(assets_path); ofs_or_error.has_value())
  {
    OArchive oar{*ofs_or_error};
    oar << Field{"assets", *graph.assets_};
  }
  else
  {
    return make_unexpected(SceneGraphErrorCode::kInvalidScriptAssets);
  }

  if (auto ok_or_error = graph.save(graph.path()); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(ok_or_error.error().code);
  }

  auto updated_manifest_or_error = graph.manifest();
  if (!updated_manifest_or_error.has_value())
  {
    SDE_LOG_ERROR() << updated_manifest_or_error.error();
    return make_unexpected(updated_manifest_or_error.error().code);
  }

  if (auto ok_or_error = updated_manifest_or_error->save(manifest_path); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(SceneGraphErrorCode::kInvalidSceneManifest);
  }

  return {};
}

}  // namespace sde::game
