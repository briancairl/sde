/**
 * @copyright 2024-present Brian Cairl
 *
 * @file script_runtime.hpp
 */
#pragma once

// C++ Standard Library
#include <memory>
#include <string>

// EnTT
#include <entt/fwd.hpp>

// SDE
#include "sde/app_properties.hpp"
#include "sde/game/script.hpp"
#include "sde/game/script_fwd.hpp"
#include "sde/serialization_binary_file_fwd.hpp"

namespace sde::game
{

class ScriptRuntime : public Script<ScriptRuntime>
{
public:
  using UPtr = std::unique_ptr<ScriptRuntime>;
  using OArchive = serial::binary_ofarchive;
  using IArchive = serial::binary_ifarchive;

  virtual ~ScriptRuntime() = default;

  virtual bool onLoad(IArchive& ar, SharedAssets& assets) = 0;

  virtual bool onSave(OArchive& ar, SharedAssets& assets) = 0;

  virtual bool onInitialize(Systems& systems, Assets& assets, const AppProperties& app) = 0;

  virtual expected<void, ScriptError> onUpdate(Systems& systems, Assets& assets, const AppProperties& app) = 0;

  const std::string& getIdentity() const { return script_name_; }

protected:
  ScriptRuntime(std::string script_name) : script_name_{std::move(script_name)} {}

private:
  std::string script_name_;
};

}  // namespace sde::game
