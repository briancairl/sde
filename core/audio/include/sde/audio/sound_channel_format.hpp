/**
 * @copyright 2024-present Brian Cairl
 *
 * @file channel_format.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/resource.hpp"

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
struct SoundChannelFormat : Resource<SoundChannelFormat>
{
  /// Number of channels (mono, stereo)
  SoundChannelCount count = SoundChannelCount::kMono;
  /// Number of bits per channel sample
  SoundChannelBitDepth element_type = SoundChannelBitDepth::kU8;
  /// Bit rate
  std::size_t bits_per_second = 0;

  auto field_list()
  {
    return FieldList(
      Field{"count", count}, Field{"element_type", element_type}, Field{"bits_per_second", bits_per_second});
  }
};

}  // namespace sde::audio
