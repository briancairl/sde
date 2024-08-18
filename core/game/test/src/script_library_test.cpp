#include "sde/game/native_script_runtime.hpp"

struct data
{};

bool load(data* self, sde::game::IArchive& ar) { return true; }

bool save(data* self, sde::game::OArchive& ar) { return true; }

bool initialize(data* self, sde::game::Assets& assets, sde::AppState& app_state, const sde::AppProperties& app_props)
{
  return true;
}

bool update(data* self, sde::game::Assets& assets, sde::AppState& app_state, const sde::AppProperties& app_props)
{
  return true;
}


SDE_NATIVE_SCRIPT__INSTANCE(data);
