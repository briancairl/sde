// C++ Standard Library
#include <algorithm>
#include <fstream>

/// EnTT
#include <entt/entt.hpp>

// JSON
#include <nlohmann/json.hpp>

// SDE
#include "sde/game/scene.hpp"
#include "sde/game/script_runtime.hpp"
#include "sde/logging.hpp"

#include "sde/audio/assets.hpp"
#include "sde/audio/mixer.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/scene.hpp"
#include "sde/game/script.hpp"
#include "sde/game/systems.hpp"
#include "sde/geometry_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::game
{

namespace
{
bool saveWithManifest(nlohmann::json& manifest, const asset::path& parent_path, const Assets& assets)
{
  using namespace sde::serial;
  return IterateUntil(assets, [&manifest, &parent_path](const auto& field) {
    const auto assets_data_path = parent_path / std::string{format("%s.bin", field.name)};

    manifest[field.name] = {{"data", assets_data_path.string()}, {"version", Version(field).value}};

    if (auto ofs_or_error = file_ostream::create(assets_data_path); ofs_or_error.has_value())
    {
      SDE_LOG_DEBUG_FMT("saving: %s(%p)", field.name, field.value);
      binary_oarchive oar{*ofs_or_error};
      oar << field;
      SDE_LOG_DEBUG_FMT("saved: %s(%p)", field.name, field.value);
      return true;
    }
    SDE_LOG_DEBUG_FMT("failed to open: %s", assets_data_path.string().c_str());
    return false;
  });
}

bool loadWithManifest(const nlohmann::json& manifest, const asset::path& parent_path, Assets& assets)
{
  using namespace sde::serial;
  return IterateUntil(assets, [&manifest, &parent_path](auto& field) {
    const auto element_json = manifest[field.name];
    if (element_json.is_null())
    {
      SDE_LOG_DEBUG_FMT("field[%s] has no manifest entry", field.name);
      return false;
    }

    const auto data_json = element_json["data"];
    if (data_json.is_null())
    {
      SDE_LOG_DEBUG_FMT("field[%s] missing 'data' entry", field.name);
      return false;
    }

    const auto version_json = element_json["version"];
    if (version_json.is_null())
    {
      SDE_LOG_DEBUG_FMT("field[%s] missing 'version' entry", field.name);
      return false;
    }

    if (const std::size_t previous_version = version_json; Version(field) != previous_version)
    {
      SDE_LOG_DEBUG_FMT("field[%s] 'version' mismatch %lu vs %lu", field.name, Version(field).value, previous_version);
      return false;
    }

    const auto assets_data_path = parent_path / std::string{format("%s.bin", field.name)};

    if (auto ifs_or_error = file_istream::create(assets_data_path); ifs_or_error.has_value())
    {
      SDE_LOG_DEBUG_FMT("loading: %s(%p)", field.name, field.value);
      binary_iarchive iar{*ifs_or_error};
      iar >> field;
      SDE_LOG_DEBUG_FMT("loaded: %s(%p)", field.name, field.value);
      return true;
    }

    SDE_LOG_DEBUG_FMT("failed to open: %s", assets_data_path.string().c_str());
    return false;
  });
}

bool saveWithManifest(
  nlohmann::json& manifest,
  const asset::path& parent_path,
  const std::vector<ScriptData>& scripts,
  const Assets& assets)
{
  using namespace sde::serial;
  for (const auto& [script_name, script_ptr] : scripts)
  {
    const auto script_data_path = parent_path / std::string{format("%s.bin", script_name.c_str())};

    manifest[script_name] = {{"data", script_data_path.string()}, {"version", script_ptr->version().value}};

    if (auto ofs_or_error = file_ostream::create(script_data_path); ofs_or_error.has_value())
    {
      SDE_LOG_DEBUG_FMT("saving: %s(%p)", script_name.c_str(), script_ptr.get());
      binary_oarchive oar{*ofs_or_error};
      if (auto ok_or_error = script_ptr->save(oar, assets); !ok_or_error.has_value())
      {
        return false;
      }
      SDE_LOG_DEBUG_FMT("saved: %s(%p)", script_name.c_str(), script_ptr.get());
    }
    SDE_LOG_DEBUG_FMT("failed to open: %s", script_data_path.string().c_str());
  }
  return true;
}

bool loadWithManifest(
  const nlohmann::json& manifest,
  const asset::path& parent_path,
  std::vector<ScriptData>& scripts,
  Assets& assets)
{
  using namespace sde::serial;
  for (const auto& [script_name, script_element_json] : manifest.items())
  {
    const auto data_json = script_element_json["data"];
    if (data_json.is_null())
    {
      SDE_LOG_DEBUG("field missing 'data' entry");
      return false;
    }

    const auto version_json = script_element_json["version"];
    if (version_json.is_null())
    {
      SDE_LOG_DEBUG("field missing 'version' entry");
      return false;
    }

    const auto assets_data_path = parent_path / std::string{format("%s.bin", script_name.c_str())};

    auto script_ptr = game::ScriptRuntimeLoader::load(script_name, script_element_json);

    if (const std::size_t previous_version = version_json; script_ptr->version() != previous_version)
    {
      SDE_LOG_DEBUG_FMT(
        "script[%s] 'version' mismatch %lu vs %lu", script_name.c_str(), script_ptr->version().value, previous_version);
      return false;
    }


    if (auto ifs_or_error = file_istream::create(assets_data_path); ifs_or_error.has_value())
    {
      SDE_LOG_DEBUG_FMT("loading: %s", script_name.c_str());
      binary_iarchive iar{*ifs_or_error};
      if (auto ok_or_error = script_ptr->load(iar, assets); !ok_or_error.has_value())
      {
        return false;
      }
      SDE_LOG_DEBUG_FMT("loaded: %s", script_name.c_str());
      scripts.emplace_back(std::string{script_name}, std::move(script_ptr));
    }
  }
  return true;
}

}  // namespace

ScriptData::ScriptData(std::string&& _name, ScriptRuntime::UPtr _script) :
    name{std::move(_name)}, script{std::move(_script)}
{}

expected<void, SceneError> Scene::save(const asset::path& path) const
{
  // Ensure the path is directory-like
  if (path.has_extension())
  {
    SDE_LOG_DEBUG("SceneError::kPathInvalid");
    return make_unexpected(SceneError::kPathInvalid);
  }

  // If path does not exist, create directories
  if (std::error_code errc; !asset::exists(path) and (!asset::create_directories(path) or errc))
  {
    return make_unexpected(SceneError::kPathFailedToCreate);
  }

  nlohmann::json scene_manifest;

  // Write asset data
  if (!saveWithManifest(scene_manifest, path, assets_))
  {
    SDE_LOG_DEBUG("SceneError::kFailedToSave");
    return make_unexpected(SceneError::kFailedToSave);
  }

  const auto scripts_path = path / "scripts";

  // If path does not exist, create directories
  if (std::error_code errc; !asset::exists(scripts_path) and (!asset::create_directories(scripts_path) or errc))
  {
    SDE_LOG_DEBUG("SceneError::kPathFailedToCreate");
    return make_unexpected(SceneError::kPathFailedToCreate);
  }

  // Write script data
  if (nlohmann::json script_manifest; saveWithManifest(script_manifest, scripts_path, scripts_, assets_))
  {
    scene_manifest["scripts"] = std::move(script_manifest);
  }
  else
  {
    SDE_LOG_DEBUG("SceneError::kFailedToSave");
    return make_unexpected(SceneError::kFailedToSave);
  }

  // Write manifest
  const auto manifest_path = path / "manifest.json";
  if (std::ofstream manifest_stream{manifest_path}; manifest_stream.is_open())
  {
    manifest_stream << std::setw(4) << scene_manifest << std::endl;
    return {};
  }
  SDE_LOG_DEBUG("SceneError::kFailedToSave");
  return make_unexpected(SceneError::kFailedToSave);
}

expected<void, SceneError> Scene::load(const asset::path& path)
{
  // Ensure the path is directory-like
  if (path.has_extension() or !asset::exists(path) or !asset::is_directory(path))
  {
    SDE_LOG_DEBUG("SceneError::kPathInvalid");
    return make_unexpected(SceneError::kPathInvalid);
  }

  nlohmann::json scene_manifest;

  // Read manifest
  const auto manifest_path = path / "manifest.json";
  if (std::ifstream manifest_stream{manifest_path}; manifest_stream.is_open())
  {
    scene_manifest = nlohmann::json::parse(manifest_stream);
  }
  else
  {
    SDE_LOG_DEBUG("SceneError::kFailedToLoad");
    return make_unexpected(SceneError::kFailedToLoad);
  }

  // Read asset data
  if (!loadWithManifest(scene_manifest, path, assets_))
  {
    SDE_LOG_DEBUG("SceneError::kFailedToLoad");
    return make_unexpected(SceneError::kFailedToLoad);
  }

  const auto script_manifest = scene_manifest["scripts"];
  if (script_manifest.is_null())
  {
    SDE_LOG_DEBUG("SceneError::kFailedToLoad");
    return make_unexpected(SceneError::kFailedToLoad);
  }

  const auto scripts_path = path / "scripts";

  // Read script data
  if (!loadWithManifest(script_manifest, scripts_path, scripts_, assets_))
  {
    SDE_LOG_DEBUG("SceneError::kFailedToLoad");
    return make_unexpected(SceneError::kFailedToLoad);
  }
  return {};
}

void Scene::addScript(std::string name, ScriptRuntime::UPtr script)
{
  scripts_.emplace_back(std::move(name), std::move(script));
}

void Scene::removeScript(const std::string& name)
{
  const auto remove_script_itr =
    std::remove_if(std::begin(scripts_), std::end(scripts_), [&name](const ScriptData& script) -> bool {
      return script.name == name;
    });
  scripts_.erase(remove_script_itr, std::end(scripts_));
}

bool Scene::tick(AppState& state, const AppProperties& properties)
{
  for (auto& [name, script] : scripts_)
  {
    if (!script->update(assets_, state, properties))
    {
      return false;
    }
  }
  return true;
}

}  // namespace sde::game
