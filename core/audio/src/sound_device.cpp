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

void NativeDeviceDeleter::operator()(device_handle_t id) const
{
  SDE_LOG_DEBUG_FMT("device closed: %p", id);
  alcCloseDevice(reinterpret_cast<ALCdevice*>(id));
}

SoundDevice::SoundDevice(NativeSoundDevice&& device) : device_{std::move(device)} {}

SoundDevice SoundDevice::create(const char* device_name)
{
  ALCdevice* native_device_handle{nullptr};
  if (device_name == nullptr)
  {
    const char* default_device_name = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    SDE_LOG_DEBUG_FMT("alcOpenDevice: %s", default_device_name);
    native_device_handle = alcOpenDevice(default_device_name);
  }
  else
  {
    native_device_handle = alcOpenDevice(device_name);
  }
  return SoundDevice{NativeSoundDevice{native_device_handle}};
}

}  // namespace sde::audio
