// C++ Standard Library
#include <fstream>
#include <ostream>

// JSON
#include <nlohmann/json.hpp>

// SDE
#include "sde/app.hpp"
#include "sde/game/game.hpp"
#include "sde/game/game_resources.hpp"
#include "sde/geometry_io.hpp"
#include "sde/logging.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serial/std/optional.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization_binary_file.hpp"


namespace sde::game
{
namespace
{

[[nodiscard]] bool save_entities(GameResources& resources, const asset::path& entity_data_path)
{
  SDE_LOG_INFO() << "Saving:" << SDE_OSNV(entity_data_path);
  auto ofs_or_error = serial::file_ostream::create(entity_data_path);
  if (!ofs_or_error.has_value())
  {
    SDE_LOG_ERROR() << ofs_or_error.error() << " " << SDE_OSNV(entity_data_path);
    return false;
  }

  OArchive oar{*ofs_or_error};
  if (auto ok_or_error = resources.get<EntityCache>().save(resources.all(), oar); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "Failed to save entity data: " << ok_or_error.error();
    return false;
  }
  return true;
}


[[nodiscard]] bool load_entities(GameResources& resources, const asset::path& entity_data_path)
{
  SDE_LOG_INFO() << "Loading:" << SDE_OSNV(entity_data_path);
  if (resources.get<EntityCache>().empty())
  {
    return true;
  }

  auto ifs_or_error = serial::file_istream::create(entity_data_path);
  if (!ifs_or_error.has_value())
  {
    SDE_LOG_ERROR() << ifs_or_error.error() << " " << SDE_OSNV(entity_data_path);
    return false;
  }

  IArchive iar{*ifs_or_error};
  if (auto ok_or_error = resources.get<EntityCache>().load(resources.all(), iar); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "Failed to load entity data: " << ok_or_error.error();
    return false;
  }
  return true;
}


[[nodiscard]] bool save_resources(const GameResources& resources, const asset::path& resources_path)
{
  SDE_LOG_INFO() << "Saving:" << SDE_OSNV(resources_path);
  if (auto ofs_or_error = serial::file_ostream::create(resources_path); ofs_or_error.has_value())
  {
    OArchive oar{*ofs_or_error};
    oar << Field{"resources", resources};
  }
  else
  {
    SDE_LOG_ERROR() << ofs_or_error.error() << " " << SDE_OSNV(resources_path);
    return false;
  }
  return true;
}


[[nodiscard]] bool load_resources(GameResources& resources, const asset::path& resources_path)
{
  SDE_LOG_INFO() << "Loading:" << SDE_OSNV(resources_path);
  if (auto ifs_or_error = serial::file_istream::create(resources_path); ifs_or_error.has_value())
  {
    IArchive iar{*ifs_or_error};
    iar >> Field{"resources", resources};
  }
  else if (ifs_or_error.error() == serial::FileStreamError::kFileDoesNotExist)
  {
    SDE_LOG_WARN() << SDE_OSNV(resources_path);
  }
  else
  {
    SDE_LOG_ERROR() << ifs_or_error.error() << " " << SDE_OSNV(resources_path);
    return false;
  }

  // Reload resources
  if (auto ok_or_error = resources.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "Failed to refresh resources: " << ok_or_error.error();
    return false;
  }
  return true;
}

[[nodiscard]] bool load_components(GameResources& resources, const nlohmann::json& components_json)
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

[[nodiscard]] bool load_scripts(GameResources& resources, const nlohmann::json& scripts_json)
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

[[nodiscard]] bool load_scenes(GameResources& resources, const nlohmann::json& scenes_json)
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
        const auto child_scene_handle = resources.get<SceneCache>().to_handle(child_scene_name);
        if (!child_scene_handle)
        {
          SDE_LOG_ERROR() << "Scene: " << scene_name << " : invalid child scene " << SDE_OSNV(child_scene_name);
          return false;
        }
        else
        {
          resources.get<SceneCache>().update_if_exists(
            scene_name, [child_scene_handle](auto& scene) { scene.nodes.push_back({.child = child_scene_handle}); });
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

}  // namespace

std::ostream& operator<<(std::ostream& os, GameError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(GameError::kScriptLoadError)
    SDE_OS_ENUM_CASE(GameError::kComponentLoadError)
    SDE_OS_ENUM_CASE(GameError::kResourceLoadError)
    SDE_OS_ENUM_CASE(GameError::kSceneLoadError)
    SDE_OS_ENUM_CASE(GameError::kSceneEntryPointInvalid)
    SDE_OS_ENUM_CASE(GameError::kResourceSaveError)
    SDE_OS_ENUM_CASE(GameError::kInvalidRootDirectory)
    SDE_OS_ENUM_CASE(GameError::kInvalidManifest)
    SDE_OS_ENUM_CASE(GameError::kMissingManifest)
    SDE_OS_ENUM_CASE(GameError::kEntityLoadError)
    SDE_OS_ENUM_CASE(GameError::kEntitySaveError)
  }
  return os;
}

bool Game::setActiveScene(SceneHandle next_scene, const AppProperties& app_properties)
{
  if (active_scene_ == next_scene)
  {
    return true;
  }

  // Get handle to script instances
  auto& scripts = resources_.get<NativeScriptInstanceCache>();

  // Save previous scene data
  for (const auto& node : active_scene_sequence_)
  {
    if (auto ok_or_error = scripts.save(node.handle, config_.script_data_path); ok_or_error.has_value())
    {
      SDE_LOG_DEBUG() << "Saved data for " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " to "
                      << SDE_OSNV(config_.script_data_path);
    }
    else
    {
      SDE_LOG_ERROR() << "Saving data for " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " to "
                      << SDE_OSNV(config_.script_data_path) << " failed with error: " << ok_or_error.error();
    }

    if (node.instance.shutdown(resources_, app_properties))
    {
      SDE_LOG_DEBUG() << "Shutdown " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name);
    }
    else
    {
      SDE_LOG_ERROR() << "Shutdown " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " failed";
    }
  }

  // Change scene sequence
  if (auto sequence_or_error = resources_.get<SceneCache>().expand(next_scene, resources_.all());
      sequence_or_error.has_value())
  {
    SDE_LOG_DEBUG() << "Scene change from: " << SDE_OSNV(active_scene_) << " --> " << SDE_OSNV(next_scene);
    active_scene_ = next_scene;
    active_scene_sequence_ = std::move(sequence_or_error).value();
  }
  else
  {
    SDE_LOG_ERROR() << "Scene change from: " << SDE_OSNV(active_scene_) << " --> " << SDE_OSNV(next_scene)
                    << " failed with error: " << sequence_or_error.error();
    return false;
  }

  // Load current scene data
  for (const auto& node : active_scene_sequence_)
  {
    if (auto ok_or_error = scripts.load(node.handle, config_.script_data_path); ok_or_error.has_value())
    {
      SDE_LOG_DEBUG() << "Loaded data for " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " from "
                      << SDE_OSNV(config_.script_data_path);
    }
    else if (ok_or_error.error() == NativeScriptInstanceError::kInstanceDataUnavailable)
    {
      SDE_LOG_WARN() << "No previous save for " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " in "
                     << SDE_OSNV(config_.script_data_path);
    }
    else
    {
      SDE_LOG_ERROR() << "Loading data for " << SDE_OSNV(node.handle) << " : " << SDE_OSNV(node.name) << " from "
                      << SDE_OSNV(config_.script_data_path) << " failed with error: " << ok_or_error.error();
    }
  }

  return true;
}

void Game::spin(App& app)
{
  // Set initial window icon
  if (auto image_or_error = resources_.get<graphics::ImageCache>().find_or_create(
        config_.window_icon_path, resources_.all(), config_.window_icon_path);
      image_or_error.has_value())
  {
    app.window().setWindowIcon(image_or_error->value->ref());
    SDE_LOG_INFO() << "Set window icon: " << SDE_OSNV(config_.window_icon_path);
  }
  else
  {
    SDE_LOG_ERROR() << "Set window icon: " << SDE_OSNV(config_.window_icon_path)
                    << " failed with : " << image_or_error.error();
  }

  // Set initial cursor icon
  if (auto image_or_error = resources_.get<graphics::ImageCache>().find_or_create(
        config_.cursor_icon_path, resources_.all(), config_.cursor_icon_path);
      image_or_error.has_value())
  {
    app.window().setCursorIcon(image_or_error->value->ref());
    SDE_LOG_INFO() << "Set cursor icon: " << SDE_OSNV(config_.cursor_icon_path);
  }
  else
  {
    SDE_LOG_ERROR() << "Set cursor icon: " << SDE_OSNV(config_.cursor_icon_path)
                    << " failed with : " << image_or_error.error();
  }

  // Run the game, starting from root scene
  app.spin(
    [this](const auto& app_properties) {
      if (!setActiveScene(resources_.getNextScene(), app_properties))
      {
        return AppDirective::kClose;
      }

      for (const auto& [script_name, script_handle, script_instance] : active_scene_sequence_)
      {
        if (!script_instance.initialize(script_handle, script_name, resources_, app_properties))
        {
          SDE_LOG_ERROR() << SDE_OSNV(script_name) << SDE_OSNV(script_handle) << " failed to initialize";
          return AppDirective::kClose;
        }
      }

      return AppDirective::kContinue;
    },
    [this](const auto& app_properties) {
      for (const auto& [script_name, script_handle, script_instance] : active_scene_sequence_)
      {
        if (!script_instance.update(resources_, app_properties))
        {
          SDE_LOG_ERROR() << SDE_OSNV(script_name) << SDE_OSNV(script_handle) << " failed to update";
          return AppDirective::kClose;
        }
      }
      return (active_scene_ == resources_.getNextScene()) ? AppDirective::kContinue : AppDirective::kReset;
    },
    [this](const auto& app_properties) {
      if (!setActiveScene(SceneHandle::null(), app_properties))
      {
        SDE_LOG_WARN() << "Some scene data may have been lost!";
      }
    },
    config_.rate);
}


expected<Game, GameError> Game::create(const asset::path& path)
{
  // Ensure game directory exists
  if (!asset::exists(path) or !asset::is_directory(path))
  {
    SDE_LOG_ERROR() << "Invalid root game directory: " << SDE_OSNV(path);
    return make_unexpected(GameError::kInvalidRootDirectory);
  }

  // Create game resource caches
  GameResources resources{path};

  // Resolve path to game manifest
  const auto manifest_path = resources.path("manifest.json");
  if (!asset::exists(manifest_path))
  {
    SDE_LOG_ERROR() << "Missing game manifest: " << SDE_OSNV(manifest_path);
    return make_unexpected(GameError::kMissingManifest);
  }
  else if (!asset::is_regular_file(manifest_path))
  {
    SDE_LOG_ERROR() << "Invalid game manifest: " << SDE_OSNV(manifest_path) << " (must be regular file)";
    return make_unexpected(GameError::kInvalidManifest);
  }

  // Load scene manifest (JSON)
  nlohmann::json manifest_json;
  SDE_ASSERT_NO_EXCEPT(
    {
      std::ifstream ifs{manifest_path};
      ifs >> manifest_json;
    },
    std::exception);

  // Load basic game configutation
  GameConfig config;
  SDE_ASSERT_NO_EXCEPT(
    {
      auto config_json = manifest_json["config"];
      config.rate = Rate::fromHertz(static_cast<float>(config_json["rate"]));
      config.assets_data_path = resources.path(asset::path{config_json["assets_data_path"]});
      config.entity_data_path = resources.path(asset::path{config_json["entity_data_path"]});
      config.script_data_path = resources.path(asset::path{config_json["script_data_path"]});
      config.window_icon_path = resources.path(asset::path{config_json["window_icon_path"]});
      config.cursor_icon_path = resources.path(asset::path{config_json["cursor_icon_path"]});
    },
    std::exception);

  // Load game assets
  SDE_ASSERT_NO_EXCEPT(
    {
      if (!load_resources(resources, config.assets_data_path))
      {
        SDE_LOG_ERROR() << "failed to load resources";
        return make_unexpected(GameError::kResourceLoadError);
      }
    },
    std::exception);

  // Load components from manifest
  SDE_LOG_INFO() << "Loading components from manifest...";
  SDE_ASSERT_NO_EXCEPT(
    {
      if (!load_components(resources, manifest_json["components"]))
      {
        SDE_LOG_ERROR() << "failed to load components";
        return make_unexpected(GameError::kComponentLoadError);
      }
    },
    std::exception);

  // Load scripts from manifest
  SDE_LOG_INFO() << "Loading scripts from manifest...";
  SDE_ASSERT_NO_EXCEPT(
    {
      if (!load_scripts(resources, manifest_json["scripts"]))
      {
        SDE_LOG_ERROR() << "failed to load scripts";
        return make_unexpected(GameError::kScriptLoadError);
      }
    },
    std::exception);

  // Load scenes from manifest
  SDE_LOG_INFO() << "Loading scenes from manifest...";
  SDE_ASSERT_NO_EXCEPT(
    {
      if (!load_scenes(resources, manifest_json["scenes"]))
      {
        SDE_LOG_ERROR() << "failed to load scenes";
        return make_unexpected(GameError::kSceneLoadError);
      }
    },
    std::exception);

  // Load game entities
  SDE_LOG_INFO() << "Loading components from manifest...";
  SDE_ASSERT_NO_EXCEPT(
    {
      if (!load_entities(resources, config.entity_data_path))
      {
        SDE_LOG_ERROR() << "failed to load components";
        return make_unexpected(GameError::kEntityLoadError);
      }
    },
    std::exception);

  // Set entry point scene
  SDE_ASSERT_NO_EXCEPT(
    {
      if (!resources.setNextScene(manifest_json["entry"]))
      {
        SDE_LOG_ERROR() << "failed to set scene entry point";
        return make_unexpected(GameError::kSceneEntryPointInvalid);
      }
    },
    std::exception);

  // Return game object
  Game game;
  {
    game.config_ = std::move(config);
    game.resources_ = std::move(resources);
    game.active_scene_ = SceneHandle::null();
    game.active_scene_sequence_.clear();
  }
  return {std::move(game)};
}

expected<void, GameError> Game::dump()
{
  // Resolve directory for script data
  const auto scripts_directory = resources_.directory(config_.script_data_path);

  // Save game entities
  SDE_LOG_INFO() << "Saving entity data...";
  SDE_ASSERT_NO_EXCEPT(
    {
      if (!save_entities(resources_, config_.entity_data_path))
      {
        SDE_LOG_ERROR() << "failed to load components";
        return make_unexpected(GameError::kEntityLoadError);
      }
    },
    std::exception);

  // Save resources
  SDE_LOG_INFO() << "Saving resource data...";
  SDE_ASSERT_NO_EXCEPT(
    {
      if (!save_resources(resources_, config_.assets_data_path))
      {
        SDE_LOG_ERROR() << "failed to save resources";
        return make_unexpected(GameError::kResourceSaveError);
      }
    },
    std::exception);

  return {};
}


}  // namespace sde::game
