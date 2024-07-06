// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/assets.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::game
{

Assets::Assets() {}

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
  return {};
}

}  // namespace sde::game
