/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
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
#include "sde/unique_resource.hpp"

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

std::ostream& operator<<(std::ostream& os, SoundError error);

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
  NativeSoundBufferID native_id = NativeSoundBufferID{0};

  auto field_list()
  {
    return FieldList(
      (Field{"sound_data", sound_data}),
      (_Stub{"channel_format", channel_format}),
      (_Stub{"buffer_length", buffer_length}),
      (_Stub{"native_id", native_id}));
  }
};

class SoundCache : public ResourceCache<SoundCache>
{
  friend fundemental_type;

private:
  expected<void, SoundError> reload(dependencies deps, Sound& sound);
  expected<void, SoundError> unload(dependencies deps, Sound& sound);
  expected<Sound, SoundError> generate(dependencies deps, const asset::path& sound_data_path);
  expected<Sound, SoundError> generate(dependencies deps, SoundDataHandle sound_data);
};

}  // namespace sde::audio
