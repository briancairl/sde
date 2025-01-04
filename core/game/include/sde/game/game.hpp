/**
 * @copyright 2024-present Brian Cairl
 *
 * @file game.hpp
 */
#pragma once

// SDE
#include "sde/app_fwd.hpp"
#include "sde/asset.hpp"
#include "sde/game/game_configuration.hpp"
#include "sde/game/game_resources.hpp"
#include "sde/game/scene_handle.hpp"

namespace sde::game
{

class Game
{
public:
  Game(Game&& other) = default;
  Game& operator=(Game&& other) = default;

  void spin(App& app);

  [[nodiscard]] static Game create(const asset::path& path);

private:
  Game(GameConfiguration&& configuration, GameResources&& resources, SceneHandle&& root);
  Game() = default;
  Game(const Game& other) = delete;
  Game& operator=(const Game& other) = delete;

  GameConfiguration configuration_;
  GameResources resources_;
  SceneHandle root_;
};

}  // namespace sde::game
