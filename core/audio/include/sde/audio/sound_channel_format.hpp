/**
 * @copyright 2024-present Brian Cairl
 *
 * @file channel_format.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

namespace sde::audio
{

/**
 * @brief Designates number of sound channels
 */
enum struct SoundChannelCount
{
  kMono,
  kStereo
};

std::ostream& operator<<(std::ostream& os, SoundChannelCount count);

/**
 * @brief Designates bit-depth of sound data sample
 */
enum struct SoundChannelBitDepth
{
  kU8,
  kU16
};

std::ostream& operator<<(std::ostream& os, SoundChannelBitDepth element_type);

/**
 * @brief Audio channel description
 */
struct SoundChannelFormat
{
  /// Number of channels (mono, stereo)
  SoundChannelCount count = SoundChannelCount::kMono;
  /// Number of bits per channel sample
  SoundChannelBitDepth element_type = SoundChannelBitDepth::kU8;
  /// Bit rate
  std::size_t bits_per_second = 0;
};

std::ostream& operator<<(std::ostream& os, const SoundChannelFormat& format);

}  // namespace sde::audio
