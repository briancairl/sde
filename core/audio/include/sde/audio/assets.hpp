/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// SDE
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_data.hpp"

namespace sde::audio
{

enum class AssetError
{
  kFailedSoundDataLoading,
  kFailedSoundLoading,
};

struct Assets
{
  /// Sound data cache
  SoundDataCache sound_data;
  /// Sound cache
  SoundCache sounds;

  Assets();

  [[nodiscard]] expected<void, AssetError> refresh();
};

}  // namespace sde::audio
