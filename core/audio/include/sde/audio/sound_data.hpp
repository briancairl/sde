/**
 * @copyright 2024-present Brian Cairl
 *
 * @file sound_data.hpp
 */
#pragma once

// C++ Standard Library
#include <cstddef>

// SDE
#include "sde/asset.hpp"
#include "sde/audio/sound_channel_format.hpp"
#include "sde/audio/sound_data_fwd.hpp"
#include "sde/audio/sound_data_handle.hpp"
#include "sde/expected.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/unique_resource.hpp"
#include "sde/view.hpp"

namespace sde::audio
{
/**
 * @brief Error codes pertaining to sound data creation and loading
 */
enum class SoundDataError
{
  kSoundDataNotFound,
  kMissingSoundFile,
  kElementAlreadyExists,
  kInvalidHandle,
  kInvalidSoundFile,
};

std::ostream& operator<<(std::ostream& os, SoundDataError count);

struct SoundDataBufferDeleter
{
  void operator()(void* data) const;
};

using SoundDataBuffer = UniqueResource<void*, SoundDataBufferDeleter>;

/**
 * @brief In memory sound data, typically loaded from disk
 */
struct SoundData : Resource<SoundData>
{
  /// Path to sound
  asset::path path;

  /// Sound samples
  SoundDataBuffer buffered_samples = SoundDataBuffer{nullptr};

  /// Length of buffer, in bytes
  std::size_t buffer_length = 0;

  /// Sound channel formatting
  SoundChannelFormat buffer_channel_format = {};

  auto field_list()
  {
    return FieldList(
      (Field{"path", path}),
      (_Stub{"buffered_samples", buffered_samples}),
      (_Stub{"buffer_length", buffer_length}),
      (_Stub{"buffer_channel_format", buffer_channel_format}));
  }

  /**
   * @brief Returns pointer to sound data
   */
  [[nodiscard]] auto data() const
  {
    return View<const std::byte>{reinterpret_cast<const std::byte*>(buffered_samples.value()), buffer_length};
  }
};

class SoundDataCache : public ResourceCache<SoundDataCache>
{
  friend fundemental_type;

private:
  static expected<void, SoundDataError> reload(dependencies deps, SoundData& sound);
  static expected<void, SoundDataError> unload(dependencies deps, SoundData& sound);
  expected<SoundData, SoundDataError> generate(dependencies deps, const asset::path& sound_path);
};

}  // namespace sde::audio