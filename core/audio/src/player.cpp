// C++ Standard Library
#include <ostream>

// Backend
#include "openal.inl"

// SDE
#include "sde/audio/player.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::audio
{
namespace
{

struct DeviceNativeDeleter
{
  void operator()(device_handle_t handle) const { alcCloseDevice(reinterpret_cast<ALCdevice*>(handle)); }
};

using DeviceNative = UniqueResource<device_handle_t, DeviceNativeDeleter>;


struct ContextNativeDeleter
{
  void operator()(context_handle_t handle) const { alcDestroyContext(reinterpret_cast<ALCcontext*>(handle)); }
};

using ContextNative = UniqueResource<context_handle_t, ContextNativeDeleter>;

}  // namespace


}  // namespace sde::audio
