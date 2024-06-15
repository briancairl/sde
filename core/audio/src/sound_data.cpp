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
  case SoundDataError::kMissingSoundFile: return os << "MissingSoundFile";
  case SoundDataError::kInvalidSoundFile: return os << "InvalidSoundFile";
  case SoundDataError::kInvalidSoundFileSeek: return os << "InvalidSoundFileSeek";
  case SoundDataError::kInvalidSoundFileReadSize: return os << "InvalidSoundFileReadSize";
  case SoundDataError::kInvalidSoundFileChannelCount: return os << "InvalidSoundFileChannelCount";
  case SoundDataError::kInvalidSoundFileBitDepth: return os << "InvalidSoundFileBitDepth";
  }
  // clang-format on
  return os;
}

SoundData::SoundData(SoundData&& other) { this->swap(other); }

SoundData::SoundData(
  std::byte* const data,
  const std::size_t buffer_length,
  const std::size_t bits_per_second,
  const SoundChannelFormat& channel_format) :
    data_{data}, buffer_length_{buffer_length}, bits_per_second_{bits_per_second}, channel_format_{channel_format}
{}

SoundData::~SoundData()
{
  if (data_ == nullptr)
  {
    return;
  }
  std::free(reinterpret_cast<void*>(data_));
}

SoundData& SoundData::operator=(SoundData&& other)
{
  this->swap(other);
  return *this;
}

expected<SoundData, SoundDataError> SoundData::load(const asset::path& path)
{
  // Check that sound file exists
  if (!asset::exists(path))
  {
    return make_unexpected(SoundDataError::kMissingSoundFile);
  }

  // Read WAV meta information
  auto wave =
    make_unique_resource(WaveOpenFileForReading(path.c_str()), [](WaveInfo* wave_ptr) { WaveCloseFile(wave_ptr); });
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

  auto channel_bit_depth_opt = toSoundChannelBitDepth(wave->bitsPerSample);
  if (!channel_bit_depth_opt.has_value())
  {
    return make_unexpected(SoundDataError::kInvalidSoundFileBitDepth);
  }

  return SoundData::create(
    reinterpret_cast<std::byte*>(wave_data),
    wave->dataSize,
    wave->sampleRate,
    SoundChannelFormat{
      .count = std::move(channel_count_opt).value(), .bit_depth = std::move(channel_bit_depth_opt).value()});
}

void SoundData::swap(SoundData& other)
{
  std::swap(data_, other.data_);
  std::swap(buffer_length_, other.buffer_length_);
  std::swap(bits_per_second_, other.bits_per_second_);
  std::swap(channel_format_, other.channel_format_);
}

}  // namespace sde::audio
