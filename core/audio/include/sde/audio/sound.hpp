/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound.hpp
 */
#pragma once

// C++ Standard Library

// SDE
#include "sde/asset.hpp"
#include "sde/audio/sound_channel_format.hpp"
#include "sde/audio/sound_data_fwd.hpp"
#include "sde/audio/sound_data_handle.hpp"
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
  kAssetLoadingFailed,
  kInvalidHandle,
  kInvalidSoundData,
  kElementAlreadyExists,
  kBackendBufferCreationFailure,
  kBackendBufferTransferFailure,
};

struct NativeSoundBufferDeleter
{
  void operator()(buffer_handle_t id) const;
};

using NativeSoundBufferID = UniqueResource<buffer_handle_t, NativeSoundBufferDeleter>;

struct SoundInfo
{
  SoundDataHandle sound_data = SoundDataHandle{};
  SoundChannelFormat channel_format = {};
  std::size_t buffer_length = 0;
  NativeSoundBufferID native_id;
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
  friend cache_base;

public:
  explicit SoundCache(SoundDataCache& sound_data);

private:
  SoundDataCache* sound_data_ = nullptr;
  expected<void, SoundError> reload(SoundInfo& sound);
  expected<void, SoundError> unload(SoundInfo& sound);
  expected<SoundInfo, SoundError> generate(const asset::path& sound_data_path);
  expected<SoundInfo, SoundError>
  generate(SoundDataHandle sound_data, ResourceLoading loading = ResourceLoading::kImmediate);
};

}  // namespace sde::audio
