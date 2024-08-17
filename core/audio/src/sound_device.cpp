// C++ Standard Library
#include <algorithm>
#include <ostream>

// Backend
#include "openal.inl"

// SDE
#include "sde/audio/sound_device.hpp"
#include "sde/logging.hpp"

namespace sde::audio
{
namespace
{}  // namespace


void NativeContextDeleter::operator()(context_handle_t id) const
{
  SDE_LOG_DEBUG_FMT("context destroyed: %p", id);
  alcDestroyContext(reinterpret_cast<ALCcontext*>(id));
}

void NativeDeviceDeleter::operator()(device_handle_t id) const
{
  SDE_LOG_DEBUG_FMT("device closed: %p", id);
  alcCloseDevice(reinterpret_cast<ALCdevice*>(id));
}

SoundDevice::SoundDevice(NativeSoundDevice&& device, NativeContext&& default_context) :
    device_{std::move(device)}, default_context_{std::move(default_context)}
{}

expected<SoundDevice, SoundDeviceError> SoundDevice::create(const char* device_name)
{
  // Wrap device handle so that it will get cleaned up on failued
  NativeSoundDevice native_device_handle = [device_name] {
    if (device_name == nullptr)
    {
      const char* default_device_name = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
      SDE_LOG_DEBUG_FMT("alcOpenDevice: %s", default_device_name);
      return NativeSoundDevice{alcOpenDevice(default_device_name)};
    }
    else
    {
      return NativeSoundDevice{alcOpenDevice(device_name)};
    }
  }();

  // Check for failure to create device
  if (native_device_handle.isNull())
  {
    return make_unexpected(SoundDeviceError::kFailedToCreateBackendDevice);
  }

  // Create a default context for sound creation
  NativeContext native_context_handle{
    alcCreateContext(reinterpret_cast<ALCdevice*>(native_device_handle.value()), NULL)};
  if (native_context_handle.isNull())
  {
    return make_unexpected(SoundDeviceError::kFailedToCreateBackendContext);
  }

  if (alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(native_context_handle.value())) != ALC_TRUE)
  {
    SDE_LOG_DEBUG_FMT("alcMakeContextCurrent(%p)", native_context_handle.value());
    return make_unexpected(SoundDeviceError::kFailedToCreateBackendContext);
  }

  return SoundDevice{std::move(native_device_handle), std::move(native_context_handle)};
}

}  // namespace sde::audio
