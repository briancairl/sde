/**
 * @copyright 2024-present Brian Cairl
 *
 * @file player.hpp
 */
#pragma once

// C++ Standard Library
#include <vector>

// SDE
#include "sde/audio/typedef.hpp"
#include "sde/expected.hpp"
#include "sde/resource_wrapper.hpp"
#include "sde/view.hpp"

namespace sde::audio
{

struct NativeDeviceDeleter
{
  void operator()(device_handle_t id) const;
};

using NativeDevice = UniqueResource<device_handle_t, NativeDeviceDeleter>;

struct NativeContextDeleter
{
  void operator()(context_handle_t id) const;
};

using NativeContext = UniqueResource<context_handle_t, NativeContextDeleter>;

struct NativeSourceDeleter
{
  void operator()(source_handle_t id) const;
};

using NativeSource = UniqueResource<source_handle_t, NativeSourceDeleter>;

enum class MixerGroupError
{
  kBackendContextCreationFailure,
  kBackendTrackCreationFailure,
};

struct MixerGroupOptions
{
  std::size_t track_count = 16;
};

class MixerGroup
{
public:
  [[nodiscard]] static expected<MixerGroup, MixerGroupError>
  create(const NativeDevice& device, const MixerGroupOptions& options);

private:
  MixerGroup(NativeContext&& context, std::vector<NativeSource>&& tracks);

  NativeContext context_;
  std::vector<NativeSource> tracks_;
};

struct MixerOptions
{
  const char* device_name = nullptr;
};

enum class MixerError
{
  kBackendCannotOpenDevice,
  kMixerGroupCreationFailure,
};

/**
 * @brief High-level interface for sound playback
 */
class Mixer
{
public:
  ~Mixer() = default;
  Mixer(Mixer&& other) = default;

  [[nodiscard]] static expected<Mixer, MixerError> create(const MixerOptions& options = {});

private:
  Mixer(NativeDevice&& device);
  Mixer() = delete;
  Mixer(const Mixer&) = delete;

  NativeDevice device_;
};

}  // namespace sde::audio
