/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound.hpp
 */
#pragma once

// C++ Standard Library

// SDE
#include "sde/audio/sound_channel_format.hpp"
#include "sde/audio/sound_fwd.hpp"
#include "sde/audio/sound_handle.hpp"
#include "sde/audio/typedef.hpp"
#include "sde/expected.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::audio
{

enum struct SoundError
{
  kAssetNotFound,
  kElementAlreadyExists,
  kInvalidPlayerContext,
  kBackendBufferCreationFailure,
  kBackendBufferTransferFailure,
};

struct SoundOptions
{};

struct NativeBufferDeleter
{
  void operator()(buffer_handle_t id) const;
};

using NativeBufferID = UniqueResource<buffer_handle_t, NativeBufferDeleter>;

struct SoundInfo
{
  SoundOptions options;
  std::size_t buffer_length;
  std::size_t bit_rate;
  NativeBufferID native_id;
};

}  // namespace sde::audio

namespace sde
{

template <> struct ResourceCacheTypes<audio::SoundCache>
{
  using error_type = audio::SoundError;
  using handle_type = audio::SoundHandle;
  using value_type = audio::SoundInfo;
};

}  // namespace sde

namespace sde::audio
{

class SoundCache : public ResourceCache<SoundCache>
{
  friend class ResourceCache<SoundCache>;

private:
  expected<SoundInfo, SoundError> generate(const SoundData& sound, const SoundOptions& options = {});
};

}  // namespace sde::audio
