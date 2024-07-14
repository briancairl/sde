// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/assets.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

Assets::Assets([[maybe_unused]] Systems& systems) : entities{registry} {}

expected<void, AssetError> Assets::refresh()
{
  if (auto ok_or_error = graphics.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedGraphicsLoading");
    return make_unexpected(AssetError::kFailedGraphicsLoading);
  }
  if (auto ok_or_error = audio.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedAudioLoading");
    return make_unexpected(AssetError::kFailedAudioLoading);
  }
  if (auto ok_or_error = entities.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedEntitiesLoading");
    return make_unexpected(AssetError::kFailedEntitiesLoading);
  }
  return {};
}

}  // namespace sde::game
