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
#include "sde/resource_wrapper.hpp"
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
  SoundDataBuffer buffered_samples;

  /// Length of buffer, in bytes
  std::size_t buffer_length = 0;

  /// Sound channel formatting
  SoundChannelFormat buffer_channel_format = {};

  auto fields_list()
  {
    return std::make_tuple(
      (Field{"path", path}),
      (Field{"buffered_samples", buffered_samples} | kNotSerialized),
      (Field{"buffer_length", buffer_length} | kNotSerialized),
      (Field{"buffer_channel_format", buffer_channel_format} | kNotSerialized));
  }

  /**
   * @brief Returns pointer to sound data
   */
  [[nodiscard]] auto data() const
  {
    return View<const std::byte>{reinterpret_cast<const std::byte*>(buffered_samples.value()), buffer_length};
  }
};

}  // namespace sde::audio

namespace sde
{

template <> struct ResourceCacheTypes<audio::SoundDataCache>
{
  using error_type = audio::SoundDataError;
  using handle_type = audio::SoundDataHandle;
  using value_type = audio::SoundData;
};

}  // namespace sde

namespace sde::audio
{

class SoundDataCache : public ResourceCache<SoundDataCache>
{
  friend cache_base;

private:
  static expected<void, SoundDataError> reload(SoundData& sound);
  static expected<void, SoundDataError> unload(SoundData& sound);
  expected<SoundData, SoundDataError> generate(const asset::path& sound_path);
};

}  // namespace sde::audio