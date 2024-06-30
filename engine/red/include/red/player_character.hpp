#pragma once

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/script.hpp"
#include "sde/graphics/tile_set_handle.hpp"

// RED
#include "red/components.hpp"

using namespace sde;
using namespace sde::game;

class PlayerCharacter : public Script<PlayerCharacter>
{
  friend script;

private:
  bool onInitialize(entt::registry& registry, Systems& systems, Assets& assets, const AppProperties& app);

  expected<void, ScriptError>
  onUpdate(entt::registry& registry, Systems& systems, const Assets& assets, const AppProperties& app);

  entt::entity id_;
  sde::graphics::TileSetHandle idle_[8UL];
  sde::graphics::TileSetHandle walk_[8UL];
  sde::graphics::TileSetHandle run_[8UL];
};
