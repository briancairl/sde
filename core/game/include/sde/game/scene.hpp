/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// C++ Standard Library
#include <string>
#include <vector>

// SDE
#include "sde/app_fwd.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/script_runtime_fwd.hpp"
#include "sde/game/systems_fwd.hpp"
#include "sde/resource.hpp"

namespace sde::game
{

/**
 * @brief Loaded game script data
 */
struct ScriptData : public Resource<ScriptData>
{
  std::string name;
  ScriptRuntimeUPtr script;

  ScriptData(std::string&& _name, ScriptRuntimeUPtr _script);

  auto field_list() { return FieldList(Field{"name", name}, _Stub{"script", script}); }
};

/**
 * @brief All active game data
 */
class Scene : public Resource<Scene>
{
  friend fundemental_type;

public:
  void save(const asset::path& path) const;

  void load(const asset::path& path);

  void addScript(std::string name, ScriptRuntimeUPtr script);

  void removeScript(const std::string& name);

  bool tick(AppState& state, const AppProperties& properties);

private:
  Assets assets_;

  std::vector<ScriptData> scripts_;

  auto field_list() { return FieldList(Field{"assets", assets_}, _Stub{"scripts", scripts_}); }
};

}  // namespace sde::game
