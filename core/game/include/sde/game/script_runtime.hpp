/**
 * @copyright 2024-present Brian Cairl
 *
 * @file script_runtime.hpp
 */
#pragma once

// C++ Standard Library
#include <functional>
#include <string>

// EnTT
#include <entt/fwd.hpp>

// JSON
#include <nlohmann/json_fwd.hpp>

// SDE
#include "sde/app_properties.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/script.hpp"
#include "sde/game/script_fwd.hpp"
#include "sde/game/script_runtime_fwd.hpp"

namespace sde::game
{

class ScriptRuntime : public Script<ScriptRuntime>
{
public:
  using UPtr = ScriptRuntimeUPtr;

  virtual ~ScriptRuntime() = default;

  virtual bool onLoad(IArchive& ar) { return true; }

  virtual bool onSave(OArchive& ar) const { return true; }

  virtual bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app_props) { return true; }

  virtual expected<void, ScriptError>
  onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app_props) = 0;

  const std::string& type() const { return script_type_name_; }

protected:
  explicit ScriptRuntime(std::string script_type_name) : script_type_name_{std::move(script_type_name)} {}
  std::string script_type_name_;
};

struct ScriptRuntimeLoader
{
  static void
  add(const std::string& script_name, std::function<ScriptRuntimeUPtr(const nlohmann::json&)> script_loader);
  static ScriptRuntimeUPtr load(const std::string& script_name, const nlohmann::json& script_loader_manifest);
};

}  // namespace sde::game
