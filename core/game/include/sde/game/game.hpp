/**
 * @copyright 2024-present Brian Cairl
 *
 * @file game.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>

// SDE
#include "sde/app_fwd.hpp"
#include "sde/game/game_resources.hpp"
#include "sde/game/scene.hpp"
#include "sde/resource.hpp"
#include "sde/vector.hpp"

namespace sde::game
{

enum class GameError
{
  kInvalidRootDirectory,
  kInvalidManifest,
  kMissingManifest,
  kScriptLoadError,
  kComponentLoadError,
  kSceneLoadError,
  kSceneEntryPointInvalid,
  kResourceLoadError,
  kResourceSaveError,
  kEntityLoadError,
  kEntitySaveError,
};

std::ostream& operator<<(std::ostream& os, GameError error);

struct GameConfig : Resource<GameConfig>
{
  Rate rate = {};
  asset::path assets_data_path = {};
  asset::path entity_data_path = {};
  asset::path script_data_path = {};
  asset::path window_icon_path = {};
  asset::path cursor_icon_path = {};

  auto field_list()
  {
    return FieldList(
      Field{"rate", rate},
      Field{"assets_data_path", assets_data_path},
      Field{"entity_data_path", entity_data_path},
      Field{"script_data_path", script_data_path},
      Field{"window_icon_path", window_icon_path},
      Field{"cursor_icon_path", cursor_icon_path});
  }
};

class Game
{
public:
  Game(Game&& other) = default;
  Game& operator=(Game&& other) = default;

  void spin(App& app);

  [[nodiscard]] expected<void, GameError> dump();
  [[nodiscard]] static expected<Game, GameError> create(const asset::path& path);

private:
  Game() = default;
  Game(const Game& other) = delete;
  Game& operator=(const Game& other) = delete;

  bool setActiveScene(SceneHandle scene, const AppProperties& app_properties);

  GameResources resources_;

  GameConfig config_;

  SceneHandle active_scene_ = SceneHandle::null();

  sde::vector<SceneNodeFlattened> active_scene_sequence_ = {};
};

[[nodiscard]] inline expected<Game, GameError> create(const asset::path& path) { return Game::create(path); }

[[nodiscard]] inline expected<void, GameError> dump(Game& game) { return game.dump(); }


}  // namespace sde::game
