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
    switch (format.element_type)
    {
    case SoundChannelBitDepth::kU8:
      return AL_FORMAT_STEREO8;
    case SoundChannelBitDepth::kU16:
      return AL_FORMAT_STEREO16;
    }
  }
  else
  {
    switch (format.element_type)
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

std::ostream& operator<<(std::ostream& os, SoundError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(SoundError::kAssetNotFound)
    SDE_OS_ENUM_CASE(SoundError::kAssetLoadingFailed)
    SDE_OS_ENUM_CASE(SoundError::kInvalidHandle)
    SDE_OS_ENUM_CASE(SoundError::kInvalidSoundData)
    SDE_OS_ENUM_CASE(SoundError::kElementAlreadyExists)
    SDE_OS_ENUM_CASE(SoundError::kBackendBufferCreationFailure)
    SDE_OS_ENUM_CASE(SoundError::kBackendBufferTransferFailure)
  }
  return os;
}

void NativeSoundBufferDeleter::operator()(buffer_handle_t id) const { alDeleteBuffers(1, &id); }

SoundCache::SoundCache(SoundDataCache& sound_data) : sound_data_{std::addressof(sound_data)} {}

expected<void, SoundError> SoundCache::reload(Sound& sound)
{
  const auto* sound_data = sound_data_->get_if(sound.sound_data);
  if (sound_data == nullptr)
  {
    SDE_LOG_ERROR() << "InvalidSoundData: " << SDE_OSNV(sound.sound_data);
    return make_unexpected(SoundError::kInvalidSoundData);
  }

  buffer_handle_t id;
  alGenBuffers(1, &id);

  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    SDE_LOG_ERROR() << "BackendBufferCreationFailure: " << SDE_OSNV(al_error_to_str(error));
    return make_unexpected(SoundError::kBackendBufferCreationFailure);
  }

  NativeSoundBufferID native_id{id};
  alBufferData(
    native_id,
    toALChannelFormat(sound_data->buffer_channel_format),
    sound_data->data().data(),
    sound_data->data().size(),
    sound_data->buffer_channel_format.bits_per_second);

  if (const auto error = alGetError(); error != AL_NO_ERROR)
  {
    SDE_LOG_ERROR() << "BackendBufferTransferFailure: " << SDE_OSNV(al_error_to_str(error));
    return make_unexpected(SoundError::kBackendBufferTransferFailure);
  }

  sound.channel_format = sound_data->buffer_channel_format;
  sound.buffer_length = sound_data->buffer_length;
  sound.native_id = std::move(native_id);
  return {};
}

expected<void, SoundError> SoundCache::unload(Sound& sound)
{
  sound.native_id = NativeSoundBufferID{0};
  return {};
}

expected<Sound, SoundError> SoundCache::generate(const asset::path& sound_data_path)
{
  auto sound_data_or_error = sound_data_->create(sound_data_path);
  if (!sound_data_or_error.has_value())
  {
    SDE_LOG_ERROR() << "InvalidSoundData: " << sound_data_or_error.error();
    return make_unexpected(SoundError::kInvalidSoundData);
  }
  return generate(sound_data_or_error->handle);
}

expected<Sound, SoundError> SoundCache::generate(SoundDataHandle sound_data)
{
  Sound sound{.sound_data = sound_data, .channel_format = {}, .buffer_length = 0, .native_id = NativeSoundBufferID{0}};
  if (auto ok_or_error = reload(sound); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return sound;
}

}  // namespace sde::audio
