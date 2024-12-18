/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound_fwd.hpp
 */
#pragma once

// SDE
#include "sde/resource_cache_traits.hpp"

namespace sde::audio
{

enum struct SoundError;
struct Sound;
struct SoundHandle;
class SoundCache;
class SoundDataCache;

}  // namespace sde::audio

namespace sde
{
template <> struct ResourceCacheTraits<audio::SoundCache>
{
  using error_type = audio::SoundError;
  using handle_type = audio::SoundHandle;
  using value_type = audio::Sound;
  using dependencies = ResourceDependencies<audio::SoundDataCache>;
};

template <> struct ResourceHandleToCache<audio::SoundHandle>
{
  using type = audio::SoundCache;
};
}  // namespace sde
