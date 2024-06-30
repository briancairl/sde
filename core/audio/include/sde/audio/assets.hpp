/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// SDE
#include "sde/audio/sound.hpp"

namespace sde::audio
{

struct Assets
{
  /// Sound cache
  SoundCacheWithAssets sounds;
};

}  // namespace sde::audio
