// C++ Standard Library
#include <optional>
#include <ostream>

// Backend
#include "openal.inl"

// SDE
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_data.hpp"
#include "sde/logging.hpp"

namespace sde::audio
{
namespace  // anonymous
{
inline ALenum toALChannelFormat(const SoundChannelFormat& format)
{
  if (format.count == SoundChannelCount::kStereo)
  {
    switch (format.bit_depth)
    {
    case SoundChannelBitDepth::kU8:
      return AL_FORMAT_STEREO8;
    case SoundChannelBitDepth::kU16:
      return AL_FORMAT_STEREO16;
    }
  }
  else
  {
    switch (format.bit_depth)
    {
    case SoundChannelBitDepth::kU8:
      return AL_FORMAT_MONO8;
    case SoundChannelBitDepth::kU16:
      return AL_FORMAT_MONO16;
    }
  }
  return 0;
}

}  // namespace anonymous

void NativeBufferDeleter::operator()(buffer_handle_t id) const { alDeleteBuffers(1, &id); }

expected<SoundInfo, SoundError> SoundCache::generate(const SoundData& sound, const SoundOptions& options)
{
  NativeBufferID native_id{[] {
    buffer_handle_t id;
    alGenBuffers(1, &id);
    return id;
  }()};

  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    return make_unexpected(SoundError::kBackendBufferCreationFailure);
  }

  alBufferData(
    native_id,
    toALChannelFormat(sound.getChannelFormat()),
    sound.data().data(),
    sound.data().size(),
    sound.getBitRate());

  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    return make_unexpected(SoundError::kBackendBufferTransferFailure);
  }

  return SoundInfo{
    .options = options,
    .buffer_length = sound.data().size(),
    .bit_rate = sound.getBitRate(),
    .native_id = std::move(native_id),
  };
}

SoundCache::result_type
SoundCacheLoader::operator()(SoundCache& cache, const asset::path& path, const SoundOptions& options) const
{
  if (auto sound_data_or_error = SoundData::load(path); sound_data_or_error.has_value())
  {
    SDE_LOG_INFO_FMT("sound loaded from disk: %s", path.string().c_str());
    return cache.create(*sound_data_or_error, options);
  }
  SDE_LOG_DEBUG("AssetLoadingFailed");
  return make_unexpected(SoundError::kAssetLoadingFailed);
}

SoundCache::result_type SoundCacheLoader::operator()(
  SoundCache& cache,
  const SoundHandle& handle,
  const asset::path& path,
  const SoundOptions& options) const
{
  if (auto sound_data_or_error = SoundData::load(path); sound_data_or_error.has_value())
  {
    SDE_LOG_INFO_FMT("sound loaded from disk: %s", path.string().c_str());
    return cache.insert(handle, *sound_data_or_error, options);
  }
  SDE_LOG_DEBUG("AssetLoadingFailed");
  return make_unexpected(SoundError::kAssetLoadingFailed);
}

}  // namespace sde::audio
