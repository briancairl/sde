/**
 * @copyright 2024-present Brian Cairl
 *
 * @file device.hpp
 */
#pragma once

// SDE
#include "sde/audio/sound_device_fwd.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::audio
{

struct NativeDeviceDeleter
{
  void operator()(NativeSoundDeviceHandle id) const;
};

using NativeSoundDevice = UniqueResource<NativeSoundDeviceHandle, NativeDeviceDeleter>;

struct SoundDevice
{
public:
  static SoundDevice create(const char* device_name = nullptr);
  NativeSoundDeviceHandle handle() const { return device_.value(); }

private:
  explicit SoundDevice(NativeSoundDevice&& device);
  NativeSoundDevice device_;
};

}  // namespace sde::audio
