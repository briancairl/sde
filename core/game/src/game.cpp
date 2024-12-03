// C++ Standard Library
#include <ostream>

// SDE
#include "sde/app.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/game.hpp"
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

[[nodiscard]] bool load(Assets& assets, const asset::path& assets_path)
{
  if (auto ifs_or_error = serial::file_istream::create(assets_path); ifs_or_error.has_value())
  {
    IArchive iar{*ifs_or_error};
    iar >> Field{"assets", assets};
    if (auto ok_or_error = assets.refresh(); !ok_or_error.has_value())
    {
      SDE_LOG_ERROR() << ok_or_error.error() << " " << SDE_OSNV(assets_path);
      return false;
    }
  }
  else if (ifs_or_error.error() == serial::FileStreamError::kFileDoesNotExist)
  {
    SDE_LOG_WARN() << SDE_OSNV(assets_path);
  }
  else
  {
    SDE_LOG_ERROR() << ifs_or_error.error() << " " << SDE_OSNV(assets_path);
    return false;
  }
  return true;
}

[[nodiscard]] bool save(const Assets& assets, const asset::path& assets_path)
{
  if (auto ofs_or_error = serial::file_ostream::create(assets_path); ofs_or_error.has_value())
  {
    OArchive oar{*ofs_or_error};
    oar << Field{"assets", assets};
  }
  else
  {
    SDE_LOG_ERROR() << ofs_or_error.error() << " " << SDE_OSNV(assets_path);
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

Game::Game(Assets&& assets, SceneGraph&& scene_graph) : assets_{std::move(assets)}, scene_graph_{std::move(scene_graph)}
{}

void Game::spin(App& app)
{
  app.spin(
    [this](const auto& app_properties) {
      if (const auto ok_or_error = scene_graph_.initialize(assets_, app_properties); !ok_or_error.has_value())
      {
        SDE_LOG_ERROR() << ok_or_error.error();
        return AppDirective::kClose;
      }
      return AppDirective::kContinue;
    },
    [this](const auto& app_properties) {
      if (const auto ok_or_error = scene_graph_.tick(assets_, app_properties); !ok_or_error.has_value())
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

  Assets assets;

  if (!load(assets, path / "assets.bin"))
  {
    return make_unexpected(GameError::kAssetLoadError);
  }

  auto scene_manifest_or_error = SceneManifest::create(path / "scene_graph.json");
  if (!scene_manifest_or_error.has_value())
  {
    SDE_LOG_ERROR() << scene_manifest_or_error.error();
    return make_unexpected(GameError::kSceneGraphLoadError);
  }

  auto scene_graph_or_error = SceneGraph::create(assets, *scene_manifest_or_error);
  if (!scene_graph_or_error.has_value())
  {
    SDE_LOG_ERROR() << scene_graph_or_error.error();
    return make_unexpected(GameError::kSceneGraphLoadError);
  }

  if (const auto script_data_path = path / "script_data"; !asset::exists(script_data_path))
  {
    SDE_LOG_WARN() << "No script data: " << SDE_OSNV(script_data_path);
  }
  else if (const auto ok_or_error = scene_graph_or_error->load(assets, script_data_path))
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(GameError::kSceneGraphLoadError);
  }

  return {Game{std::move(assets), std::move(scene_graph_or_error).value()}};
}

expected<void, GameError> Game::dump(Game& game, const asset::path& path)
{
  if (!asset::exists(path) and !asset::create_directories(path))
  {
    SDE_LOG_ERROR() << "Invalid script data directory: " << SDE_OSNV(path);
    return make_unexpected(GameError::kInvalidRootDirectory);
  }

  if (!save(game.assets_, path / "assets.bin"))
  {
    return make_unexpected(GameError::kAssetSaveError);
  }

  auto scene_manifest_or_error = game.scene_graph_.manifest(game.assets_);
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
  else if (const auto ok_or_error = game.scene_graph_.save(game.assets_, script_data_path))
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
