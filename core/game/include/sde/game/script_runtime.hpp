/**
 * @copyright 2024-present Brian Cairl
 *
 * @file script_runtime.hpp
 */
#pragma once

// C++ Standard Library
#include <string>

// EnTT
#include <entt/fwd.hpp>

// SDE
#include "sde/app_properties.hpp"
#include "sde/game/script.hpp"
#include "sde/game/script_fwd.hpp"
#include "sde/game/script_runtime_fwd.hpp"
#include "sde/serialization_binary_file_fwd.hpp"

namespace sde::game
{

class ScriptRuntime : public Script<ScriptRuntime>
{
public:
  using UPtr = ScriptRuntimeUPtr;
  using OArchive = serial::binary_ofarchive;
  using IArchive = serial::binary_ifarchive;

  virtual ~ScriptRuntime() = default;

  virtual bool onLoad(IArchive& ar, SharedAssets& assets) = 0;

  virtual bool onSave(OArchive& ar, SharedAssets& assets) = 0;

  virtual bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app_props) = 0;

  virtual expected<void, ScriptError>
  onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app_props) = 0;

  const std::string& type() const { return script_type_name_; }

protected:
  explicit ScriptRuntime(std::string script_type_name) : script_type_name_{std::move(script_type_name)} {}
  std::string script_type_name_;
};

}  // namespace sde::game
