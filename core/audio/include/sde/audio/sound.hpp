/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound.hpp
 */
#pragma once

// C++ Standard Library
#include <tuple>

// SDE
#include "sde/asset.hpp"
#include "sde/audio/sound_channel_format.hpp"
#include "sde/audio/sound_data_fwd.hpp"
#include "sde/audio/sound_data_handle.hpp"
#include "sde/audio/sound_fwd.hpp"
#include "sde/audio/sound_handle.hpp"
#include "sde/audio/typedef.hpp"
#include "sde/expected.hpp"
#include "sde/resource.hpp"
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

struct Sound : Resource<Sound>
{
  SoundDataHandle sound_data = SoundDataHandle{};
  SoundChannelFormat channel_format = {};
  std::size_t buffer_length = 0;
  NativeSoundBufferID native_id;

  auto field_list()
  {
    return FieldList(
      (Field{"sound_data", sound_data}),
      (_Stub{"channel_format", channel_format}),
      (_Stub{"buffer_length", buffer_length}),
      (_Stub{"native_id", native_id}));
  }
};

}  // namespace sde::audio

namespace sde
{

template <> struct ResourceCacheTypes<audio::SoundCache>
{
  using error_type = audio::SoundError;
  using handle_type = audio::SoundHandle;
  using value_type = audio::Sound;
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
  expected<void, SoundError> reload(Sound& sound);
  expected<void, SoundError> unload(Sound& sound);
  expected<Sound, SoundError> generate(const asset::path& sound_data_path);
  expected<Sound, SoundError> generate(SoundDataHandle sound_data);
};

}  // namespace sde::audio
