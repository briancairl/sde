// C++ Standard Library
#include <optional>
#include <ostream>

// Backend
#include "openal.inl"

// SDE
#include "sde/audio/player.hpp"
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_data.hpp"

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

expected<SoundInfo, SoundError>
SoundCache::generate(const PlayerContext& context, const SoundData& sound, const SoundOptions& options)
{
  if (!context.setActive())
  {
    return make_unexpected(SoundError::kInvalidPlayerContext);
  }

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
    .native_id = std::move(native_id),
  };
}

}  // namespace sde::audio
