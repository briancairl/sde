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
#include "sde/resource.hpp"

namespace sde::audio
{

enum class AssetError
{
  kFailedSoundDataLoading,
  kFailedSoundLoading,
};

std::ostream& operator<<(std::ostream& os, AssetError error);

struct Assets : Resource<Assets>
{
  /// Sound data cache
  SoundDataCache sound_data;

  /// Sound cache
  SoundCache sounds;

  Assets() = default;

  Assets(Assets&&) = default;
  Assets& operator=(Assets&&) = default;

  Assets(const Assets&) = delete;
  Assets& operator=(const Assets&) = delete;

  [[nodiscard]] expected<void, AssetError> refresh();

  void strip();

  auto field_list() { return FieldList(Field{"sound_data", sound_data}, Field{"sounds", sounds}); }
};

}  // namespace sde::audio
