#define SDE_SCRIPT_NAME "physics"

// SDE
#include "sde/game/native_script_runtime.hpp"

// RED
#include "red/components.hpp"


using namespace sde;
using namespace sde::game;
using namespace sde::graphics;

struct physics
{};

bool load(physics* self, sde::game::IArchive& ar)
{
  using namespace sde::serial;
  return true;
}

bool save(physics* self, sde::game::OArchive& ar)
{
  using namespace sde::serial;
  return true;
}

bool initialize(physics* self, sde::game::GameResources& resources, const sde::AppProperties& app) { return true; }

bool update(physics* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  auto& registry = resources.get<Registry>();
  registry.view<Position, Dynamics>().each(
    [dt = toSeconds(app.time_delta)](Position& pos, const Dynamics& state) { pos.center += state.velocity * dt; });
  return true;
}

SDE_NATIVE_SCRIPT__REGISTER_AUTO(physics);