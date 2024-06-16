// C++ Standard Library
#include <ostream>

// Backend
#include "openal.inl"

// SDE
#include "sde/audio/player.hpp"
#include "sde/logging.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::audio
{

void NativeDeviceDeleter::operator()(device_handle_t id) const { alcCloseDevice(reinterpret_cast<ALCdevice*>(id)); }

void NativeContextDeleter::operator()(context_handle_t id) const
{
  alcDestroyContext(reinterpret_cast<ALCcontext*>(id));
}

PlayerContext::PlayerContext(context_handle_t handle) : UniqueResource<context_handle_t, NativeContextDeleter>{handle}
{}

bool PlayerContext::setActive() const
{
  return alcMakeContextCurrent(reinterpret_cast<ALCcontext*>(this->value())) == ALC_TRUE;
}

Player::Player(NativeDevice&& device, PlayerContext&& context) :
    device_{std::move(device)}, context_{std::move(context)}
{}

expected<Player, PlayerError> Player::create(const PlayerOptions& options)
{
  const auto native_device_handle = [&options] {
    if (options.device_name == nullptr)
    {
      const char* default_device_name = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
      return alcOpenDevice(default_device_name);
    }
    else
    {
      return alcOpenDevice(options.device_name);
    }
  }();
  if (const auto error = alGetError(); error != ALC_NO_ERROR)
  {
    SDE_LOG_DEBUG_FMT("error: %s", al_error_to_str(error));
    return make_unexpected(PlayerError::kBackendCannotOpenDevice);
  }

  NativeDevice device{reinterpret_cast<void*>(native_device_handle)};

  const auto native_context_handle = alcCreateContext(native_device_handle, NULL);
  if (const auto error = alGetError(); error != ALC_NO_ERROR)
  {
    SDE_LOG_DEBUG_FMT("error: %s", al_error_to_str(error));
    return make_unexpected(PlayerError::kBackendFailedContextCreation);
  }

  PlayerContext player_context{reinterpret_cast<void*>(native_context_handle)};

  return Player{std::move(device), std::move(player_context)};
}

}  // namespace sde::audio
