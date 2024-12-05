// C++ Standard Library
#include <ostream>

// SDE
#include "sde/audio/assets.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::audio
{

std::ostream& operator<<(std::ostream& os, AssetError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(AssetError::kFailedSoundDataLoading)
    SDE_OS_ENUM_CASE(AssetError::kFailedSoundLoading)
  }
  return os;
}

expected<void, AssetError> Assets::refresh()
{
  if (auto ok_or_error = sound_data.refresh(); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(AssetError::kFailedSoundDataLoading);
  }
  if (auto ok_or_error = sounds.refresh(ResourceDependencies{sound_data}); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(AssetError::kFailedSoundLoading);
  }
  return {};
}

void Assets::strip() { SDE_ASSERT_OK(sound_data.relinquish()); }

}  // namespace sde::audio
