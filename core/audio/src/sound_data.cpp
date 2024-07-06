// C++ Standard Library
#include <optional>
#include <ostream>

// LibAudio
#include <audio/wave.h>

// SDE
#include "sde/audio/sound_data.hpp"
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
  case SoundDataError::kElementAlreadyExists: return os << "ElementAlreadyExists";
  case SoundDataError::kInvalidSoundFileSeek: return os << "InvalidSoundFileSeek";
  case SoundDataError::kInvalidSoundFileReadSize: return os << "InvalidSoundFileReadSize";
  case SoundDataError::kInvalidSoundFileChannelCount: return os << "InvalidSoundFileChannelCount";
  case SoundDataError::kInvalidSoundFileBitDepth: return os << "InvalidSoundFileBitDepth";
  }
  // clang-format on
  return os;
}

void SoundDataBufferDeleter::operator()(void* data) const { std::free(data); }

expected<void, SoundDataError> SoundDataCache::reload(SoundDataHandle sound)
{
  if (const auto handle_and_value_itr = handle_to_value_cache_.find(sound);
      handle_and_value_itr != std::end(handle_to_value_cache_))
  {
    return reload(handle_and_value_itr->second);
  }
  return make_unexpected(SoundDataError::kSoundDataNotFound);
}

expected<void, SoundDataError> SoundDataCache::unload(SoundDataHandle sound)
{
  if (const auto handle_and_value_itr = handle_to_value_cache_.find(sound);
      handle_and_value_itr != std::end(handle_to_value_cache_))
  {
    auto& info = handle_and_value_itr->second;
    info.buffered_samples = SoundDataBuffer{nullptr};
    return {};
  }
  return make_unexpected(SoundDataError::kSoundDataNotFound);
}

expected<void, SoundDataError> SoundDataCache::reload(SoundDataInfo& sound)
{
  // Check that sound file exists
  if (!asset::exists(sound.path))
  {
    return make_unexpected(SoundDataError::kMissingSoundFile);
  }

  // Read WAV meta information
  auto wave = make_unique_resource(
    WaveOpenFileForReading(sound.path.c_str()), [](WaveInfo* wave_ptr) { WaveCloseFile(wave_ptr); });
  if (wave == nullptr)
  {
    return make_unexpected(SoundDataError::kInvalidSoundFile);
  }

  // Seek WAV to start
  if (const auto retcode = WaveSeekFile(0, wave); retcode != 0)
  {
    return make_unexpected(SoundDataError::kInvalidSoundFileSeek);
  }

  // Read WAV data
  auto* wave_data = reinterpret_cast<char*>(std::malloc(wave->dataSize));
  if (const auto read_size = WaveReadFile(wave_data, wave->dataSize, wave); read_size != wave->dataSize)
  {
    return make_unexpected(SoundDataError::kInvalidSoundFileReadSize);
  }

  auto channel_count_opt = toSoundChannelCount(wave->channels);
  if (!channel_count_opt.has_value())
  {
    return make_unexpected(SoundDataError::kInvalidSoundFileChannelCount);
  }

  auto channel_element_type_opt = toSoundChannelBitDepth(wave->bitsPerSample);
  if (!channel_element_type_opt.has_value())
  {
    return make_unexpected(SoundDataError::kInvalidSoundFileBitDepth);
  }

  sound.buffered_samples = SoundDataBuffer{wave_data};
  sound.buffer_length = static_cast<std::size_t>(wave->dataSize);
  sound.buffer_channel_format = {
    .count = std::move(channel_count_opt).value(),
    .element_type = std::move(channel_element_type_opt).value(),
    .bits_per_second = static_cast<std::size_t>(wave->sampleRate)};
  return {};
}

expected<SoundDataInfo, SoundDataError> SoundDataCache::generate(const asset::path& sound_path, ResourceLoading loading)
{
  SoundDataInfo sound{
    .path = sound_path, .buffered_samples = SoundDataBuffer{nullptr}, .buffer_length = 0, .buffer_channel_format = {}};
  if (loading == ResourceLoading::kDeferred)
  {
    return sound;
  }
  if (auto ok_or_error = reload(sound); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return sound;
}


}  // namespace sde::audio
