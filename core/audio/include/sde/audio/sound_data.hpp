/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound_data.hpp
 */
#pragma once

// C++ Standard Library
#include <cstddef>
#include <memory>

// SDE
#include "sde/asset.hpp"
#include "sde/audio/sound_channel_format.hpp"
#include "sde/expected.hpp"
#include "sde/view.hpp"

namespace sde::audio
{

/**
 * @brief Error codes pertaining to sound data creation and loading
 */
enum class SoundDataError
{
  kMissingSoundFile,
  kInvalidSoundFile,
  kInvalidSoundFileSeek,
  kInvalidSoundFileReadSize,
  kInvalidSoundFileChannelCount,
  kInvalidSoundFileBitDepth,
};

std::ostream& operator<<(std::ostream& os, SoundDataError count);

/**
 * @brief In memory sound data, typically loaded from disk
 */
class SoundData
{
public:
  ~SoundData();

  SoundData(SoundData&& other);
  SoundData& operator=(SoundData&&);

  /**
   * @brief Instances sound from sound data
   */
  [[nodiscard]] static expected<SoundData, SoundDataError> create(
    std::byte* data,
    const std::size_t buffer_length,
    const std::size_t bits_per_second,
    const SoundChannelFormat& channel_format);

  /**
   * @brief Loads a sound from a file
   */
  [[nodiscard]] static expected<SoundData, SoundDataError> load(const asset::path& path);

  /**
   * @brief Number of bytes used to store sound data
   */
  [[nodiscard]] constexpr std::size_t getBufferLength() const { return buffer_length_; };

  /**
   * @brief Intended bit-rate of sound data
   */
  [[nodiscard]] constexpr std::size_t getBitRate() const { return bits_per_second_; };

  /**
   * @brief Intended sound channel format
   */
  [[nodiscard]] constexpr const SoundChannelFormat& getSoundChannelFormat() { return channel_format_; };

  /**
   * @brief Returns pointer to sound data
   */
  [[nodiscard]] auto data() const
  {
    return View<const std::byte>{reinterpret_cast<const std::byte*>(data_), buffer_length_};
  }

  /**
   * @brief Returns two source resources
   */
  void swap(SoundData& other);

private:
  SoundData() = default;
  SoundData(const SoundData&) = delete;
  SoundData& operator=(const SoundData&) = delete;

  /**
   * @brief Instances sound from sound data
   */
  SoundData(
    std::byte* data,
    const std::size_t buffer_length,
    const std::size_t bits_per_second,
    const SoundChannelFormat& channel_format);

  /// Sound data buffer
  std::byte* data_ = nullptr;
  /// Length of buffer, in bytes
  std::size_t buffer_length_ = 0;
  /// Bit rate
  std::size_t bits_per_second_ = 0;
  /// Sound channel formatting
  SoundChannelFormat channel_format_;
};

}  // namespace sde::audio
