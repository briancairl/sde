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
#include "sde/asset.hpp"
#include "sde/game/scene_graph.hpp"

namespace sde::game
{

enum class GameError
{
  kInvalidRootDirectory,
  kComponentLoadError,
  kAssetLoadError,
  kAssetSaveError,
  kSceneGraphLoadError,
  kSceneGraphSaveError,
};

std::ostream& operator<<(std::ostream& os, GameError error);

class Game
{
public:
  void spin(App& app);

  Game(Game&& other) = default;
  Game& operator=(Game&& other) = default;

  Game(Assets&& assets, SceneGraph&& scene_graph);

  [[nodiscard]] static expected<Game, GameError> create(const asset::path& path);
  [[nodiscard]] static expected<void, GameError> dump(Game& game, const asset::path& path);

private:
  Game(const Game& other) = delete;
  Game& operator=(const Game& other) = delete;

  Assets assets_;
  SceneGraph scene_graph_;
};

[[nodiscard]] inline expected<Game, GameError> create(const asset::path& path) { return Game::create(path); }

[[nodiscard]] inline expected<void, GameError> dump(Game& game, const asset::path& path)
{
  return Game::dump(game, path);
}


}  // namespace sde::game
