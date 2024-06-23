/**
 * @copyright 2024-present Brian Cairl
 *
 * @file script.hpp
 */
#pragma once

// C++ Standard Library
#include <optional>

// EnTT
#include <entt/fwd.hpp>

// SDE
#include "sde/app_properties.hpp"
#include "sde/crtp.hpp"
#include "sde/expected.hpp"
#include "sde/game/assets_fwd.hpp"
#include "sde/time.hpp"

namespace sde::game
{

enum class ScriptError
{
  kInitializationFailed,
  kCriticalUpdateFailure,
  kNonCriticalUpdateFailure,
};

template <typename ScriptT> struct Script : public crtp_base<Script<ScriptT>>
{
public:
  Script(Script&& other) = default;
  Script& operator=(Script&& other) = default;

  void reset() { t_start_.reset(); }

  expected<void, ScriptError> update(entt::registry& registry, Assets& assets, const AppProperties& app)
  {
    if (t_start_.has_value())
    {
      return this->derived().onUpdate(registry, assets, app);
    }
    else if (this->derived().onInitialize(registry, assets))
    {
      t_start_ = app.time;
      return {};
    }
    return make_unexpected(ScriptError::kInitializationFailed);
  }

protected:
  using script = Script<ScriptT>;

  Script() = default;
  Script(const Script& other) = delete;
  Script& operator=(const Script& other) = delete;

private:
  constexpr static bool onInitialize([[maybe_unused]] entt::registry& registry, [[maybe_unused]] Assets& assets)
  {
    return true;
  }

  constexpr static expected<void, ScriptError> onUpdate(
    [[maybe_unused]] entt::registry& registry,
    [[maybe_unused]] const Assets& assets,
    [[maybe_unused]] const AppProperties& app)
  {
    return {};
  }

  std::optional<TimeOffset> t_start_;
};

}  // namespace sde::game
