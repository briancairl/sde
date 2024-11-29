// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/assets.hpp"
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

}  // namespace

std::ostream& operator<<(std::ostream& os, SceneGraphErrorCode error)
{
  switch (error)
  {
    SDE_OSTREAM_ENUM_CASE(SceneGraphErrorCode::kInvalidSceneCreation)
    SDE_OSTREAM_ENUM_CASE(SceneGraphErrorCode::kInvalidSceneRoot)
    SDE_OSTREAM_ENUM_CASE(SceneGraphErrorCode::kInvalidScript)
    SDE_OSTREAM_ENUM_CASE(SceneGraphErrorCode::kInvalidScriptData)
    SDE_OSTREAM_ENUM_CASE(SceneGraphErrorCode::kPreScriptFailure)
    SDE_OSTREAM_ENUM_CASE(SceneGraphErrorCode::kPostScriptFailure)
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const SceneGraphError& error)
{
  return os << "{code: " << error.code << ", scene: " << error.scene << ", script_name: " << error.script_name << '}';
}

SceneGraph::SceneGraph(Assets& assets) : assets_{std::addressof(assets)}, root_{SceneHandle::null()} {}

template <typename OnVisitT>
expected<void, SceneGraphError> SceneGraph::visit(const SceneHandle scene_handle, OnVisitT on_visit) const
{
  auto this_scene = assets_->scenes(scene_handle);
  SDE_ASSERT_TRUE(this_scene);

  // Call scripts which run before children
  for (const auto& script : this_scene->pre_scripts)
  {
    if (on_visit(script))
    {
      return make_unexpected(
        SceneGraphError{SceneGraphErrorCode::kPreScriptFailure, scene_handle, script.instance.name()});
    }
  }

  // Run child scenes
  for (const auto& child_handle : this_scene->children)
  {
    if (const auto ok_or_error = visit(child_handle, on_visit); !ok_or_error.has_value())
    {
      return make_unexpected(ok_or_error.error());
    }
  }

  // Call scripts which run after children
  for (const auto& script : this_scene->post_scripts)
  {
    if (on_visit(script))
    {
      return make_unexpected(
        SceneGraphError{SceneGraphErrorCode::kPostScriptFailure, scene_handle, script.instance.name()});
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
      IArchive iar{*ifs_or_error};
      return script.instance.load(iar);
    }
    SDE_LOG_ERROR_FMT("Failed to open data file: %s", data_file_path.c_str());
    return false;
  });
}

expected<void, SceneGraphError> SceneGraph::save(const asset::path& directory) const
{
  return SceneGraph::visit(root_, [&](const SceneScriptInstance& script) -> bool {
    const auto data_file_path = directory / [&script]() -> asset::path {
      if (script.instance_data_path.has_value())
      {
        return *script.instance_data_path;
      }
      else
      {
        return asset::path{toDataFilePath(sde::string{script.instance.name()})};
      }
    }();


    // Load script data for this instance
    if (auto ofs_or_error = serial::file_ostream::create(data_file_path); ofs_or_error.has_value())
    {
      SDE_LOG_INFO_FMT("Saving to open data file: %s", data_file_path.c_str());
      OArchive oar{*ofs_or_error};
      return script.instance.save(oar);
    }
    SDE_LOG_ERROR_FMT("Failed to open data file: %s", data_file_path.c_str());
    return false;
  });
}

expected<void, SceneGraphError> SceneGraph::initialize(const AppProperties& properties) const
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

expected<void, SceneGraphError> SceneGraph::tick(const AppProperties& properties) const
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

expected<SceneGraph, SceneGraphErrorCode> SceneGraph::create(Assets& assets, const SceneManifest& manifest)
{
  sde::unordered_map<sde::string, SceneHandle> name_to_handle;
  for (const auto& [scene_name, scene_data] : manifest.scenes())
  {
    sde::vector<SceneScriptInstance> pre_scripts;
    for (const auto& script_data : scene_data.pre_scripts)
    {
      if (auto script_instance_or_error = instance(assets, script_data); script_instance_or_error.has_value())
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
      if (auto script_instance_or_error = instance(assets, script_data); script_instance_or_error.has_value())
      {
        post_scripts.push_back(std::move(script_instance_or_error).value());
      }
      else
      {
        return make_unexpected(script_instance_or_error.error());
      }
    }

    auto scene_or_error = assets.scenes.create(scene_name, std::move(pre_scripts), std::move(post_scripts));
    if (!scene_or_error.has_value())
    {
      SDE_LOG_ERROR_FMT("SceneGraphErrorCode::kInvalidSceneCreation (failed to create scene: %s)", scene_name.c_str());
      return make_unexpected(SceneGraphErrorCode::kInvalidSceneCreation);
    }

    [[maybe_unused]] const auto [_, added] = name_to_handle.emplace(scene_name, std::move(scene_or_error->handle));
    SDE_ASSERT_TRUE(added);
  }

  for (const auto& [scene_name, scene_data] : manifest.scenes())
  {
    const auto scene_itr = name_to_handle.find(scene_name);
    SDE_ASSERT_NE(scene_itr, std::end(name_to_handle));
    assets.scenes.update_if_exists(scene_itr->second, [&name_to_handle, &children = scene_data.children](auto& scene) {
      for (const auto& child_name : children)
      {
        const auto child_script_itr = name_to_handle.find(child_name);
        SDE_ASSERT_NE(child_script_itr, std::end(name_to_handle));
        scene.children.push_back(child_script_itr->second);
      }
    });
  }

  SceneGraph graph{assets};
  {
    const auto scene_itr = name_to_handle.find(manifest.root());
    SDE_ASSERT_NE(scene_itr, std::end(name_to_handle));
    graph.root_ = scene_itr->second;
  }

  if (graph.root_.isNull())
  {
    return make_unexpected(SceneGraphErrorCode::kInvalidSceneRoot);
  }
  return graph;
}

}  // namespace sde::game
