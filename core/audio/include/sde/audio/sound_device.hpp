/**
 * @copyright 2024-present Brian Cairl
 *
 * @file device.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/audio/sound_device_fwd.hpp"
#include "sde/expected.hpp"
#include "sde/unique_resource.hpp"

namespace sde::audio
{

struct NativeContextDeleter
{
  void operator()(NativeSoundContextHandle id) const;
};

using NativeContext = UniqueResource<NativeSoundContextHandle, NativeContextDeleter>;

struct NativeDeviceDeleter
{
  void operator()(NativeSoundDeviceHandle id) const;
};

using NativeSoundDevice = UniqueResource<NativeSoundDeviceHandle, NativeDeviceDeleter>;

enum class SoundDeviceError
{
  kFailedToCreateBackendDevice,
  kFailedToCreateBackendContext,
};

std::ostream& operator<<(std::ostream& os, SoundDeviceError error);

struct SoundDevice
{
public:
  static expected<SoundDevice, SoundDeviceError> create(const char* device_name = nullptr);
  NativeSoundDeviceHandle handle() const { return device_.value(); }

private:
  SoundDevice(NativeSoundDevice&& device, NativeContext&& default_context);
  NativeSoundDevice device_;
  NativeContext default_context_;
};

}  // namespace sde::audio
