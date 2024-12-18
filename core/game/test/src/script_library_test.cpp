#include "sde/game/native_script_runtime.hpp"

struct custom_script
{};

bool load(custom_script* self, sde::game::IArchive& ar) { return true; }

bool save(custom_script* self, sde::game::OArchive& ar) { return true; }

bool initialize(custom_script* self, sde::game::GameResources& resources, const sde::AppProperties& app_props)
{
  return true;
}

bool update(custom_script* self, sde::game::GameResources& resources, const sde::AppProperties& app_props)
{
  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(custom_script);
