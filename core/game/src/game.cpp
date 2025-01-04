// C++ Standard Library
#include <ostream>

// SDE
#include "sde/app.hpp"
#include "sde/game/archive.hpp"
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


namespace sde::game
{
namespace
{

[[nodiscard]] bool save_game_data(GameResources& resources, const asset::path& path)
{
  SDE_LOG_INFO() << "Saving to: " << SDE_OSNV(path);
  auto ofs_or_error = serial::file_ostream::create(path);
  if (!ofs_or_error.has_value())
  {
    SDE_LOG_ERROR() << ofs_or_error.error() << " " << SDE_OSNV(path);
    return false;
  }

  // Wrap file stream in archive interface
  auto oar = serial::binary_ofarchive{*ofs_or_error};

  // Save all game resources
  oar << serial::named{"resources", resources};

  // Create associative wrapper for archive
  auto oar_assoc_or_error = serial::make_associative(oar);
  if (!oar_assoc_or_error.has_value())
  {
    SDE_LOG_ERROR() << SDE_OSNV(path) << " " << oar_assoc_or_error.error();
    return false;
  }

  // Save all entity data
  if (auto ok_or_error = resources.get<EntityCache>().save(resources.all(), *oar_assoc_or_error);
      !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "Failed to save entity data: " << ok_or_error.error();
    return false;
  }
  return true;
}


[[nodiscard]] bool load_game_data(GameResources& resources, const asset::path& path)
{
  SDE_LOG_INFO() << "Loading from: " << SDE_OSNV(path);
  auto ifs_or_error = serial::file_istream::create(path);
  if (!ifs_or_error.has_value())
  {
    SDE_LOG_ERROR() << ifs_or_error.error() << " " << SDE_OSNV(path);
    return ifs_or_error.error() == serial::file_stream_error::kFileDoesNotExist;
  }

  // Wrap file stream in archive interface
  auto iar = serial::binary_ifarchive{*ifs_or_error};

  // Load all game resources
  iar >> serial::named{"resources", resources};

  // Reload resources
  if (auto ok_or_error = resources.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "Failed to refresh resources: " << ok_or_error.error();
    return false;
  }

  // Create associative wrapper for archive
  auto iar_assoc_or_error = serial::make_associative(iar);
  if (!iar_assoc_or_error.has_value())
  {
    SDE_LOG_ERROR() << SDE_OSNV(path) << " " << iar_assoc_or_error.error();
    return false;
  }

  // Load all entity data
  if (auto ok_or_error = resources.get<EntityCache>().load(resources.all(), *iar_assoc_or_error);
      !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "Failed to load entity data: " << ok_or_error.error();
    return false;
  }
  return true;
}

}  // namespace

Game::Game(GameConfiguration&& configuration, GameResources&& resources, SceneHandle&& root) :
    configuration_{std::move(configuration)}, resources_{std::move(resources)}, root_{std::move(root)}
{}

void Game::spin(App& app)
{
  // Set initial window icon
  if (auto image_or_error = resources_.get<graphics::ImageCache>().find_or_create(
        configuration_.window_icon_path, resources_.all(), configuration_.window_icon_path);
      image_or_error.has_value())
  {
    app.window().setWindowIcon(image_or_error->value->ref());
    SDE_LOG_INFO() << "Set window icon: " << SDE_OSNV(configuration_.window_icon_path);
  }
  else
  {
    SDE_LOG_ERROR() << "Set window icon: " << SDE_OSNV(configuration_.window_icon_path)
                    << " failed with : " << image_or_error.error();
  }

  Scene root_scene;
  if (auto scene_or_error = resources_.get<SceneCache>().expand(root_, resources_.all()); scene_or_error)
  {
    root_scene = std::move(scene_or_error).value();
  }
  else
  {
    SDE_LOG_ERROR() << SDE_OSNV(scene_or_error.error());
    return;
  }

  // Run the game, starting from root scene
  app.spin(
    [this, &root_scene](const auto& app_properties) {
      if (auto ok_or_error = root_scene.load(configuration_.script_directory); !ok_or_error)
      {
        SDE_LOG_WARN() << "Failed loading: " << ok_or_error.error();
      }

      if (auto ok_or_error = root_scene.initialize(resources_, app_properties); ok_or_error)
      {
        return AppDirective::kContinue;
      }
      else
      {
        SDE_LOG_ERROR() << SDE_OSNV(ok_or_error.error());
        return AppDirective::kClose;
      }
    },
    [this, &root_scene](const auto& app_properties) {
      if (auto ok_or_error = root_scene.update(resources_, app_properties); ok_or_error)
      {
        return AppDirective::kContinue;
      }
      else
      {
        return AppDirective::kClose;
      }
    },
    [this, &root_scene](const auto& app_properties) {
      if (auto ok_or_error = root_scene.save(configuration_.script_directory); !ok_or_error)
      {
        SDE_LOG_ERROR() << "Failed saving: " << ok_or_error.error();
      }
      if (auto ok_or_error = root_scene.shutdown(resources_, app_properties); !ok_or_error)
      {
        SDE_LOG_ERROR() << "Failed shutting down: " << ok_or_error.error();
      }
      SDE_ASSERT_TRUE(save_game_data(resources_, configuration_.assets_data_path)) << " failed to save game";
    },
    configuration_.rate);
}

Game Game::create(const asset::path& path)
{
  // Load base configuration
  auto configuration{GameConfiguration::load(path)};

  // Create game resource caches
  GameResources resources{configuration.working_directory};

  SDE_ASSERT_TRUE(load_game_data(resources, configuration.assets_data_path))
    << "Failed to load game: " << SDE_OSNV(path);

  // Resolve path to game manifest
  auto root = load_manifest(resources, configuration);

  // Return game object
  return {std::move(configuration), std::move(resources), std::move(root)};
}

}  // namespace sde::game
