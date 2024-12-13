/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_data.hpp"
#include "sde/resource_collection.hpp"

namespace sde::audio
{

enum class AssetError
{
  kFailedSoundDataLoading,
  kFailedSoundLoading,
};

std::ostream& operator<<(std::ostream& os, AssetError error);

struct Assets : ResourceCollection<
                  ResourceCollectionEntry<decltype("sound_data"_label), SoundDataCache>,
                  ResourceCollectionEntry<decltype("sounds"_label), SoundCache>>
{
  Assets() = default;

  [[nodiscard]] expected<void, AssetError> refresh();

  void strip();
};

}  // namespace sde::audio
