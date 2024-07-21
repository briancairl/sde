// C++ Standard Library
#include <unordered_map>

/// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/script_runtime.hpp"
#include "sde/logging.hpp"

namespace sde::game
{
namespace
{

std::unordered_map<std::string, std::function<ScriptRuntimeUPtr(const nlohmann::json&)>> kLoaders;

}  // namespace

void ScriptRuntimeLoader::add(
  const std::string& script_name,
  std::function<ScriptRuntimeUPtr(const nlohmann::json&)> script_loader)
{
  const auto itr_and_added = kLoaders.emplace(script_name, std::move(script_loader));
  SDE_ASSERT_TRUE(std::get<1>(itr_and_added));
}

ScriptRuntimeUPtr
ScriptRuntimeLoader::load(const std::string& script_name, const nlohmann::json& script_loader_manifest)
{
  const auto itr = kLoaders.find(script_name);
  SDE_ASSERT_NE(itr, kLoaders.end());
  return itr->second(script_loader_manifest);
}

}  // namespace sde::game
