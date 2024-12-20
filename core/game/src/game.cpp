// C++ Standard Library
#include <ostream>

// SDE
#include "sde/app.hpp"
#include "sde/game/component_preload.hpp"
#include "sde/game/game.hpp"
#include "sde/game/game_resources.hpp"
#include "sde/game/scene_graph.hpp"
#include "sde/game/scene_manifest.hpp"
#include "sde/geometry_io.hpp"
#include "sde/logging.hpp"
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

[[nodiscard]] bool load(GameResources& resources, const asset::path& resources_path)
{
  if (auto ifs_or_error = serial::file_istream::create(resources_path); ifs_or_error.has_value())
  {
    IArchive iar{*ifs_or_error};
    iar >> Field{"resources", resources};
    if (auto ok_or_error = resources.refresh(); !ok_or_error.has_value())
    {
      SDE_LOG_ERROR() << ok_or_error.error() << " " << SDE_OSNV(resources_path);
      return false;
    }
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
  SDE_LOG_INFO() << "GameResources loaded: " << SDE_OSNV(resources_path);
  return true;
}

[[nodiscard]] bool save(const GameResources& resources, const asset::path& resources_path)
{
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

}  // namespace

std::ostream& operator<<(std::ostream& os, GameError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(GameError::kComponentLoadError)
    SDE_OS_ENUM_CASE(GameError::kAssetLoadError)
    SDE_OS_ENUM_CASE(GameError::kAssetSaveError)
    SDE_OS_ENUM_CASE(GameError::kSceneGraphLoadError)
    SDE_OS_ENUM_CASE(GameError::kSceneGraphSaveError)
    SDE_OS_ENUM_CASE(GameError::kInvalidRootDirectory)
  }
  return os;
}

Game::Game(GameResources&& resources, SceneGraph&& scene_graph) :
    resources_{std::move(resources)}, scene_graph_{std::move(scene_graph)}
{}

void Game::spin(App& app)
{
  app.spin(
    [this](const auto& app_properties) {
      if (const auto ok_or_error = scene_graph_.initialize(resources_, app_properties); !ok_or_error.has_value())
      {
        SDE_LOG_ERROR() << ok_or_error.error();
        return AppDirective::kClose;
      }
      return AppDirective::kContinue;
    },
    [this](const auto& app_properties) {
      if (const auto ok_or_error = scene_graph_.tick(resources_, app_properties); !ok_or_error.has_value())
      {
        SDE_LOG_ERROR() << ok_or_error.error();
        return AppDirective::kClose;
      }
      return AppDirective::kContinue;
    });
}


expected<Game, GameError> Game::create(const asset::path& path)
{
  if (!asset::exists(path))
  {
    SDE_LOG_ERROR() << "Invalid script data directory: " << SDE_OSNV(path);
    return make_unexpected(GameError::kInvalidRootDirectory);
  }

  GameResources resources;

  if (!load(resources, path / "resources.bin"))
  {
    return make_unexpected(GameError::kAssetLoadError);
  }

  if (auto ok_or_error = componentPreload(resources, path / "components.json"); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(GameError::kComponentLoadError);
  }

  auto scene_manifest_or_error = SceneManifest::create(path / "scene_graph.json");
  if (!scene_manifest_or_error.has_value())
  {
    SDE_LOG_ERROR() << scene_manifest_or_error.error();
    return make_unexpected(GameError::kSceneGraphLoadError);
  }

  auto scene_graph_or_error = SceneGraph::create(resources, *scene_manifest_or_error);
  if (!scene_graph_or_error.has_value())
  {
    SDE_LOG_ERROR() << scene_graph_or_error.error();
    return make_unexpected(GameError::kSceneGraphLoadError);
  }

  if (const auto script_data_path = path / "script_data"; !asset::exists(script_data_path))
  {
    SDE_LOG_WARN() << "No script data: " << SDE_OSNV(script_data_path);
  }
  else if (const auto ok_or_error = scene_graph_or_error->load(resources, script_data_path); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(GameError::kSceneGraphLoadError);
  }

  return {Game{std::move(resources), std::move(scene_graph_or_error).value()}};
}

expected<void, GameError> Game::dump(Game& game, const asset::path& path)
{
  if (!asset::exists(path) and !asset::create_directories(path))
  {
    SDE_LOG_ERROR() << "Invalid script data directory: " << SDE_OSNV(path);
    return make_unexpected(GameError::kInvalidRootDirectory);
  }

  if (!save(game.resources_, path / "resources.bin"))
  {
    return make_unexpected(GameError::kAssetSaveError);
  }

  auto scene_manifest_or_error = game.scene_graph_.manifest(game.resources_);
  if (!scene_manifest_or_error.has_value())
  {
    SDE_LOG_ERROR() << scene_manifest_or_error.error();
    return make_unexpected(GameError::kSceneGraphSaveError);
  }

  if (const auto script_data_path = path / "script_data";
      !asset::exists(script_data_path) and !asset::create_directories(script_data_path))
  {
    SDE_LOG_ERROR() << "Invalid script data directory: " << SDE_OSNV(script_data_path);
    return make_unexpected(GameError::kSceneGraphSaveError);
  }
  else if (const auto ok_or_error = game.scene_graph_.save(game.resources_, script_data_path); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(GameError::kSceneGraphSaveError);
  }

  if (auto ok_or_error = scene_manifest_or_error->save(path / "scene_graph.json"); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(GameError::kSceneGraphSaveError);
  }

  return {};
}


}  // namespace sde::game
