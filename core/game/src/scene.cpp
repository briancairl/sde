// C++ Standard Library
#include <algorithm>
#include <ostream>

/// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/scene.hpp"
#include "sde/game/script_runtime.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

ScriptData::ScriptData(std::string&& _name, ScriptRuntime::UPtr _script) :
    name{std::move(_name)}, script{std::move(_script)}
{}

void Scene::save(const asset::path& path) const {}

void Scene::load(const asset::path& path) {}

void Scene::addScript(std::string name, ScriptRuntime::UPtr script)
{
  scripts_.emplace_back(std::move(name), std::move(script));
}

void Scene::removeScript(const std::string& name)
{
  const auto remove_script_itr =
    std::remove_if(std::begin(scripts_), std::end(scripts_), [&name](const ScriptData& script) -> bool {
      return script.name == name;
    });
  scripts_.erase(remove_script_itr, std::end(scripts_));
}

bool Scene::tick(AppState& state, const AppProperties& properties)
{
  for (auto& [name, script] : scripts_)
  {
    if (!script->update(assets_, state, properties))
    {
      return false;
    }
  }
  return true;
}

}  // namespace sde::game
