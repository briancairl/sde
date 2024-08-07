// C++ Standard Library
#include <ostream>

// SDE
#include "sde/audio/assets.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::audio
{

Assets::Assets() : sound_data{}, sounds{sound_data} {}

expected<void, AssetError> Assets::refresh()
{
  if (auto ok_or_error = sound_data.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedSoundDataLoading");
    return make_unexpected(AssetError::kFailedSoundDataLoading);
  }
  if (auto ok_or_error = sounds.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_DEBUG("FailedSoundLoading");
    return make_unexpected(AssetError::kFailedSoundLoading);
  }
  sound_data.relinquish();
  return {};
}

}  // namespace sde::audio
