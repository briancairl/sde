// C++ Standard Library
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/script.hpp"
#include "sde/graphics/tile_set_handle.hpp"

using namespace sde;

struct Info
{
  std::string name;
};

struct Size
{
  Vec2f extent;
};

struct State
{
  Vec2f position;
  Vec2f velocity;
  Vec2f looking;
};

struct Direction
{
  Vec2f looking;
};

class PlayerCharacter : public sde::game::Script<PlayerCharacter>
{
  friend script;

private:
  bool onInitialize(entt::registry& registry, sde::game::Assets& assets);

  expected<void, sde::game::ScriptError>
  onUpdate(entt::registry& registry, const sde::game::Assets& assets, const AppProperties& app);

  entt::entity id_;
  sde::graphics::TileSetHandle idle_[8UL];
  sde::graphics::TileSetHandle walk_[8UL];
  sde::graphics::TileSetHandle run_[8UL];
};
