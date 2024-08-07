// C++ Standard Library
#include <optional>
#include <ostream>

// LibAudio
#include <audio/wave.h>

// SDE
#include "sde/audio/sound_data.hpp"
#include "sde/logging.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::audio
{
namespace
{
std::optional<SoundChannelCount> toSoundChannelCount(std::uint32_t channel_count)
{
  switch (channel_count)
  {
  case 1:
    return SoundChannelCount::kMono;
  case 2:
    return SoundChannelCount::kStereo;
  }
  return std::nullopt;
}

std::optional<SoundChannelBitDepth> toSoundChannelBitDepth(std::uint32_t bits_per_sample)
{
  switch (bits_per_sample)
  {
  case 8:
    return SoundChannelBitDepth::kU8;
  case 16:
    return SoundChannelBitDepth::kU16;
  }
  return std::nullopt;
}

}  // namespace

std::ostream& operator<<(std::ostream& os, SoundDataError count)
{
  // clang-format off
  switch (count)
  {
  case SoundDataError::kSoundDataNotFound: return os << "SoundDataNotFound";
  case SoundDataError::kMissingSoundFile: return os << "MissingSoundFile";
  case SoundDataError::kInvalidSoundFile: return os << "InvalidSoundFile";
  case SoundDataError::kInvalidHandle: return os << "InvalidHandle";
  case SoundDataError::kElementAlreadyExists: return os << "ElementAlreadyExists";
  }
  // clang-format on
  return os;
}

void SoundDataBufferDeleter::operator()(void* data) const { std::free(data); }

expected<void, SoundDataError> SoundDataCache::unload(SoundData& sound)
{
  sound.buffered_samples = SoundDataBuffer{nullptr};
  return {};
}

expected<void, SoundDataError> SoundDataCache::reload(SoundData& sound)
{
  // Check that sound file exists
  if (!asset::exists(sound.path))
  {
    SDE_LOG_DEBUG("MissingSoundFile");
    return make_unexpected(SoundDataError::kMissingSoundFile);
  }

  // Read WAV meta information
  auto wave =
    UniqueResource{WaveOpenFileForReading(sound.path.c_str()), [](WaveInfo* wave_ptr) { WaveCloseFile(wave_ptr); }};
  if (wave == nullptr)
  {
    SDE_LOG_DEBUG("InvalidSoundFile");
    return make_unexpected(SoundDataError::kInvalidSoundFile);
  }

  // Seek WAV to start
  if (const auto retcode = WaveSeekFile(0, wave); retcode != 0)
  {
    SDE_LOG_DEBUG("InvalidSoundFile");
    return make_unexpected(SoundDataError::kInvalidSoundFile);
  }

  // Read WAV data
  auto* wave_data = reinterpret_cast<char*>(std::malloc(wave->dataSize));
  if (const auto read_size = WaveReadFile(wave_data, wave->dataSize, wave); read_size != wave->dataSize)
  {
    SDE_LOG_DEBUG("InvalidSoundFile");
    return make_unexpected(SoundDataError::kInvalidSoundFile);
  }

  auto channel_count_opt = toSoundChannelCount(wave->channels);
  if (!channel_count_opt.has_value())
  {
    SDE_LOG_DEBUG("InvalidSoundFile");
    return make_unexpected(SoundDataError::kInvalidSoundFile);
  }

  auto channel_element_type_opt = toSoundChannelBitDepth(wave->bitsPerSample);
  if (!channel_element_type_opt.has_value())
  {
    SDE_LOG_DEBUG("InvalidSoundFile");
    return make_unexpected(SoundDataError::kInvalidSoundFile);
  }

  sound.buffered_samples = SoundDataBuffer{wave_data};
  sound.buffer_length = static_cast<std::size_t>(wave->dataSize);
  sound.buffer_channel_format = {
    .count = std::move(channel_count_opt).value(),
    .element_type = std::move(channel_element_type_opt).value(),
    .bits_per_second = static_cast<std::size_t>(wave->sampleRate)};

  SDE_LOG_DEBUG_FMT("Loaded sound from file: %s (%lu bytes)", sound.path.string().c_str(), sound.buffer_length);
  return {};
}

expected<SoundData, SoundDataError> SoundDataCache::generate(const asset::path& sound_path)
{
  SoundData sound{
    .path = sound_path, .buffered_samples = SoundDataBuffer{nullptr}, .buffer_length = 0, .buffer_channel_format = {}};
  if (auto ok_or_error = reload(sound); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return sound;
}


}  // namespace sde::audio
