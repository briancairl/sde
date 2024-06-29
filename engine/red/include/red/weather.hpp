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

class Weather : public Script<Weather>
{
  friend script;

private:
  bool onInitialize(entt::registry& registry, Resources& resources, Assets& assets, const AppProperties& app);

  expected<void, ScriptError>
  onUpdate(entt::registry& registry, Resources& resources, const Assets& assets, const AppProperties& app);

  entt::entity id_;
  sde::graphics::TileSetHandle rain_;
};
