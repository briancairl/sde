/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound_data_fwd.hpp
 */
#pragma once

// SDE
#include "sde/resource_cache_traits.hpp"

namespace sde::audio
{
enum class SoundDataError;
struct SoundData;
struct SoundDataHandle;
class SoundDataCache;
}  // namespace sde::audio

namespace sde
{
template <> struct ResourceCacheTraits<audio::SoundDataCache>
{
  using error_type = audio::SoundDataError;
  using handle_type = audio::SoundDataHandle;
  using value_type = audio::SoundData;
  using dependencies = no_dependencies;
};

template <> struct ResourceHandleToCache<audio::SoundDataHandle>
{
  using type = audio::SoundDataCache;
};
}  // namespace sde