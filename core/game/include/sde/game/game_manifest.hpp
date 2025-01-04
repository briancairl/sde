/**
 * @copyright 2025-present Brian Cairl
 *
 * @file game_manifest.hpp
 */
#pragma once

// SDE
#include "sde/asset.hpp"
#include "sde/game/game_resources_fwd.hpp"
#include "sde/game/scene_fwd.hpp"
#include "sde/resource.hpp"
#include "sde/time.hpp"

namespace sde::game
{

struct GameConfiguration : Resource<GameConfiguration>
{
  Rate rate = {};
  asset::path working_directory = {};
  asset::path assets_data_path = {};
  asset::path script_directory = {};
  asset::path window_icon_path = {};
  asset::path manifest_path = {};
  asset::path config_path = {};

  static GameConfiguration load(const asset::path& config_path);

  auto field_list()
  {
    return FieldList(
      Field{"rate", rate},
      Field{"working_directory", working_directory},
      Field{"assets_data_path", assets_data_path},
      Field{"script_directory", script_directory},
      Field{"window_icon_path", window_icon_path},
      Field{"manifest_path", manifest_path},
      Field{"config_path", config_path});
  }
};

SceneHandle load_manifest(GameResources& resources, const GameConfiguration& config);

}  // namespace sde::game
