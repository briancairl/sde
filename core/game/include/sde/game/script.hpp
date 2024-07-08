/**
 * @copyright 2024-present Brian Cairl
 *
 * @file script.hpp
 */
#pragma once

// C++ Standard Library
#include <optional>
#include <string_view>

// EnTT
#include <entt/fwd.hpp>

// SDE
#include "sde/app_properties.hpp"
#include "sde/crtp.hpp"
#include "sde/expected.hpp"
#include "sde/game/assets_fwd.hpp"
#include "sde/game/systems_fwd.hpp"
#include "sde/time.hpp"

namespace sde::game
{

enum class ScriptError
{
  kInitializationFailed,
  kCriticalUpdateFailure,
  kNonCriticalUpdateFailure,
  kNotLoaded,
  kLoadedPreviously,
  kLoadFailed,
  kSaveFailed,
};

template <typename ScriptT> struct Script : public crtp_base<Script<ScriptT>>
{
public:
  using SharedAssets = game::Assets;

  Script(Script&& other) = default;
  Script& operator=(Script&& other) = default;

  std::string_view identity() const { return this->derived().getIdentity(); }

  void reset() { t_init_.reset(); }

  template <typename Archive> expected<void, ScriptError> load(Archive& ar)
  {
    if (not_loaded_ and this->derived().onLoad(ar))
    {
      not_loaded_ = false;
      return {};
    }
    return make_unexpected(ScriptError::kLoadFailed);
  }

  template <typename Archive> expected<void, ScriptError> save(Archive& ar)
  {
    if (this->derived().onSave(ar))
    {
      return {};
    }
    return make_unexpected(ScriptError::kSaveFailed);
  }

  expected<void, ScriptError>
  update(entt::registry& registry, Systems& systems, SharedAssets& assets, const AppProperties& app)
  {
    if (t_init_.has_value())
    {
      return this->derived().onUpdate(registry, systems, assets, app);
    }
    else if (this->derived().onInitialize(registry, systems, assets, app))
    {
      t_init_ = app.time;
      return {};
    }
    return make_unexpected(ScriptError::kInitializationFailed);
  }

protected:
  using script = Script<ScriptT>;


  Script() = default;
  Script(const Script& other) = delete;
  Script& operator=(const Script& other) = delete;

  const std::optional<TimeOffset>& getTimeInit() const { return t_init_; }

private:
  bool not_loaded_ = true;
  std::optional<TimeOffset> t_init_;
};

}  // namespace sde::game
