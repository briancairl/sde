// C++ Standard Library
#include <ostream>

// Backend
#include "openal.inl"

// SDE
#include "sde/audio/mixer.hpp"
#include "sde/logging.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::audio
{

void NativeSourceDeleter::operator()(source_handle_t id) const
{
  if (id != 0)
  {
    alDeleteSources(1, &id);
  }
}

void NativeDeviceDeleter::operator()(device_handle_t id) const
{
  if (id != nullptr)
  {
    SDE_LOG_DEBUG_FMT("device closed: %p", id);
    alcCloseDevice(reinterpret_cast<ALCdevice*>(id));
  }
}

void NativeContextDeleter::operator()(context_handle_t id) const
{
  if (id != nullptr)
  {
    alcDestroyContext(reinterpret_cast<ALCcontext*>(id));
  }
}

expected<MixerGroup, MixerGroupError> MixerGroup::create(const NativeDevice& device, const MixerGroupOptions& options)
{
  // Create listener context
  const auto native_context_handle = alcCreateContext(reinterpret_cast<ALCdevice*>(device.value()), NULL);
  if (native_context_handle == nullptr)
  {
    SDE_LOG_DEBUG_FMT("alcCreateContext: %p", native_context_handle);
    return make_unexpected(MixerGroupError::kBackendContextCreationFailure);
  }

  // Set context as current context
  NativeContext context{reinterpret_cast<void*>(native_context_handle)};
  if (alcMakeContextCurrent(native_context_handle) != ALC_TRUE)
  {
    SDE_LOG_DEBUG_FMT("alcMakeContextCurrent: %p", native_context_handle);
    return make_unexpected(MixerGroupError::kBackendContextCreationFailure);
  }

  // Create create sources attached to this context
  std::vector<NativeSource> tracks;
  for (std::size_t t = 0; t < options.track_count; ++t)
  {
    source_handle_t native_source_id{0};
    alGenSources(1, &native_source_id);
    if (const auto error = alGetError(); error != AL_NO_ERROR)
    {
      SDE_LOG_DEBUG_FMT("%s", al_error_to_str(error));
      return make_unexpected(MixerGroupError::kBackendTrackCreationFailure);
    }
    tracks.emplace_back(native_source_id);
  }
  return MixerGroup{std::move(context), std::move(tracks)};
}

MixerGroup::MixerGroup(NativeContext&& context, std::vector<NativeSource>&& tracks) :
    context_{std::move(context)}, tracks_{std::move(tracks)}
{}

expected<Mixer, MixerError> Mixer::create(const MixerOptions& options)
{
  const auto native_device_handle = [&options] {
    if (options.device_name == nullptr)
    {
      const char* default_device_name = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
      SDE_LOG_DEBUG_FMT("alcOpenDevice: %s", default_device_name);
      return alcOpenDevice(default_device_name);
    }
    else
    {
      return alcOpenDevice(options.device_name);
    }
  }();
  if (native_device_handle == nullptr)
  {
    SDE_LOG_DEBUG_FMT("alcOpenDevice: %p", native_device_handle);
    return make_unexpected(MixerError::kBackendCannotOpenDevice);
  }
  return Mixer{NativeDevice{reinterpret_cast<void*>(native_device_handle)}};
}

Mixer::Mixer(NativeDevice&& device) : device_{std::move(device)} {}

}  // namespace sde::audio
